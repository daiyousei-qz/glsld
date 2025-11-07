#include "Compiler/SyntaxToken.h"
#include "Feature/Hover.h"
#include "Server/StandardDocumentation.h"
#include "Support/Markdown.h"
#include "Support/SourceText.h"

#include <string>

namespace glsld
{
    auto ComputeHoverText(const HoverContent& hover) -> std::string
    {
        MarkdownBuilder builder;

        // Header explaining what the hover symbol is
        StringView hoverTypeName = [&]() {
            switch (hover.type) {
            case SymbolDeclType::HeaderName:
                return "Header Name";
            case SymbolDeclType::Macro:
                return "Macro";
            case SymbolDeclType::LayoutQualifier:
                return "Layout Qualifier";
            case SymbolDeclType::GlobalVariable:
                if (hover.builtin) {
                    return "Built-in Variable";
                }
                else {
                    return "Global Variable";
                }
            case SymbolDeclType::LocalVariable:
                return "Local Variable";
            case SymbolDeclType::Swizzle:
                return "Swizzle";
            case SymbolDeclType::StructMember:
                return "Struct Member";
            case SymbolDeclType::Parameter:
                return "Parameter";
            case SymbolDeclType::Function:
                if (hover.builtin) {
                    return "Built-in Function";
                }
                else {
                    return "Function";
                }
            case SymbolDeclType::Type:
                return "Type";
            case SymbolDeclType::Block:
                return "Interface Block";
            case SymbolDeclType::BlockInstance:
                if (hover.builtin) {
                    return "Built-in Block Instance";
                }
                else {
                    return "Interface Block Instance";
                }
            case SymbolDeclType::BlockMember:
                if (hover.builtin) {
                    return "Built-in Block Member";
                }
                else {
                    return "Interface Block Member";
                }
            }
        }();
        builder.AppendHeader(3, "{}{} `{}`", hover.unknown ? "Unknown " : "", hoverTypeName, hover.name);

        // Hover Info
        if (!hover.returnType.empty()) {
            builder.AppendParagraph("Return Type: `{}`", hover.returnType);
        }
        if (!hover.parameters.empty()) {
            builder.AppendParagraph("Parameters:");
            for (const auto& param : hover.parameters) {
                builder.AppendBullet("`{}`", param);
            }
            builder.AppendParagraph("");
        }
        if (!hover.exprType.empty() && hover.returnType.empty()) {
            // We omit the expression type if the return type is already shown.
            builder.AppendParagraph("Type: `{}`", hover.exprType);
        }
        if (!hover.exprValue.empty()) {
            builder.AppendParagraph("Value: `{}`", hover.exprValue);
        }

        // Description
        if (!hover.description.empty()) {
            builder.AppendRuler();
            builder.AppendParagraph(hover.description);
        }

        // Reconstructed Code
        if (!hover.code.empty()) {
            builder.AppendRuler();
            builder.AppendCodeBlock(hover.code, "glsl");
        }

        return builder.Export();
    }

    static auto CreateHoverContentForPPSymbol(const LanguageQueryInfo& info, const SymbolQueryResult& symbolInfo)
        -> std::optional<HoverContent>
    {
        GLSLD_ASSERT(symbolInfo.ppSymbolOccurrence);
        if (auto headerNameInfo = symbolInfo.ppSymbolOccurrence->GetHeaderNameInfo(); headerNameInfo) {
            return HoverContent{
                .type        = SymbolDeclType::HeaderName,
                .name        = symbolInfo.spelledText,
                .description = fmt::format("See `{}`", headerNameInfo->headerAbsolutePath),
                .range       = symbolInfo.spelledRange,
            };
        }
        else if (auto macroInfo = symbolInfo.ppSymbolOccurrence->GetMacroInfo(); macroInfo) {
            if (!macroInfo->definition) {
                return HoverContent{
                    .type    = SymbolDeclType::Macro,
                    .name    = symbolInfo.spelledText,
                    .range   = symbolInfo.spelledRange,
                    .unknown = true,
                };
            }

            std::vector<std::string> params;
            for (const auto& param : macroInfo->definition->params) {
                params.push_back(param.text.Str());
            }

            std::string codeBuffer = "#define " + macroInfo->macroName.text.Str();
            if (macroInfo->definition->isFunctionLike) {
                codeBuffer += "(";
                for (size_t i = 0; i < params.size(); ++i) {
                    if (i > 0) {
                        codeBuffer += ", ";
                    }
                    codeBuffer += params[i];
                }
                codeBuffer += ")";
            }

            for (const auto& token : macroInfo->definition->tokens) {
                codeBuffer += " ";
                codeBuffer += token.text.StrView();
            }

            return HoverContent{
                .type        = SymbolDeclType::Macro,
                .name        = symbolInfo.spelledText,
                .parameters  = std::move(params),
                .description = "",
                .code        = std::move(codeBuffer),
                .range       = symbolInfo.spelledRange,
            };
        }

        return std::nullopt;
    }

    static auto CreateHoverContentForAstSymbol(const LanguageQueryInfo& info, const SymbolQueryResult& symbolInfo)
        -> std::optional<HoverContent>
    {
        GLSLD_ASSERT(symbolInfo.astSymbolOccurrence);

        HoverContent result;
        result.type  = symbolInfo.symbolType;
        result.name  = symbolInfo.spelledText;
        result.range = symbolInfo.spelledRange;

        if (auto expr = symbolInfo.astSymbolOccurrence->As<AstExpr>(); expr) {
            result.exprType = expr->GetDeducedType()->GetDebugName().Str();
            if (auto constValue = EvalAstExpr(*expr); !constValue.IsError()) {
                result.exprValue = constValue.ToString();
            }
        }

        if (!symbolInfo.symbolDecl) {
            result.unknown = symbolInfo.symbolType != SymbolDeclType::Swizzle &&
                             symbolInfo.symbolType != SymbolDeclType::LayoutQualifier;
        }
        else {
            result.description = info.QueryCommentDescription(*symbolInfo.symbolDecl);
            result.builtin = symbolInfo.symbolDecl && symbolInfo.symbolDecl->GetSyntaxRange().GetTranslationUnit() ==
                                                          TranslationUnitID::SystemPreamble;

            if (auto funcDecl = symbolInfo.symbolDecl->As<AstFunctionDecl>();
                funcDecl && symbolInfo.symbolType == SymbolDeclType::Function) {
                result.returnType = funcDecl->GetReturnType()->GetResolvedType()->GetDebugName().Str();
                if (!funcDecl->GetParams().empty()) {
                    for (auto paramDecl : funcDecl->GetParams()) {
                        std::string paramText;
                        ReconstructSourceText(paramText, *paramDecl);
                        result.parameters.push_back(std::move(paramText));
                    }
                }

                ReconstructSourceText(result.code, *funcDecl);
            }
            else if (auto paramDecl = symbolInfo.symbolDecl->As<AstParamDecl>();
                     paramDecl && symbolInfo.symbolType == SymbolDeclType::Parameter) {
                if (result.exprType.empty()) {
                    result.exprType = paramDecl->GetResolvedType()->GetDebugName().Str();
                }
                ReconstructSourceText(result.code, *paramDecl);
            }
            else if (auto varDeclaratorDecl = symbolInfo.symbolDecl->As<AstVariableDeclaratorDecl>();
                     varDeclaratorDecl && (symbolInfo.symbolType == SymbolDeclType::GlobalVariable ||
                                           symbolInfo.symbolType == SymbolDeclType::LocalVariable)) {
                auto resolvedType = varDeclaratorDecl->GetResolvedType();
                if (result.exprType.empty()) {
                    result.exprType = resolvedType->GetDebugName().Str();
                }

                // We only compute value for declaration here. Const variable value in expression will be handled later.
                if (symbolInfo.isDeclaration && varDeclaratorDecl->IsConstVariable()) {
                    if (auto init = varDeclaratorDecl->GetInitializer(); init) {
                        if (auto constValue = EvalAstInitializer(*init, resolvedType); !constValue.IsError()) {
                            result.exprValue = constValue.ToString();
                        }
                    }
                }

                ReconstructSourceText(result.code, *varDeclaratorDecl->GetQualType(), varDeclaratorDecl->GetNameToken(),
                                      varDeclaratorDecl->GetArraySpec(), varDeclaratorDecl->GetInitializer());
            }
            else if (auto structFieldDeclaratorDecl = symbolInfo.symbolDecl->As<AstStructFieldDeclaratorDecl>();
                     structFieldDeclaratorDecl && symbolInfo.symbolType == SymbolDeclType::StructMember) {
                if (result.exprType.empty()) {
                    result.exprType = structFieldDeclaratorDecl->GetResolvedType()->GetDebugName().Str();
                }
                ReconstructSourceText(
                    result.code, *structFieldDeclaratorDecl->GetQualType(), structFieldDeclaratorDecl->GetNameToken(),
                    structFieldDeclaratorDecl->GetArraySpec(), structFieldDeclaratorDecl->GetInitializer());
            }
            else if (auto structDecl = symbolInfo.symbolDecl->As<AstStructDecl>();
                     structDecl && symbolInfo.symbolType == SymbolDeclType::Type) {
                // TODO: maybe show struct size and alignment?
                ReconstructSourceText(result.code, *structDecl);
            }
            else if (auto blockFieldDeclaratorDecl = symbolInfo.symbolDecl->As<AstBlockFieldDeclaratorDecl>();
                     blockFieldDeclaratorDecl && symbolInfo.symbolType == SymbolDeclType::BlockMember) {
                if (result.exprType.empty()) {
                    result.exprType = blockFieldDeclaratorDecl->GetResolvedType()->GetDebugName().Str();
                }
                ReconstructSourceText(
                    result.code, *blockFieldDeclaratorDecl->GetQualType(), blockFieldDeclaratorDecl->GetNameToken(),
                    blockFieldDeclaratorDecl->GetArraySpec(), blockFieldDeclaratorDecl->GetInitializer());
            }
            else if (auto blockDecl = symbolInfo.symbolDecl->As<AstInterfaceBlockDecl>();
                     blockDecl && (symbolInfo.symbolType == SymbolDeclType::Block ||
                                   symbolInfo.symbolType == SymbolDeclType::BlockInstance)) {
                // TODO: maybe show block layouts?
                ReconstructSourceText(result.code, *blockDecl);
            }
        }

        return result;
    }

    auto CollectHover(const LanguageQueryInfo& info, TextPosition position) -> std::optional<HoverContent>
    {
        const auto& compilerObject = info.GetCompilerResult();

        if (auto symbolInfo = info.QuerySymbolByPosition(position); symbolInfo) {
            // FIXME: is the following needed?
            // if (symbolInfo->token.IsKeyword()) {
            //     // Don't provide hover for keyword
            //     return std::nullopt;
            // }

            if (symbolInfo->ppSymbolOccurrence) {
                return CreateHoverContentForPPSymbol(info, *symbolInfo);
            }
            else if (symbolInfo->astSymbolOccurrence) {
                return CreateHoverContentForAstSymbol(info, *symbolInfo);
            }
        }

        return std::nullopt;
    }

    auto GetHoverOptions(const HoverConfig& config) -> std::optional<lsp::HoverOptions>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return lsp::HoverOptions{};
    }

    auto HandleHover(const HoverConfig& config, const LanguageQueryInfo& info, const lsp::HoverParams& params)
        -> std::optional<lsp::Hover>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return CollectHover(info, FromLspPosition(params.position)).transform([](const HoverContent& hoverContent) {
            return lsp::Hover{
                .contents = lsp::MarkupContent{true, ComputeHoverText(hoverContent)},
                .range    = ToLspRange(hoverContent.range),
            };
        });
    }

} // namespace glsld
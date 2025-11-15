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
        if (!hover.exprType.empty()) {
            if (hover.type == SymbolDeclType::Function) {
                builder.AppendParagraph("Return Type: `{}`", hover.exprType);
            }
            else {
                builder.AppendParagraph("Type: `{}`", hover.exprType);
            }
        }
        if (!hover.parameters.empty()) {
            builder.AppendParagraph("Parameters:");
            for (const auto& param : hover.parameters) {
                builder.AppendBullet("`{}`", param);
            }
            builder.AppendParagraph("");
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

        const Type* exprType = nullptr;
        const Type* declType = nullptr;
        ConstValue exprValue;
        ConstValue declValue;

        std::vector<std::string> reconstructedFuncParams;
        std::string reconstructedDecl;

        if (auto expr = symbolInfo.astSymbolOccurrence->As<AstExpr>(); expr) {
            exprType  = expr->GetDeducedType();
            exprValue = EvalAstExpr(*expr);
        }

        if (symbolInfo.symbolDecl) {
            if (auto funcDecl = symbolInfo.symbolDecl->As<AstFunctionDecl>();
                funcDecl && symbolInfo.symbolType == SymbolDeclType::Function) {
                declType = funcDecl->GetReturnType()->GetResolvedType();

                for (auto paramDecl : funcDecl->GetParams()) {
                    std::string paramText;
                    ReconstructSourceText(paramText, *paramDecl);
                    reconstructedFuncParams.push_back(std::move(paramText));
                }

                ReconstructSourceText(reconstructedDecl, *funcDecl);
            }
            else if (auto paramDecl = symbolInfo.symbolDecl->As<AstParamDecl>();
                     paramDecl && symbolInfo.symbolType == SymbolDeclType::Parameter) {
                declType = paramDecl->GetResolvedType();
                ReconstructSourceText(reconstructedDecl, *paramDecl);
            }
            else if (auto varDeclaratorDecl = symbolInfo.symbolDecl->As<AstVariableDeclaratorDecl>();
                     varDeclaratorDecl && (symbolInfo.symbolType == SymbolDeclType::GlobalVariable ||
                                           symbolInfo.symbolType == SymbolDeclType::LocalVariable)) {
                declType = varDeclaratorDecl->GetResolvedType();

                // We only compute value for declaration here. Const variable value in expression will be handled later.
                if (symbolInfo.isDeclaration && varDeclaratorDecl->IsConstVariable()) {
                    if (auto init = varDeclaratorDecl->GetInitializer(); init) {
                        exprValue = EvalAstInitializer(*init, declType);
                    }
                }

                ReconstructSourceText(reconstructedDecl, *varDeclaratorDecl->GetQualType(),
                                      varDeclaratorDecl->GetNameToken(), varDeclaratorDecl->GetArraySpec(),
                                      varDeclaratorDecl->GetInitializer());
            }
            else if (auto structFieldDeclaratorDecl = symbolInfo.symbolDecl->As<AstStructFieldDeclaratorDecl>();
                     structFieldDeclaratorDecl && symbolInfo.symbolType == SymbolDeclType::StructMember) {
                declType = structFieldDeclaratorDecl->GetResolvedType();
                ReconstructSourceText(reconstructedDecl, *structFieldDeclaratorDecl->GetQualType(),
                                      structFieldDeclaratorDecl->GetNameToken(),
                                      structFieldDeclaratorDecl->GetArraySpec(),
                                      structFieldDeclaratorDecl->GetInitializer());
            }
            else if (auto structDecl = symbolInfo.symbolDecl->As<AstStructDecl>();
                     structDecl && symbolInfo.symbolType == SymbolDeclType::Type) {
                // TODO: maybe show struct size and alignment?
                ReconstructSourceText(reconstructedDecl, *structDecl);
            }
            else if (auto blockFieldDeclaratorDecl = symbolInfo.symbolDecl->As<AstBlockFieldDeclaratorDecl>();
                     blockFieldDeclaratorDecl && symbolInfo.symbolType == SymbolDeclType::BlockMember) {
                declType = blockFieldDeclaratorDecl->GetResolvedType();
                ReconstructSourceText(reconstructedDecl, *blockFieldDeclaratorDecl->GetQualType(),
                                      blockFieldDeclaratorDecl->GetNameToken(),
                                      blockFieldDeclaratorDecl->GetArraySpec(),
                                      blockFieldDeclaratorDecl->GetInitializer());
            }
            else if (auto blockDecl = symbolInfo.symbolDecl->As<AstInterfaceBlockDecl>();
                     blockDecl && (symbolInfo.symbolType == SymbolDeclType::Block ||
                                   symbolInfo.symbolType == SymbolDeclType::BlockInstance)) {
                // TODO: maybe show block layouts?
                declType = blockDecl->GetResolvedBlockType();
                ReconstructSourceText(reconstructedDecl, *blockDecl);
            }
        }

        return HoverContent{
            .type        = symbolInfo.symbolType,
            .name        = symbolInfo.spelledText,
            .exprType    = exprType   ? exprType->GetDebugName().Str()
                           : declType ? declType->GetDebugName().Str()
                                      : "",
            .parameters  = std::move(reconstructedFuncParams),
            .exprValue   = !exprValue.IsError() ? exprValue.ToString() : "",
            .description = symbolInfo.symbolDecl ? info.QueryCommentDescription(*symbolInfo.symbolDecl) : "",
            .code        = std::move(reconstructedDecl),
            .range       = symbolInfo.spelledRange,
            .unknown     = !symbolInfo.symbolDecl && symbolInfo.symbolType != SymbolDeclType::Swizzle &&
                       symbolInfo.symbolType != SymbolDeclType::LayoutQualifier,
            .builtin = symbolInfo.symbolDecl && symbolInfo.symbolDecl->GetSyntaxRange().GetTranslationUnit() ==
                                                    TranslationUnitID::SystemPreamble,
        };
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
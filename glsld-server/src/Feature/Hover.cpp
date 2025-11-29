#include "Feature/Hover.h"

#include "Ast/Eval.h"
#include "Compiler/SyntaxToken.h"
#include "Server/SourceReconstruction.h"
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
                return "Global Variable";
            case SymbolDeclType::LocalVariable:
                return "Local Variable";
            case SymbolDeclType::Swizzle:
                return "Swizzle";
            case SymbolDeclType::StructMember:
                return "Struct Member";
            case SymbolDeclType::Parameter:
                return "Parameter";
            case SymbolDeclType::Function:
                return "Function";
            case SymbolDeclType::Type:
                return "Type";
            case SymbolDeclType::Block:
                return "Interface Block";
            case SymbolDeclType::BlockInstance:
                return "Interface Block Instance";
            case SymbolDeclType::BlockMember:
                return "Interface Block Member";
            }
        }();
        builder.AppendHeader(3, "{}{} `{}`",
                             hover.builtin   ? "Built-in "
                             : hover.unknown ? "Unknown "
                                             : "",
                             hoverTypeName, hover.name);

        // Hover Info
        if (!hover.symbolType.empty()) {
            if (hover.type == SymbolDeclType::Function) {
                builder.AppendParagraph("Return Type: `{}`", hover.symbolType);
            }
            else {
                builder.AppendParagraph("Type: `{}`", hover.symbolType);
            }
        }
        if (!hover.parameters.empty()) {
            builder.AppendParagraph("Parameters:");
            for (const auto& param : hover.parameters) {
                builder.AppendBullet("`{}`", param);
            }
            builder.AppendParagraph("");
        }
        if (!hover.symbolValue.empty()) {
            builder.AppendParagraph("Value: `{}`", hover.symbolValue);
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

        const Type* symbolType = nullptr;
        ConstValue symbolValue;
        if (auto expr = symbolInfo.astSymbolOccurrence->As<AstExpr>(); expr) {
            symbolType  = expr->GetDeducedType();
            symbolValue = EvalAstExpr(*expr);
        }
        else if (auto funcDecl = symbolInfo.astSymbolOccurrence->As<AstFunctionDecl>(); funcDecl) {
            symbolType = funcDecl->GetReturnType()->GetResolvedType();
        }
        else if (auto paramDecl = symbolInfo.astSymbolOccurrence->As<AstParamDecl>(); paramDecl) {
            symbolType = paramDecl->GetResolvedType();
        }
        else if (auto varDeclaratorDecl = symbolInfo.astSymbolOccurrence->As<AstVariableDeclaratorDecl>();
                 varDeclaratorDecl) {
            symbolType = varDeclaratorDecl->GetResolvedType();

            // We only compute value for declaration here. Const variable value in expression will be handled later.
            if (symbolInfo.isDeclaration && varDeclaratorDecl->IsConstVariable()) {
                if (auto init = varDeclaratorDecl->GetInitializer(); init) {
                    symbolValue = EvalAstInitializer(*init, symbolType);
                }
            }
        }
        else if (auto structFieldDeclaratorDecl = symbolInfo.astSymbolOccurrence->As<AstStructFieldDeclaratorDecl>();
                 structFieldDeclaratorDecl) {
            symbolType = structFieldDeclaratorDecl->GetResolvedType();
        }
        else if (auto blockFieldDeclaratorDecl = symbolInfo.astSymbolOccurrence->As<AstBlockFieldDeclaratorDecl>();
                 blockFieldDeclaratorDecl) {
            symbolType = blockFieldDeclaratorDecl->GetResolvedType();
        }
        else if (auto blockDecl = symbolInfo.astSymbolOccurrence->As<AstInterfaceBlockDecl>();
                 blockDecl && symbolInfo.symbolType == SymbolDeclType::BlockInstance) {
            symbolType = blockDecl->GetResolvedBlockType();
        }

        std::vector<std::string> reconstructedFuncParams;
        std::string reconstructedDecl;
        if (symbolInfo.symbolDecl) {
            SourceReconstructionBuilder reconstructBuilder;

            reconstructedDecl = reconstructBuilder.Print(*symbolInfo.symbolDecl);
            if (auto funcDecl = symbolInfo.symbolDecl->As<AstFunctionDecl>();
                funcDecl && symbolInfo.symbolType == SymbolDeclType::Function) {
                for (auto paramDecl : funcDecl->GetParams()) {
                    reconstructedFuncParams.push_back(reconstructBuilder.Print(*paramDecl));
                }
            }
        }

        return HoverContent{
            .type        = symbolInfo.symbolType,
            .name        = symbolInfo.spelledText,
            .symbolType  = symbolType ? symbolType->GetDebugName().Str() : "",
            .parameters  = std::move(reconstructedFuncParams),
            .symbolValue = !symbolValue.IsError() ? symbolValue.ToString() : "",
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
#include "Hover.h"
#include "Basic/Common.h"
#include "Compiler/SyntaxToken.h"
#include "Markdown.h"
#include "SourceText.h"
#include "StandardDocumentation.h"
#include <string>

namespace glsld
{
    static auto ComputeHoverText(const HoverContent& hover) -> std::string
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
        builder.Append(fmt::format("### {}{} `{}`\n", hover.unknown ? "Unknown " : "", hoverTypeName, hover.name));

        // Hover Info
        if (!hover.returnType.empty()) {
            builder.AppendParagraph(fmt::format("Return Type: `{}`", hover.returnType));
        }
        if (!hover.parameters.empty()) {
            builder.AppendParagraph("Parameters:");
            for (const auto& param : hover.parameters) {
                builder.Append(fmt::format(" - `{}`\n", param));
            }
        }
        if (!hover.exprType.empty() && hover.returnType.empty()) {
            // We omit the expression type if the return type is already shown.
            builder.AppendParagraph(fmt::format("Type: `{}`", hover.exprType));
        }
        if (!hover.exprValue.empty()) {
            builder.AppendParagraph(fmt::format("Value: `{}`", hover.exprValue));
        }

        // Description
        if (!hover.description.empty()) {
            builder.AppendRuler();
            builder.AppendParagraph(hover.description);
        }

        // Documentation
        if (!hover.documentation.empty()) {
            builder.AppendRuler();
            builder.AppendParagraph(hover.documentation);
        }

        // Reconstructed Code
        if (!hover.code.empty()) {
            builder.AppendRuler();
            builder.AppendCodeBlock(hover.code, "glsl");
        }

        return builder.Export();
    }

    static auto ComposeCommentDescription(TextRange declTextRange, ArrayView<RawCommentToken> preceedingComments,
                                          ArrayView<RawCommentToken> trailingComments) -> std::string
    {
        auto unwrapComment = [](const RawCommentToken& token) -> StringView {
            auto commentText = token.text.StrView();
            if (commentText.StartWith("//")) {
                return commentText.Drop(2).Trim();
            }
            else if (commentText.StartWith("/*")) {
                return commentText.Drop(2).DropBack(2);
            }
            return commentText;
        };

        // Case 1: Trailing comment in the same line
        // e.g. `int a; // comment`
        if (trailingComments.size() == 1 && declTextRange.GetNumLines() == 1 &&
            trailingComments.front().spelledRange.end.line == declTextRange.start.line) {
            return unwrapComment(trailingComments.front()).Str();
        }

        // Case 2: Preceeding comments in the previous lines
        // e.g. ```
        //      // comment
        //      int a;
        //      ```
        if (!preceedingComments.empty()) {
            std::string result;
            for (const auto& token : preceedingComments) {
                result += unwrapComment(token);
                result += "\n";
            }

            return result;
        }

        return "";
    }

    static auto CreateHoverContentForPPSymbol(const LanguageQueryProvider& provider,
                                              const SymbolQueryResult& symbolInfo) -> std::optional<HoverContent>
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
        else if (auto macroInfo = symbolInfo.ppSymbolOccurrence->GetMacroInfo(); macroInfo && macroInfo->definition) {
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
        else if (auto macroUsageInfo = symbolInfo.ppSymbolOccurrence->GetMacroInfo(); macroUsageInfo) {
            return HoverContent{
                .type        = SymbolDeclType::Macro,
                .name        = symbolInfo.spelledText,
                .description = "",
                // .code        = "",
                .range = symbolInfo.spelledRange,
            };
        }

        return std::nullopt;
    }

    static auto CreateHoverContentForAstSymbol(const LanguageQueryProvider& provider,
                                               const SymbolQueryResult& symbolInfo) -> std::optional<HoverContent>
    {
        GLSLD_ASSERT(symbolInfo.astSymbolOccurrence);
        std::string returnType;
        std::vector<std::string> parameters;
        std::string exprType;
        std::string exprValue;
        {
            if (!symbolInfo.symbolDecl.IsValid()) {
                // This branch is only used to filter out symbols without known declaration.
            }
            else if (auto funcDecl = symbolInfo.symbolDecl.GetDecl()->As<AstFunctionDecl>();
                     funcDecl && symbolInfo.symbolType == SymbolDeclType::Function) {
                returnType = funcDecl->GetReturnType()->GetResolvedType()->GetDebugName().Str();
                if (!funcDecl->GetParams().empty()) {
                    for (auto paramDecl : funcDecl->GetParams()) {
                        std::string paramText;
                        ReconstructSourceText(paramText, *paramDecl);
                        parameters.push_back(std::move(paramText));
                    }
                }
            }
            else if (auto paramDecl = symbolInfo.symbolDecl.GetDecl()->As<AstParamDecl>();
                     paramDecl && symbolInfo.symbolType == SymbolDeclType::Parameter) {
                exprType = paramDecl->GetResolvedType()->GetDebugName().Str();
            }
            else if (auto varDecl = symbolInfo.symbolDecl.GetDecl()->As<AstVariableDecl>();
                     varDecl && (symbolInfo.symbolType == SymbolDeclType::GlobalVariable ||
                                 symbolInfo.symbolType == SymbolDeclType::LocalVariable)) {
                const auto& declarator = varDecl->GetDeclarators()[symbolInfo.symbolDecl.GetIndex()];
                auto resolvedType      = varDecl->GetResolvedTypes()[symbolInfo.symbolDecl.GetIndex()];
                exprType               = resolvedType->GetDebugName().Str();

                // We only compute value for declaration here. Const variable value in expression will be handled later.
                if (symbolInfo.isDeclaration && varDecl->IsConstVariable()) {
                    if (auto init = declarator.initializer; init) {
                        exprValue = EvalAstInitializer(*init, resolvedType).ToString();
                    }
                }
            }
            else if (auto structMemberDecl = symbolInfo.symbolDecl.GetDecl()->As<AstStructFieldDecl>();
                     structMemberDecl && symbolInfo.symbolType == SymbolDeclType::StructMember) {
                const auto& declarator = structMemberDecl->GetDeclarators()[symbolInfo.symbolDecl.GetIndex()];
                auto resolvedType      = structMemberDecl->GetResolvedTypes()[symbolInfo.symbolDecl.GetIndex()];
                exprType               = resolvedType->GetDebugName().Str();
            }
            else if (auto structDecl = symbolInfo.symbolDecl.GetDecl()->As<AstStructDecl>();
                     structDecl && symbolInfo.symbolType == SymbolDeclType::Type) {
                // TODO: maybe show struct size and alignment?
            }
            else if (auto blockMemberDecl = symbolInfo.symbolDecl.GetDecl()->As<AstBlockFieldDecl>();
                     blockMemberDecl && symbolInfo.symbolType == SymbolDeclType::BlockMember) {
                const auto& declarator = blockMemberDecl->GetDeclarators()[symbolInfo.symbolDecl.GetIndex()];
                auto resolvedType      = blockMemberDecl->GetResolvedTypes()[symbolInfo.symbolDecl.GetIndex()];
                exprType               = resolvedType->GetDebugName().Str();
            }
            else if (auto blockDecl = symbolInfo.symbolDecl.GetDecl()->As<AstInterfaceBlockDecl>();
                     blockDecl && (symbolInfo.symbolType == SymbolDeclType::Block ||
                                   symbolInfo.symbolType == SymbolDeclType::BlockInstance)) {
                // TODO: maybe show block layouts?
            }

            if (symbolInfo.astSymbolOccurrence) {
                if (auto expr = symbolInfo.astSymbolOccurrence->As<AstExpr>(); expr) {
                    if (auto constValue = EvalAstExpr(*expr); !constValue.IsError()) {
                        exprValue = constValue.ToString();
                    }
                }
            }
        }

        if (!symbolInfo.symbolDecl.IsValid()) {
            bool isUnknown = symbolInfo.symbolType != SymbolDeclType::Swizzle &&
                             symbolInfo.symbolType != SymbolDeclType::LayoutQualifier;

            return HoverContent{
                .type        = symbolInfo.symbolType,
                .name        = symbolInfo.spelledText,
                .returnType  = returnType,
                .parameters  = parameters,
                .exprType    = exprType,
                .exprValue   = exprValue,
                .description = "",
                .code        = "",
                .range       = symbolInfo.spelledRange,
                .unknown     = isUnknown,
            };
        }

        auto decl = symbolInfo.symbolDecl.GetDecl();
        std::string description =
            ComposeCommentDescription(provider.LookupExpandedTextRange(decl->GetSyntaxRange()),
                                      provider.LookupPreceedingComment(decl->GetSyntaxRange().GetBeginID()),
                                      provider.LookupPreceedingComment(decl->GetSyntaxRange().GetEndID()));
        std::string documentation;
        std::string codeBuffer;
        if (auto funcDecl = decl->As<AstFunctionDecl>();
            funcDecl && symbolInfo.symbolType == SymbolDeclType::Function) {
            ReconstructSourceText(codeBuffer, *funcDecl);
            documentation = QueryFunctionDocumentation(funcDecl->GetNameToken().text.StrView()).Str();
        }
        else if (auto paramDecl = decl->As<AstParamDecl>();
                 paramDecl && symbolInfo.symbolType == SymbolDeclType::Parameter) {
            ReconstructSourceText(codeBuffer, *paramDecl);
        }
        else if (auto varDecl = decl->As<AstVariableDecl>();
                 varDecl && (symbolInfo.symbolType == SymbolDeclType::GlobalVariable ||
                             symbolInfo.symbolType == SymbolDeclType::LocalVariable)) {
            ReconstructSourceText(codeBuffer, *varDecl, symbolInfo.symbolDecl.GetIndex());
        }
        else if (auto structMemberDecl = decl->As<AstStructFieldDecl>();
                 structMemberDecl && symbolInfo.symbolType == SymbolDeclType::StructMember) {
            ReconstructSourceText(codeBuffer, *structMemberDecl, symbolInfo.symbolDecl.GetIndex());
        }
        else if (auto structDecl = decl->As<AstStructDecl>();
                 structDecl && symbolInfo.symbolType == SymbolDeclType::Type) {
            ReconstructSourceText(codeBuffer, *structDecl);
        }
        else if (auto blockMemberDecl = decl->As<AstBlockFieldDecl>();
                 blockMemberDecl && symbolInfo.symbolType == SymbolDeclType::BlockMember) {
            ReconstructSourceText(codeBuffer, *blockMemberDecl, symbolInfo.symbolDecl.GetIndex());
        }
        else if (auto blockDecl = decl->As<AstInterfaceBlockDecl>();
                 blockDecl && (symbolInfo.symbolType == SymbolDeclType::Block ||
                               symbolInfo.symbolType == SymbolDeclType::BlockInstance)) {
            ReconstructSourceText(codeBuffer, *blockDecl);
        }
        else {
            return std::nullopt;
        }

        return HoverContent{
            .type        = symbolInfo.symbolType,
            .name        = symbolInfo.spelledText,
            .returnType  = returnType,
            .parameters  = parameters,
            .exprType    = exprType,
            .exprValue   = exprValue,
            .description = description,
            .code        = codeBuffer,
            .range       = symbolInfo.spelledRange,
        };
    }

    auto QueryHoverContent(const LanguageQueryProvider& provider, TextPosition position) -> std::optional<HoverContent>
    {
        const auto& compilerObject = provider.GetCompilerResult();

        if (auto symbolInfo = QuerySymbolByPosition(provider, position); symbolInfo) {
            // FIXME: is the following needed?
            // if (symbolInfo->token.IsKeyword()) {
            //     // Don't provide hover for keyword
            //     return std::nullopt;
            // }

            if (symbolInfo->ppSymbolOccurrence) {
                return CreateHoverContentForPPSymbol(provider, *symbolInfo);
            }
            else if (symbolInfo->astSymbolOccurrence) {
                return CreateHoverContentForAstSymbol(provider, *symbolInfo);
            }
        }

        return std::nullopt;
    }

    namespace lsp
    {
        auto ComputeHover(const LanguageQueryProvider& provider, lsp::Position position) -> std::optional<Hover>
        {
            return QueryHoverContent(provider, FromLspPosition(position))
                .transform([](const HoverContent& hoverContent) {
                    return lsp::Hover{
                        .contents = ComputeHoverText(hoverContent),
                        .range    = ToLspRange(hoverContent.range),
                    };
                });
        }

    } // namespace lsp

} // namespace glsld
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
            case SymbolDeclType::LayoutQualifier:
                return "Layout Qualifier";
            case SymbolDeclType::GlobalVariable:
                return "Global Variable";
            case SymbolDeclType::LocalVariable:
                return "Local Variable";
            case SymbolDeclType::Swizzle:
                return "Swizzle";
            case SymbolDeclType::StructMember:
                return "Member Variable";
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
            case SymbolDeclType::Unknown:
                GLSLD_UNREACHABLE();
            }
        }();
        builder.Append(fmt::format("### {}{} `{}`\n", hover.unknown ? "Unknown " : "", hoverTypeName, hover.name));

        if (!hover.hoverInfo.empty()) {
            builder.AppendRuler();
            builder.AppendParagraph(hover.hoverInfo);
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

    static auto CreateHoverContent(const LanguageQueryProvider& provider, const SymbolQueryResult& symbolInfo)
        -> std::optional<HoverContent>
    {
        if (!symbolInfo.token.IsIdentifier()) {
            return std::nullopt;
        }

        StringView name = symbolInfo.token.text.StrView();
        auto tokenRange = provider.LookupExpandedTextRange(symbolInfo.token);

        std::string hoverInfo;
        if (auto expr = symbolInfo.symbolOwner->As<AstExpr>(); expr) {
            hoverInfo += fmt::format("Type: `{}`\n\n", expr->GetDeducedType()->GetDebugName());
            if (auto constValue = EvalAstExpr(*expr); !constValue.IsError()) {
                hoverInfo += fmt::format("Value: `{}`\n\n", constValue.ToString());
            }
        }
        else if (auto varDecl = symbolInfo.symbolOwner->As<AstVariableDecl>(); varDecl) {
            const auto& declarator = varDecl->GetDeclarators()[symbolInfo.symbolDecl.GetIndex()];
            auto resolvedType      = varDecl->GetResolvedTypes()[symbolInfo.symbolDecl.GetIndex()];
            hoverInfo += fmt::format("Type: `{}`\n\n", resolvedType->GetDebugName());
            if (auto init = declarator.initializer; init && varDecl->IsConstVariable()) {
                hoverInfo += fmt::format("Value: `{}`\n\n", EvalAstInitializer(*init, resolvedType).ToString());
            }
        }

        if (!symbolInfo.symbolDecl.IsValid()) {
            bool isUnknown = symbolInfo.symbolType != SymbolDeclType::Swizzle &&
                             symbolInfo.symbolType != SymbolDeclType::LayoutQualifier;

            return HoverContent{
                .type        = symbolInfo.symbolType,
                .name        = name.Str(),
                .hoverInfo   = std::move(hoverInfo),
                .description = "",
                .code        = "",
                .range       = tokenRange,
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
            .name        = name.Str(),
            .hoverInfo   = std::move(hoverInfo),
            .description = std::move(description),
            .code        = std::move(codeBuffer),
            .range       = tokenRange,
        };
    }

    auto QueryHoverContent(const LanguageQueryProvider& provider, TextPosition position) -> std::optional<HoverContent>
    {
        const auto& compilerObject = provider.GetCompilerResult();

        if (auto symbolInfo = QuerySymbolByPosition(provider, position); symbolInfo) {
            // Decl token that's either builtin or unknown
            if (symbolInfo->symbolType == SymbolDeclType::Unknown) {
                return std::nullopt;
            }

            // FIXME: is the following needed?
            if (symbolInfo->token.IsKeyword()) {
                // Don't provide hover for keyword
                return std::nullopt;
            }

            return CreateHoverContent(provider, *symbolInfo);
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
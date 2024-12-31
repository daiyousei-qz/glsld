#include "Hover.h"
#include "Basic/Common.h"
#include "Compiler/SyntaxToken.h"
#include "LanguageService.h"
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
        builder.Append("### ");
        if (hover.unknown) {
            builder.Append("Unknown ");
        }
        switch (hover.type) {
        case SymbolAccessType::LayoutQualifier:
            builder.Append("Layout Qualifier");
            break;
        case SymbolAccessType::GlobalVariable:
            builder.Append("Global Variable");
            break;
        case SymbolAccessType::LocalVariable:
            builder.Append("Local Variable");
            break;
        case SymbolAccessType::Swizzle:
            builder.Append("Swizzle");
            break;
        case SymbolAccessType::MemberVariable:
            builder.Append("Member Variable");
            break;
        case SymbolAccessType::Parameter:
            builder.Append("Parameter");
            break;
        case SymbolAccessType::Function:
            builder.Append("Function");
            break;
        case SymbolAccessType::Type:
            builder.Append("Type");
            break;
        case SymbolAccessType::InterfaceBlock:
            builder.Append("Interface Block");
            break;
        case SymbolAccessType::InterfaceBlockInstance:
            builder.Append("Interface Block Instance");
            break;
        case SymbolAccessType::Unknown:
            GLSLD_UNREACHABLE();
        }
        builder.Append(fmt::format(" `{}`", hover.name));

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

    static auto CreateHoverContent(const LanguageQueryProvider& provider, const SymbolAccessInfo& accessInfo)
        -> std::optional<HoverContent>
    {
        if (!accessInfo.token.IsIdentifier()) {
            return std::nullopt;
        }

        StringView name = accessInfo.token.text.StrView();
        auto tokenRange = provider.LookupExpandedTextRange(accessInfo.token);
        if (!accessInfo.symbolDecl.IsValid()) {
            bool isUnknown = accessInfo.symbolType != SymbolAccessType::Swizzle &&
                             accessInfo.symbolType != SymbolAccessType::LayoutQualifier;

            return HoverContent{
                .type        = accessInfo.symbolType,
                .name        = name.Str(),
                .description = "",
                .code        = "",
                .range       = tokenRange,
                .unknown     = isUnknown,
            };
        }

        auto decl = accessInfo.symbolDecl.GetDecl();
        std::string description =
            ComposeCommentDescription(provider.LookupExpandedTextRange(decl->GetSyntaxRange()),
                                      provider.LookupPreceedingComment(decl->GetSyntaxRange().GetBeginID()),
                                      provider.LookupPreceedingComment(decl->GetSyntaxRange().GetEndID()));
        std::string codeBuffer;
        if (auto funcDecl = decl->As<AstFunctionDecl>();
            funcDecl && accessInfo.symbolType == SymbolAccessType::Function) {
            ReconstructSourceText(codeBuffer, *funcDecl);
            return HoverContent{
                .type          = accessInfo.symbolType,
                .name          = name.Str(),
                .description   = std::move(description),
                .documentation = QueryFunctionDocumentation(name).Str(),
                .code          = std::move(codeBuffer),
                .range         = tokenRange,
            };
        }
        else if (auto paramDecl = decl->As<AstParamDecl>();
                 paramDecl && accessInfo.symbolType == SymbolAccessType::Parameter) {
            ReconstructSourceText(codeBuffer, *paramDecl);
            return HoverContent{
                .type        = accessInfo.symbolType,
                .name        = name.Str(),
                .description = std::move(description),
                .code        = std::move(codeBuffer),
                .range       = tokenRange,
            };
        }
        else if (auto varDecl = decl->As<AstVariableDecl>();
                 varDecl && (accessInfo.symbolType == SymbolAccessType::GlobalVariable ||
                             accessInfo.symbolType == SymbolAccessType::LocalVariable)) {
            ReconstructSourceText(codeBuffer, *varDecl, accessInfo.symbolDecl.GetIndex());
            return HoverContent{
                .type        = accessInfo.symbolType,
                .name        = name.Str(),
                .description = std::move(description),
                .code        = std::move(codeBuffer),
                .range       = tokenRange,
            };
        }
        else if (auto memberDecl = decl->As<AstFieldDecl>();
                 memberDecl && accessInfo.symbolType == SymbolAccessType::MemberVariable) {
            ReconstructSourceText(codeBuffer, *memberDecl, accessInfo.symbolDecl.GetIndex());
            return HoverContent{
                .type        = accessInfo.symbolType,
                .name        = name.Str(),
                .description = std::move(description),
                .code        = std::move(codeBuffer),
                .range       = tokenRange,
            };
        }
        else if (auto structDecl = decl->As<AstStructDecl>();
                 structDecl && accessInfo.symbolType == SymbolAccessType::Type) {
            ReconstructSourceText(codeBuffer, *structDecl);
            return HoverContent{
                .type        = accessInfo.symbolType,
                .name        = name.Str(),
                .description = std::move(description),
                .code        = std::move(codeBuffer),
                .range       = tokenRange,
            };
        }
        else if (auto blockDecl = decl->As<AstInterfaceBlockDecl>();
                 blockDecl && (accessInfo.symbolType == SymbolAccessType::InterfaceBlock ||
                               accessInfo.symbolType == SymbolAccessType::InterfaceBlockInstance)) {
            ReconstructSourceText(codeBuffer, *blockDecl);
            return HoverContent{
                .type        = accessInfo.symbolType,
                .name        = name.Str(),
                .description = std::move(description),
                .code        = std::move(codeBuffer),
                .range       = tokenRange,
            };
        }

        return std::nullopt;
    }

    auto ComputeHoverContent(const LanguageQueryProvider& provider, TextPosition position)
        -> std::optional<HoverContent>
    {
        const auto& compilerObject = provider.GetCompilerResult();

        auto accessInfo = provider.QuerySymbolByPosition(position);
        if (accessInfo) {

            // Decl token that's either builtin or unknown
            if (accessInfo->symbolType == SymbolAccessType::Unknown) {
                return std::nullopt;
            }

            // FIXME: is the following needed?
            if (accessInfo->token.IsKeyword()) {
                // Don't provide hover for keyword
                return std::nullopt;
            }

            return CreateHoverContent(provider, *accessInfo);
        }

        return std::nullopt;
    }

    auto ComputeHover(const LanguageQueryProvider& provider, lsp::Position position) -> std::optional<lsp::Hover>
    {
        return ComputeHoverContent(provider, FromLspPosition(position)).transform([](const HoverContent& hoverContent) {
            return lsp::Hover{
                .contents = ComputeHoverText(hoverContent),
                .range    = ToLspRange(hoverContent.range),
            };
        });
    }

} // namespace glsld
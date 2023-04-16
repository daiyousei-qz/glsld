#include "LanguageService.h"
#include "AstHelper.h"
#include "Markdown.h"
#include "SourceText.h"
#include "StandardDocumentation.h"

namespace glsld
{
    struct HoverContent
    {
        DeclTokenType type;
        std::string name;
        std::string description;
        std::string documentation;
        std::string code;
        bool unknown = false;
    };

    static auto ComputeHoverText(const HoverContent& hover) -> std::string
    {
        MarkdownBuilder builder;

        // Header explaining what the hover symbol is
        builder.Append("### ");
        if (hover.unknown) {
            builder.Append("Unknown ");
        }
        switch (hover.type) {
        case DeclTokenType::LayoutQualifier:
            builder.Append("Layout Qualifier");
            break;
        case DeclTokenType::Variable:
            builder.Append("Variable");
            break;
        case DeclTokenType::Swizzle:
            builder.Append("Swizzle");
            break;
        case DeclTokenType::MemberVariable:
            builder.Append("Member Variable");
            break;
        case DeclTokenType::Parameter:
            builder.Append("Parameter");
            break;
        case DeclTokenType::Function:
            builder.Append("Function");
            break;
        case DeclTokenType::Type:
            builder.Append("Type");
            break;
        case DeclTokenType::InterfaceBlock:
            builder.Append("Interface Block");
            break;
        case DeclTokenType::Unknown:
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

    static auto CreateHoverContent(DeclTokenType type, StringView name, bool unknown = true)
    {
        return HoverContent{
            .type        = type,
            .name        = name.Str(),
            .description = "",
            .code        = "",
            .unknown     = unknown,
        };
    }

    static auto CreateHoverContentFromDecl(DeclView declView, StringView hoverId) -> std::optional<HoverContent>
    {
        if (!declView.IsValid()) {
            return std::nullopt;
        }

        AstDecl& decl = *declView.GetDecl();

        HoverContent hover;
        std::string descriptionBuffer;
        std::string codeBuffer;
        if (auto funcDecl = decl.As<AstFunctionDecl>()) {
            ReconstructSourceText(codeBuffer, *funcDecl);
            return HoverContent{
                .type          = DeclTokenType::Function,
                .name          = hoverId.Str(),
                .description   = std::move(descriptionBuffer),
                .documentation = QueryFunctionDocumentation(hoverId).Str(),
                .code          = std::move(codeBuffer),
            };
        }
        else if (auto paramDecl = decl.As<AstParamDecl>()) {
            ReconstructSourceText(codeBuffer, *paramDecl);
            return HoverContent{
                .type        = DeclTokenType::Parameter,
                .name        = hoverId.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }
        else if (auto varDecl = decl.As<AstVariableDecl>()) {
            // FIXME: use correct declarator index
            ReconstructSourceText(codeBuffer, *varDecl, 0);
            return HoverContent{
                .type        = DeclTokenType::Variable,
                .name        = hoverId.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }
        else if (auto memberDecl = decl.As<AstStructMemberDecl>()) {
            ReconstructSourceText(codeBuffer, *memberDecl, declView.GetIndex());
            return HoverContent{
                .type        = DeclTokenType::MemberVariable,
                .name        = hoverId.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }
        else if (auto structDecl = decl.As<AstStructDecl>()) {
            ReconstructSourceText(codeBuffer, *structDecl);
            return HoverContent{
                .type        = DeclTokenType::Type,
                .name        = hoverId.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }
        else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>()) {
            ReconstructSourceText(codeBuffer, *blockDecl);
            return HoverContent{
                .type        = DeclTokenType::InterfaceBlock,
                .name        = hoverId.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }

        return std::nullopt;
    }

    auto ComputeHover(const CompilerObject& compilerObject, lsp::Position position) -> std::optional<lsp::Hover>
    {
        auto declTokenResult = FindDeclToken(compilerObject, FromLspPosition(position));
        if (declTokenResult) {
            auto hoverIdentifier = declTokenResult->token.text.StrView();
            auto hoverRange      = declTokenResult->range;

            if (declTokenResult->accessedDecl.IsValid()) {
                // Decl token with resolved user declaration
                GLSLD_ASSERT(declTokenResult->token.IsIdentifier());
                auto hoverContent = CreateHoverContentFromDecl(declTokenResult->accessedDecl, hoverIdentifier);
                if (hoverContent) {
                    return lsp::Hover{
                        .contents = ComputeHoverText(*hoverContent),
                        .range    = ToLspRange(hoverRange),
                    };
                }
            }
            else {
                // Decl token that's either builtin or unknown
                if (declTokenResult->accessType == DeclTokenType::Unknown) {
                    return std::nullopt;
                }

                // FIXME: is the following needed?
                if (declTokenResult->token.IsKeyword()) {
                    // Don't provide hover for keyword
                    return std::nullopt;
                }

                bool isUnknown = declTokenResult->accessType != DeclTokenType::Swizzle &&
                                 declTokenResult->accessType != DeclTokenType::LayoutQualifier;
                auto hoverType    = declTokenResult->accessType;
                auto hoverContent = CreateHoverContent(hoverType, hoverIdentifier, isUnknown);
                return lsp::Hover{
                    .contents = ComputeHoverText(hoverContent),
                    .range    = ToLspRange(hoverRange),
                };
            }
        }

        return std::nullopt;
    }

} // namespace glsld
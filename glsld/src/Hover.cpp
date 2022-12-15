#include "LanguageService.h"
#include "AstHelper.h"
#include "Markdown.h"
#include "SourceText.h"

namespace glsld
{
    struct HoverContent
    {
        DeclTokenType type;
        std::string name;
        std::string code;
        bool unknown = false;
    };

    static auto ComputeHoverText(const HoverContent& hover) -> std::string
    {
        MarkdownBuilder builder;
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

        if (!hover.code.empty()) {
            builder.AppendRuler();
            builder.Append("```glsl\n");
            builder.Append(hover.code);
            builder.Append("\n```");
        }

        return builder.Export();
    }

    static auto CreateHoverContent(DeclTokenType type, std::string_view name, bool unknown = true)
    {
        return HoverContent{
            .type    = type,
            .name    = std::string{name},
            .code    = "",
            .unknown = unknown,
        };
    }

    static auto GetHoverContent(DeclView declView, std::string_view hoverId) -> std::optional<HoverContent>
    {
        if (!declView.IsValid()) {
            return std::nullopt;
        }

        AstDecl& decl = *declView.GetDecl();

        HoverContent hover;
        std::string buffer;
        if (auto funcDecl = decl.As<AstFunctionDecl>()) {
            ReconstructSourceText(buffer, *funcDecl);
            return HoverContent{
                .type = DeclTokenType::Function,
                .name = std::string{hoverId},
                .code = std::move(buffer),
            };
        }
        else if (auto paramDecl = decl.As<AstParamDecl>()) {
            ReconstructSourceText(buffer, *paramDecl);
            return HoverContent{
                .type = DeclTokenType::Parameter,
                .name = std::string{hoverId},
                .code = std::move(buffer),
            };
        }
        else if (auto varDecl = decl.As<AstVariableDecl>()) {
            // FIXME: use correct declarator index
            ReconstructSourceText(buffer, *varDecl, 0);
            return HoverContent{
                .type = DeclTokenType::Variable,
                .name = std::string{hoverId},
                .code = std::move(buffer),
            };
        }
        else if (auto memberDecl = decl.As<AstStructMemberDecl>()) {
            ReconstructSourceText(buffer, *memberDecl, declView.GetIndex());
            return HoverContent{
                .type = DeclTokenType::MemberVariable,
                .name = std::string{hoverId},
                .code = std::move(buffer),
            };
        }
        else if (auto structDecl = decl.As<AstStructDecl>()) {
            ReconstructSourceText(buffer, *structDecl);
            return HoverContent{
                .type = DeclTokenType::Type,
                .name = std::string{hoverId},
                .code = std::move(buffer),
            };
        }
        else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>()) {
            return HoverContent{
                .type = DeclTokenType::InterfaceBlock,
                .name = std::string{hoverId},
                .code = fmt::format("block {}", hoverId),
            };
        }

        return std::nullopt;
    }

    auto ComputeHover(CompiledModule& compiler, lsp::Position position) -> std::optional<lsp::Hover>
    {
        auto declTokenResult = FindDeclToken(compiler, FromLspPosition(position));
        if (declTokenResult) {
            auto hoverIdentifier = declTokenResult->token.text.StrView();
            auto hoverRange      = declTokenResult->range;

            if (declTokenResult->accessedDecl.IsValid()) {
                // Decl token with resolved user declaration
                GLSLD_ASSERT(declTokenResult->token.IsIdentifier());
                auto hoverContent = GetHoverContent(declTokenResult->accessedDecl, hoverIdentifier);
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
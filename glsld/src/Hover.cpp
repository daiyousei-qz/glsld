#include "LanguageService.h"
#include "AstHelper.h"
#include "Markdown.h"
#include "SourceText.h"

namespace glsld
{
    enum class HoverType
    {
        Variable,
        Swizzle,
        MemberVariable,
        Parameter,
        Function,
        Type,
    };

    struct HoverContent
    {
        HoverType type;
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
        case HoverType::Variable:
            builder.Append("Variable");
            break;
        case HoverType::Swizzle:
            builder.Append("Swizzle");
            break;
        case HoverType::MemberVariable:
            builder.Append("Member Variable");
            break;
        case HoverType::Parameter:
            builder.Append("Parameter");
            break;
        case HoverType::Function:
            builder.Append("Function");
            break;
        case HoverType::Type:
            builder.Append("Type");
            break;
        default:
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

    static auto CreateHoverContent(HoverType type, std::string_view name, bool unknown = true)
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
                .type = HoverType::Function,
                .name = std::string{hoverId},
                .code = std::move(buffer),
            };
        }
        else if (auto paramDecl = decl.As<AstParamDecl>()) {
            ReconstructSourceText(buffer, *paramDecl);
            return HoverContent{
                .type = HoverType::Parameter,
                .name = std::string{hoverId},
                .code = std::move(buffer),
            };
        }
        else if (auto varDecl = decl.As<AstVariableDecl>()) {
            // FIXME: use correct declarator index
            ReconstructSourceText(buffer, *varDecl, 0);
            return HoverContent{
                .type = HoverType::Variable,
                .name = std::string{hoverId},
                .code = std::move(buffer),
            };
        }
        else if (auto memberDecl = decl.As<AstStructMemberDecl>()) {
            ReconstructSourceText(buffer, *memberDecl, declView.GetIndex());
            return HoverContent{
                .type = HoverType::MemberVariable,
                .name = std::string{hoverId},
                .code = std::move(buffer),
            };
        }
        else if (auto structDecl = decl.As<AstStructDecl>()) {
            return HoverContent{
                .type = HoverType::Type,
                .name = std::string{hoverId},
                .code = fmt::format("struct  {}", hoverId),
            };
        }

        return std::nullopt;
    }

    auto TranslateHoverType(NameAccessType type) -> HoverType
    {
        switch (type) {
        case NameAccessType::Unknown:
        case NameAccessType::Variable:
            return HoverType::Variable;
        case NameAccessType::Swizzle:
            return HoverType::Swizzle;
        case NameAccessType::Constructor:
            return HoverType::Type;
        case NameAccessType::Function:
            return HoverType::Function;
        default:
            GLSLD_UNREACHABLE();
        }
    }

    auto ComputeHover(CompiledModule& compiler, lsp::Position position) -> std::optional<lsp::Hover>
    {
        auto declTokenResult = FindDeclToken(compiler, FromLspPosition(position));
        if (declTokenResult) {
            auto hoverIdentifier = declTokenResult->token.text.StrView();
            auto hoverRange      = declTokenResult->range;

            if (declTokenResult->accessedDecl.IsValid()) {
                // Decl token with resolved user declaration
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
                if (declTokenResult->accessType == NameAccessType::Unknown) {
                    return std::nullopt;
                }

                // FIXME: is the following needed?
                if (declTokenResult->token.IsKeyword()) {
                    // Don't provide hover for keyword
                    return std::nullopt;
                }

                bool isUnknown    = declTokenResult->accessType != NameAccessType::Swizzle;
                auto hoverType    = TranslateHoverType(declTokenResult->accessType);
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
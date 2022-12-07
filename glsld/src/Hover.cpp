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

    static auto GetHoverContent(AstDecl& decl, std::string_view hoverId) -> std::optional<HoverContent>
    {
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
            return HoverContent{
                .type = HoverType::MemberVariable,
                .name = std::string{hoverId},
                .code = fmt::format("{} {}", ReconstructSourceText(memberDecl->GetType()), hoverId),
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

    auto ComputeHover(CompiledModule& compiler, lsp::Position position) -> std::optional<lsp::Hover>
    {
        struct HoverProcessor : public DeclTokenCallback<lsp::Hover>
        {
            auto ProcessToken(SyntaxToken token, TextRange range, AstDecl& decl) const
                -> std::optional<lsp::Hover> override
            {
                auto hoverContent = GetHoverContent(decl, token.text.StrView());
                if (hoverContent) {
                    return lsp::Hover{
                        .contents = ComputeHoverText(*hoverContent),
                        .range    = ToLspRange(range),
                    };
                }

                return std::nullopt;
            }
            auto ProcessTokenWithoutDecl(SyntaxToken token, TextRange range, NameAccessType type) const
                -> std::optional<lsp::Hover> override
            {
                if (type == NameAccessType::Unknown) {
                    return std::nullopt;
                }
                if (token.IsKeyword()) {
                    // Don't provide hover for keyword
                    return std::nullopt;
                }

                bool isUnknown    = type != NameAccessType::Swizzle;
                auto hoverContent = CreateHoverContent(TranslateHoverType(type), token.text.StrView(), isUnknown);
                return lsp::Hover{
                    .contents = ComputeHoverText(hoverContent),
                    .range    = ToLspRange(range),
                };
            }

            auto TranslateHoverType(NameAccessType type) const -> HoverType
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
        };

        return ProcessDeclToken(compiler, FromLspPosition(position), HoverProcessor{});
    }

} // namespace glsld
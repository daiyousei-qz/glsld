#include "LanguageService.h"
#include "AstHelper.h"
#include "Markdown.h"
#include "SourceText.h"

namespace glsld
{
    enum class HoverType
    {
        Variable,
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

    static auto ReconstructSourceText(const SyntaxToken& tok) -> std::string_view
    {
        if (tok.klass != TokenKlass::Error) {
            return tok.text.StrView();
        }
        else {
            return "<error>";
        }
    }

    static auto ReconstructSourceText(AstQualType* type) -> std::string
    {
        std::string result;
        if (auto structDecl = type->GetStructDecl()) {
            result += "struct ";
            if (structDecl->GetDeclToken()) {
                result += ReconstructSourceText(*structDecl->GetDeclToken());
            }
            result += " { ... }";
        }
        else {
            result += ReconstructSourceText(type->GetTypeNameTok());
        }

        return result;
    }
    static auto ReconstructSourceText(std::span<AstParamDecl* const> params) -> std::string
    {
        std::string result;
        result += "(";
        for (auto param : params) {
            result += ReconstructSourceText(param->GetType());
            if (param->GetDeclTok()) {
                result += " ";
                result += ReconstructSourceText(*param->GetDeclTok());
            }
            result += ",";
        }
        if (result.ends_with(',')) {
            result.pop_back();
        }
        result += ")";

        return result;
    }

    static auto CreateUnknownHoverContent(HoverType type, std::string_view name)
    {
        return HoverContent{
            .type    = type,
            .name    = std::string{name},
            .code    = "",
            .unknown = true,
        };
    }

    static auto GetHoverContent(AstDecl& decl, std::string_view hoverId) -> std::optional<HoverContent>
    {
        HoverContent hover;
        if (auto funcDecl = decl.As<AstFunctionDecl>()) {
            return HoverContent{
                .type = HoverType::Function,
                .name = std::string{hoverId},
                .code = fmt::format("{} {}{}", ReconstructSourceText(funcDecl->GetReturnType()), hoverId,
                                    ReconstructSourceText(funcDecl->GetParams())),
            };
        }
        else if (auto paramDecl = decl.As<AstParamDecl>()) {
            return HoverContent{
                .type = HoverType::Parameter,
                .name = std::string{hoverId},
                .code = fmt::format("{} {}", ReconstructSourceText(paramDecl->GetType()), hoverId),
            };
        }
        else if (auto varDecl = decl.As<AstVariableDecl>()) {
            // FIXME: array spec
            return HoverContent{
                .type = HoverType::Variable,
                .name = std::string{hoverId},
                .code = fmt::format("{} {}", ReconstructSourceText(varDecl->GetType()), hoverId),
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

    static auto GetHoverContent(AstNameAccessExpr& expr) -> std::optional<HoverContent>
    {
        if (expr.GetAccessName().klass != TokenKlass::Identifier) {
            return std::nullopt;
        }

        auto accessName = expr.GetAccessName().text.StrView();
        if (expr.GetAccessedDecl()) {
            auto accessDecl = expr.GetAccessedDecl();
            auto declHover  = GetHoverContent(*accessDecl, accessName);
            if (declHover) {
                return std::move(declHover);
            }
        }

        switch (expr.GetAccessType()) {
        case NameAccessType::Function:
            return CreateUnknownHoverContent(HoverType::Function, accessName);
        case NameAccessType::Constructor:
            return CreateUnknownHoverContent(HoverType::Type, accessName);
        case NameAccessType::Variable:
            return CreateUnknownHoverContent(HoverType::Variable, accessName);
        case NameAccessType::Unknown:
            return std::nullopt;
        default:
            GLSLD_UNREACHABLE();
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
                        .range    = TextRange::ToLspRange(range),
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

                auto hoverContent = CreateUnknownHoverContent(TranslateHoverType(type), token.text.StrView());
                return lsp::Hover{
                    .contents = ComputeHoverText(hoverContent),
                    .range    = TextRange::ToLspRange(range),
                };
            }

            auto TranslateHoverType(NameAccessType type) const -> HoverType
            {
                switch (type) {
                case NameAccessType::Unknown:
                case NameAccessType::Variable:
                    return HoverType::Variable;
                case NameAccessType::Constructor:
                    return HoverType::Type;
                case NameAccessType::Function:
                    return HoverType::Function;
                default:
                    GLSLD_UNREACHABLE();
                }
            }
        };

        return ProcessDeclToken(compiler, TextPosition::FromLspPosition(position), HoverProcessor{});
    }

} // namespace glsld
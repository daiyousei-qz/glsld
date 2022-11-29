#include "LanguageService.h"
#include "AstVisitor.h"

namespace glsld
{
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
            result += " ";
            result += ReconstructSourceText(param->GetDeclTok());
            result += ",";
        }
        if (result.ends_with(',')) {
            result.pop_back();
        }
        result += ")";

        return result;
    }

    static auto GetHoverContent(AstDecl& decl, std::string_view hoverId) -> std::optional<std::string>
    {
        if (auto funcDecl = decl.As<AstFunctionDecl>()) {
            return fmt::format("Function: `{} {}{}`", ReconstructSourceText(funcDecl->GetReturnType()), hoverId,
                               ReconstructSourceText(funcDecl->GetParams()));
        }
        else if (auto paramDecl = decl.As<AstParamDecl>()) {
            return fmt::format("Param: `{} {}`", ReconstructSourceText(paramDecl->GetType()), hoverId);
        }
        else if (auto varDecl = decl.As<AstVariableDecl>()) {
            // FIXME: array spec
            return fmt::format("Variable: `{} {}`", ReconstructSourceText(varDecl->GetType()), hoverId);
        }

        return std::nullopt;
    }

    static auto GetHoverContent(AstNameAccessExpr& expr) -> std::optional<std::string>
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
            return fmt::format("Unknown function `{}`", accessName);
        case NameAccessType::Constructor:
            return fmt::format("Unknown type `{}`", accessName);
        case NameAccessType::Variable:
            return fmt::format("Unknown variable `{}`", accessName);
        case NameAccessType::Unknown:
            return std::nullopt;
        default:
            GLSLD_UNREACHABLE();
        }
    }

    class HoverVisitor : public AstVisitor<HoverVisitor>
    {
    public:
        HoverVisitor(GlsldCompiler& compiler, lsp::Position position)
            : compiler(compiler), line(position.line), column(position.character)
        {
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            // we already find the hover
            if (hover) {
                return AstVisitPolicy::Leave;
            }

            const auto& lexContext = compiler.GetLexContext();

            auto locBegin = lexContext.LookupSyntaxLocation(node.GetRange().begin);
            auto locEnd   = lexContext.LookupSyntaxLocation(node.GetRange().end);
            if (locBegin.line <= line && locEnd.line >= line) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

        // auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        // {
        //     const auto& lexContext = compiler.GetLexContext();

        //     auto locBegin = lexContext.LookupSyntaxLocation(decl.GetName().range.begin);
        //     auto locEnd   = lexContext.LookupSyntaxLocation(decl.GetName().range.end);
        //     if (locBegin.line != line || locEnd.line != line) {
        //         return;
        //     }
        //     if (locBegin.column <= column && locEnd.column >= column) {
        //         GLSLD_ASSERT(!hover.has_value());
        //         auto hoverContent = GetHoverContent(decl, decl.GetName().text.StrView());
        //         if (hoverContent) {
        //             hover = lsp::Hover{.contents = *hoverContent,
        //                                .range    = lsp::Range{
        //                                       .start = {.line      = static_cast<uint32_t>(locBegin.line),
        //                                                 .character = static_cast<uint32_t>(locBegin.column)},
        //                                       .end   = {.line      = static_cast<uint32_t>(locEnd.line),
        //                                                 .character = static_cast<uint32_t>(locEnd.column)},
        //                                }};
        //         }
        //     }
        // }

        auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            const auto& lexContext = compiler.GetLexContext();

            auto locBegin = lexContext.LookupSyntaxLocation(expr.GetAccessName().range.begin);
            auto locEnd   = lexContext.LookupSyntaxLocation(expr.GetAccessName().range.end);
            if (locBegin.line != line || locEnd.line != line) {
                return;
            }
            if (locBegin.column <= column && locEnd.column >= column) {
                GLSLD_ASSERT(!hover.has_value());
                auto hoverContent = GetHoverContent(expr);
                if (hoverContent) {
                    hover = lsp::Hover{.contents = *hoverContent,
                                       .range    = lsp::Range{
                                              .start = {.line      = static_cast<uint32_t>(locBegin.line),
                                                        .character = static_cast<uint32_t>(locBegin.column)},
                                              .end   = {.line      = static_cast<uint32_t>(locEnd.line),
                                                        .character = static_cast<uint32_t>(locEnd.column)},
                                       }};
                }
            }
        }

        auto GetHover() -> std::optional<lsp::Hover>&
        {
            return hover;
        }

    private:
        std::optional<lsp::Hover> hover;

        GlsldCompiler& compiler;
        int line;
        int column;
    }; // namespace glsld

    auto ComputeHover(GlsldCompiler& compiler, lsp::Position position) -> std::optional<lsp::Hover>
    {
        HoverVisitor visitor{compiler, position};
        visitor.TraverseAst(compiler.GetAstContext());

        return std::move(visitor.GetHover());
    }

} // namespace glsld
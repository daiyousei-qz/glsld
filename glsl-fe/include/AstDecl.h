#pragma once
#include "SyntaxToken.h"
#include "AstBase.h"
#include "AstExpr.h"
#include "AstStmt.h"

#include <optional>

namespace glsld
{
    class MSVC_EMPTY_BASES AstDecl : public AstNodeBase, public AstPayload<AstDecl>
    {
    };

    class MSVC_EMPTY_BASES AstEmptyDecl : public AstDecl, public AstPayload<AstEmptyDecl>
    {
    public:
        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }
    };

    struct VariableDeclarator
    {
        // Symbol identifier
        SyntaxToken declTok;

        // Array specifier
        AstArraySpec* arraySize;

        // Initializer
        AstExpr* init;
    };

    class MSVC_EMPTY_BASES AstStructMemberDecl : public AstDecl, public AstPayload<AstStructMemberDecl>
    {
    public:
        AstStructMemberDecl(AstQualType* type) : type(type)
        {
            // Yes, a variable decl could declare no variable
        }
        AstStructMemberDecl(AstQualType* type, std::vector<VariableDeclarator> decls) : type(type), decls(decls)
        {
        }

        auto GetType() -> AstQualType*
        {
            return type;
        }
        auto GetDeclarators() -> ArrayView<VariableDeclarator>
        {
            return decls;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(type);
            for (const auto& declarator : decls) {
                visitor.Traverse(declarator.arraySize);
                visitor.Traverse(declarator.init);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            // FIXME: declarator?
            return "";
        }

    private:
        AstQualType* type;
        std::vector<VariableDeclarator> decls;
    };

    class MSVC_EMPTY_BASES AstStructDecl : public AstDecl, public AstPayload<AstStructDecl>
    {
    public:
        AstStructDecl(std::optional<SyntaxToken> declTok, std::vector<AstStructMemberDecl*> members)
            : declTok(declTok), members(std::move(members))
        {
        }

        auto GetDeclToken() -> const std::optional<SyntaxToken>&
        {
            return declTok;
        }
        auto GetMembers() -> ArrayView<AstStructMemberDecl* const>
        {
            return members;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            for (auto member : members) {
                visitor.Traverse(member);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            if (declTok && declTok->IsIdentifier()) {
                return fmt::format("DeclId: {}", declTok->text.StrView());
            }
            else {
                return "";
            }
        }

    private:
        std::optional<SyntaxToken> declTok        = std::nullopt;
        std::vector<AstStructMemberDecl*> members = {};
    };

    // declares variables
    class MSVC_EMPTY_BASES AstVariableDecl : public AstDecl, public AstPayload<AstVariableDecl>
    {
    public:
        AstVariableDecl(AstQualType* type) : type(type)
        {
            // Yes, a variable decl could declare no variable
        }
        AstVariableDecl(AstQualType* type, std::vector<VariableDeclarator> decls) : type(type), decls(decls)
        {
        }

        auto GetType() -> AstQualType*
        {
            return type;
        }
        auto GetDeclarators() -> ArrayView<VariableDeclarator>
        {
            return decls;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(type);
            for (const auto& declarator : decls) {
                visitor.Traverse(declarator.arraySize);
                visitor.Traverse(declarator.init);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            // FIXME: declarator?
            return "";
        }

    private:
        AstQualType* type;
        std::vector<VariableDeclarator> decls;
    };

    // declares a parameter
    class MSVC_EMPTY_BASES AstParamDecl : public AstDecl, public AstPayload<AstParamDecl>
    {
    public:
        AstParamDecl(AstQualType* type, std::optional<SyntaxToken> declTok) : type(type), declTok(declTok)
        {
        }

        auto GetType() -> AstQualType*
        {
            return type;
        }
        auto GetDeclToken() -> const std::optional<SyntaxToken>&
        {
            return declTok;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(type);
        }

        auto DumpNodeData() const -> std::string
        {
            if (declTok && declTok->IsIdentifier()) {
                return fmt::format("DeclId: {}", declTok->text.StrView());
            }
            else {
                return "";
            }
        }

    private:
        AstQualType* type;
        std::optional<SyntaxToken> declTok;
    };

    // declares a function
    class MSVC_EMPTY_BASES AstFunctionDecl : public AstDecl, public AstPayload<AstFunctionDecl>
    {
    public:
        AstFunctionDecl(AstQualType* returnType, SyntaxToken declTok, std::vector<AstParamDecl*> params)
            : returnType(returnType), declTok(declTok), params(std::move(params)), body(nullptr)
        {
        }

        AstFunctionDecl(AstQualType* returnType, SyntaxToken declTok, std::vector<AstParamDecl*> params, AstStmt* body)
            : returnType(returnType), declTok(declTok), params(std::move(params)), body(body)
        {
        }

        auto GetReturnType() -> AstQualType*
        {
            return returnType;
        }

        auto GetParams() -> ArrayView<AstParamDecl*>
        {
            return params;
        }

        auto GetName() -> const SyntaxToken&
        {
            return declTok;
        }

        auto GetBody() -> AstStmt*
        {
            return body;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(returnType);
            for (auto param : params) {
                visitor.Traverse(param);
            }
            visitor.Traverse(body);
        }

        auto DumpNodeData() const -> std::string
        {
            return fmt::format("DeclId: {}", declTok.text.StrView());
        }

    private:
        AstQualType* returnType;
        SyntaxToken declTok;
        std::vector<AstParamDecl*> params;

        // function body, nullptr if it's a declaration
        AstStmt* body;
    };

    class MSVC_EMPTY_BASES AstInterfaceBlockDecl : public AstDecl, public AstPayload<AstInterfaceBlockDecl>
    {
    public:
        AstInterfaceBlockDecl(SyntaxToken declTok, std::vector<AstStructMemberDecl*> members)
            : declTok(declTok), members(std::move(members))
        {
        }
        AstInterfaceBlockDecl(SyntaxToken declTok, std::vector<AstStructMemberDecl*> members,
                              VariableDeclarator declarator)
            : declTok(declTok), members(std::move(members)), declarator(declarator)
        {
        }

        auto GetDeclToken() -> const SyntaxToken&
        {
            return declTok;
        }
        auto GetMembers() -> ArrayView<AstStructMemberDecl* const>
        {
            return members;
        }
        auto GetDeclarator() -> const std::optional<VariableDeclarator>&
        {
            return declarator;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            for (auto member : members) {
                visitor.Traverse(member);
            }
            if (declarator) {
                visitor.Traverse(declarator->arraySize);
                visitor.Traverse(declarator->init);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            return fmt::format("DeclId: {}", declTok.text.StrView());
        }

    private:
        SyntaxToken declTok                          = {};
        std::vector<AstStructMemberDecl*> members    = {};
        std::optional<VariableDeclarator> declarator = std::nullopt;
    };
} // namespace glsld
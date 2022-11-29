#pragma once
#include "SyntaxToken.h"
#include "AstBase.h"
#include "AstExpr.h"
#include "AstStmt.h"

namespace glsld
{
    class AstDecl : public AstNodeBase
    {
    };

    struct AstEmptyDecl : public AstDecl
    {
        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
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

    struct AstStructMemberDecl : public AstDecl
    {
    public:
        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(type);
            visitor.Traverse(declarator.arraySize);
            visitor.Traverse(declarator.init);
        }

    private:
        AstQualType* type;
        VariableDeclarator declarator;
    };

    class AstStructDecl : public AstDecl
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
        auto GetMembers() -> std::span<AstStructMemberDecl* const>
        {
            return members;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            for (auto member : members) {
                visitor.Traverse(member);
            }
        }

    private:
        std::optional<SyntaxToken> declTok        = std::nullopt;
        std::vector<AstStructMemberDecl*> members = {};
    };

    // declares variables
    class AstVariableDecl : public AstDecl
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
        auto GetDeclarators() -> std::span<VariableDeclarator>
        {
            return decls;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(type);
            for (const auto& declarator : decls) {
                visitor.Traverse(declarator.arraySize);
                visitor.Traverse(declarator.init);
            }
        }

    private:
        AstQualType* type;
        std::vector<VariableDeclarator> decls;
    };

    // declares a parameter
    class AstParamDecl : public AstDecl
    {
    public:
        AstParamDecl(AstQualType* type, SyntaxToken declTok) : type(type), declTok(declTok)
        {
        }

        auto GetName() -> const SyntaxToken&
        {
            return declTok;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(type);
        }

    private:
        AstQualType* type;
        SyntaxToken declTok;
    };

    // declares a function
    class AstFunctionDecl : public AstDecl
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

        auto GetParams() -> std::span<AstParamDecl*>
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

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(returnType);
            for (auto param : params) {
                visitor.Traverse(param);
            }
            visitor.Traverse(body);
        }

    private:
        AstQualType* returnType;
        SyntaxToken declTok;
        std::vector<AstParamDecl*> params;

        // function body, nullptr if it's a declaration
        AstStmt* body;
    };

    class AstInterfaceBlockDecl : public AstDecl
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

        template <typename Visitor>
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

    private:
        SyntaxToken declTok                          = {};
        std::vector<AstStructMemberDecl*> members    = {};
        std::optional<VariableDeclarator> declarator = std::nullopt;
    };
} // namespace glsld
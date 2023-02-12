#pragma once
#include "SyntaxToken.h"
#include "AstImpl.h"
#include "AstExpr.h"
#include "AstStmt.h"

#include <optional>

namespace glsld
{
    class AstDecl : public AstImpl<AstDecl>
    {
    };

    class AstEmptyDecl : public AstImpl<AstEmptyDecl>
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
        SyntaxToken declTok = {};

        // Array specifier
        AstArraySpec* arraySize = nullptr;

        // Initializer (must be AstExpr or AstInitializerList)
        AstNodeBase* init = nullptr;
    };

    class AstStructMemberDecl : public AstImpl<AstStructMemberDecl>
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

    class AstStructDecl : public AstImpl<AstStructDecl>
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
    class AstVariableDecl : public AstImpl<AstVariableDecl>
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
    class AstParamDecl : public AstImpl<AstParamDecl>
    {
    public:
        AstParamDecl(AstQualType* type, std::optional<VariableDeclarator> declarator)
            : type(type), declarator(declarator)
        {
        }

        auto GetType() -> AstQualType*
        {
            return type;
        }
        auto GetDeclarator() -> const std::optional<VariableDeclarator>&
        {
            return declarator;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(type);
            if (declarator) {
                visitor.Traverse(declarator->arraySize);

                // FIXME: do we want to traverse this? GLSL doesn't allow it.
                visitor.Traverse(declarator->init);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            if (declarator && declarator->declTok.IsIdentifier()) {
                return fmt::format("DeclId: {}", declarator->declTok.text.StrView());
            }
            else {
                return "";
            }
        }

    private:
        AstQualType* type;
        std::optional<VariableDeclarator> declarator;
    };

    // declares a function
    class AstFunctionDecl : public AstImpl<AstFunctionDecl>
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

    class AstInterfaceBlockDecl : public AstImpl<AstInterfaceBlockDecl>
    {
    public:
        AstInterfaceBlockDecl(AstTypeQualifierSeq* quals, SyntaxToken declTok,
                              std::vector<AstStructMemberDecl*> members, std::optional<VariableDeclarator> declarator)
            : quals(quals), declTok(declTok), members(std::move(members)), declarator(declarator)
        {
        }

        auto GetQualifiers() -> const AstTypeQualifierSeq*
        {
            return quals;
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
            visitor.Traverse(quals);
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
        AstTypeQualifierSeq* quals                   = nullptr;
        SyntaxToken declTok                          = {};
        std::vector<AstStructMemberDecl*> members    = {};
        std::optional<VariableDeclarator> declarator = std::nullopt;
    };
} // namespace glsld
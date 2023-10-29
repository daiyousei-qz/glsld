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
        auto DoTraverse(Visitor& visitor) const -> bool
        {
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }
    };

    struct Declarator
    {
        // Symbol identifier
        SyntaxToken declTok = {};

        // Array specifier
        AstArraySpec* arraySize = nullptr;

        // Initializer
        AstInitializer* init = nullptr;
    };

    class AstStructMemberDecl : public AstImpl<AstStructMemberDecl>
    {
    public:
        AstStructMemberDecl(AstQualType* type) : type(type)
        {
            // Yes, a variable decl could declare no variable
        }
        AstStructMemberDecl(AstQualType* type, std::vector<Declarator> decls) : type(type), declarators(decls)
        {
        }

        auto GetType() -> AstQualType*
        {
            return type;
        }
        auto GetDeclarators() -> ArrayView<Declarator>
        {
            return declarators;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(type);
            for (const auto& declarator : declarators) {
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
        std::vector<Declarator> declarators;
    };

    class AstStructDecl : public AstImpl<AstStructDecl>
    {
    public:
        AstStructDecl(std::optional<SyntaxToken> declTok, std::vector<AstStructMemberDecl*> members)
            : declTok(declTok), members(std::move(members))
        {
        }

        auto GetDeclTok() -> const std::optional<SyntaxToken>&
        {
            return declTok;
        }
        auto GetMembers() -> ArrayView<AstStructMemberDecl*>
        {
            return members;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
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
        AstVariableDecl(AstQualType* type) : qualType(type)
        {
            // Yes, a variable decl could declare no variable
        }
        AstVariableDecl(AstQualType* type, std::vector<Declarator> decls) : qualType(type), decls(decls)
        {
        }

        auto GetType() -> AstQualType*
        {
            return qualType;
        }
        auto GetDeclarators() -> ArrayView<Declarator>
        {
            return decls;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(qualType);
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
        AstQualType* qualType;
        std::vector<Declarator> decls;
    };

    // declares a parameter
    class AstParamDecl : public AstImpl<AstParamDecl>
    {
    public:
        AstParamDecl(AstQualType* type, std::optional<Declarator> declarator) : qualType(type), declarator(declarator)
        {
        }

        auto GetType() -> AstQualType*
        {
            return qualType;
        }
        auto GetDeclarator() -> const std::optional<Declarator>&
        {
            return declarator;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(qualType);
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
        AstQualType* qualType;
        std::optional<Declarator> declarator;
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
        auto DoTraverse(Visitor& visitor) const -> bool
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
                              std::vector<AstStructMemberDecl*> members, std::optional<Declarator> declarator)
            : quals(quals), declTok(declTok), members(std::move(members)), instanceDeclarator(declarator)
        {
        }

        auto GetQualifiers() -> const AstTypeQualifierSeq*
        {
            return quals;
        }
        auto GetDeclTok() -> const SyntaxToken&
        {
            return declTok;
        }
        auto GetMembers() -> ArrayView<AstStructMemberDecl*>
        {
            return members;
        }
        auto GetDeclarator() -> const std::optional<Declarator>&
        {
            return instanceDeclarator;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(quals);
            for (auto member : members) {
                visitor.Traverse(member);
            }
            if (instanceDeclarator) {
                visitor.Traverse(instanceDeclarator->arraySize);
                visitor.Traverse(instanceDeclarator->init);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            if (instanceDeclarator) {
                return fmt::format("BlockTypeName: {}; BlockName: {}", declTok.text.StrView(),
                                   instanceDeclarator->declTok.text.StrView());
            }
            else {
                return fmt::format("BlockTypeName: {}", declTok.text.StrView());
            }
        }

    private:
        AstTypeQualifierSeq* quals                = nullptr;
        SyntaxToken declTok                       = {};
        std::vector<AstStructMemberDecl*> members = {};
        std::optional<Declarator> declarator      = std::nullopt;
    };
} // namespace glsld
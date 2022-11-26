#pragma once
#include "SyntaxToken.h"
#include "AstExpr.h"
#include "AstMisc.h"
#include "AstStmt.h"

namespace glsld
{
    class AstDecl;
    class AstStructDecl;

    class AstLayoutQualifier : public SyntaxNode
    {
    public:
    };
    class AstTypeQualifierSeq : public SyntaxNode
    {
    public:
        auto CanDeclInterfaceBlock() -> bool
        {
            return qIn || qOut || qUniform || qBuffer;
        }

        auto HasStorageQual() -> bool
        {
            return qConst || qIn || qOut || qInout || qAttribute || qUniform || qVarying || qBuffer || qShared;
        }

        auto GetHighp() -> bool
        {
            return qHighp;
        }
        auto SetHighp() -> void
        {
            qHighp = true;
        }
        auto GetMediump() -> bool
        {
            return qMediump;
        }
        auto SetMediump() -> void
        {
            qMediump = true;
        }
        auto GetLowp() -> bool
        {
            return qLowp;
        }
        auto SetLowp() -> void
        {
            qLowp = true;
        }

        auto SetConst() -> void
        {
            qConst = true;
        }
        auto GetConst() -> bool
        {
            return qConst;
        }
        auto SetIn() -> void
        {
            qIn = true;
        }
        auto GetIn() -> bool
        {
            return qIn;
        }
        auto SetOut() -> void
        {
            qOut = true;
        }
        auto GetOut() -> bool
        {
            return qOut;
        }
        auto SetInout() -> void
        {
            qInout = true;
        }
        auto GetInout() -> bool
        {
            return qInout;
        }
        auto SetAttribute() -> void
        {
            qAttribute = true;
        }
        auto GetAttribute() -> bool
        {
            return qAttribute;
        }
        auto SetUniform() -> void
        {
            qUniform = true;
        }
        auto GetUniform() -> bool
        {
            return qUniform;
        }
        auto SetVarying() -> void
        {
            qVarying = true;
        }
        auto GetVarying() -> bool
        {
            return qVarying;
        }
        auto SetBuffer() -> void
        {
            qBuffer = true;
        }
        auto GetBuffer() -> bool
        {
            return qBuffer;
        }
        auto SetShared() -> void
        {
            qShared = true;
        }
        auto GetShared() -> bool
        {
            return qShared;
        }

        auto SetCentroid() -> void
        {
            qCentroid = true;
        }
        auto GetCentroid() -> bool
        {
            return qCentroid;
        };
        auto SetSample() -> void
        {
            qSample = true;
        }
        auto GetSample() -> bool
        {
            return qSample;
        };
        auto SetPatch() -> void
        {
            qPatch = true;
        }
        auto GetPatch() -> bool
        {
            return qPatch;
        };

    protected:
        bool qHighp : 1   = false;
        bool qMediump : 1 = false;
        bool qLowp : 1    = false;

        // Storage qualifiers
        bool qConst : 1     = false;
        bool qIn : 1        = false;
        bool qOut : 1       = false;
        bool qInout : 1     = false;
        bool qAttribute : 1 = false;
        bool qUniform : 1   = false;
        bool qVarying : 1   = false;
        bool qBuffer : 1    = false;
        bool qShared : 1    = false;

        // Auxiliary storage qualifiers
        bool qCentroid : 1 = false;
        bool qSample : 1   = false;
        bool qPatch : 1    = false;
    };

    class AstQualType : public SyntaxNode
    {
    public:
        AstQualType(AstTypeQualifierSeq* qualifiers, SyntaxToken typeName) : qualifiers(qualifiers), typeName(typeName)
        {
        }
        AstQualType(AstTypeQualifierSeq* qualifiers, AstStructDecl* structDecl)
            : qualifiers(qualifiers), structDecl(structDecl)
        {
        }

    private:
        AstTypeQualifierSeq* qualifiers = nullptr;
        AstArraySpec* arraySpec         = nullptr;

        SyntaxToken typeName      = {};
        AstStructDecl* structDecl = nullptr;
    };

    class AstDecl : public SyntaxNode
    {
    };

    struct AstEmptyDecl : public AstDecl
    {
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

    private:
        AstQualType* returnType;
        SyntaxToken declTok;
        std::vector<AstParamDecl*> params;
        AstStmt* body;
    };

    class AstInterfaceBlockDecl : public AstDecl
    {
    private:
        AstInterfaceBlockDecl(SyntaxToken declTok, std::vector<AstStructMemberDecl*> members)
            : declTok(declTok), members(std::move(members))
        {
        }
        AstInterfaceBlockDecl(SyntaxToken declTok, std::vector<AstStructMemberDecl*> members,
                              VariableDeclarator declarator)
            : declTok(declTok), members(std::move(members)), declarator(declarator)
        {
        }

    private:
        SyntaxToken declTok                          = {};
        std::vector<AstStructMemberDecl*> members    = {};
        std::optional<VariableDeclarator> declarator = std::nullopt;
    };
} // namespace glsld
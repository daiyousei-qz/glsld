#pragma once
#include "Common.h"

#include <array>
#include <iterator>
#include <vector>
#include <span>
#include <string>
#include <string_view>

namespace glsld
{
    struct SyntaxLocation
    {
    };

    struct SyntaxRange
    {
        int begin;
        int end;
    };

    enum class TokenKlass
    {
        Error,
        Eof,
        IntegerConstant,
        FloatConstant,
        Identifier,

#define DECL_KEYWORD(KEYWORD) K_##KEYWORD,
#include "GlslKeywords.inc"
#undef DECL_KEYWORD

#define DECL_PUNCT(PUNCT_NAME, ...) PUNCT_NAME,
#include "GlslPunctuation.inc"
#undef DECL_PUNCT
    };

#define GLSLD_GENERATE_EXPR_OP(GLSLD_PICK)

    //
    enum class ExprOp
    {
#define DECL_EXPROP(OPNAME, ...) OPNAME,
#include "GlslExprOp.inc"
#undef DECL_EXPROP
    };

    inline constexpr auto ExprOpToString(ExprOp op) noexcept -> std::string_view
    {
        switch (op) {
#define DECL_EXPROP(OPNAME, ...)                                                                                       \
    case ExprOp::OPNAME:                                                                                               \
        return #OPNAME;
#include "GlslExprOp.inc"
#undef DECL_EXPROP
        default:
            GLSLD_UNREACHABLE();
        }
    }

    struct SyntaxToken
    {
        SyntaxRange range;
        TokenKlass klass;

        // string hosted in the atom table
        std::string_view text;
    };

    class SyntaxNode
    {
    public:
        virtual ~SyntaxNode()
        {
        }

        auto UpdateRange(SyntaxRange range) -> void
        {
            range_ = range;
        }

    private:
        SyntaxRange range_;
    };

    class AstExpr;
    class AstErrorExpr;
    class AstConstantExpr;
    class AstVarAccessExpr;
    class AstUnaryExpr;
    class AstBinaryExpr;
    class AstSelectExpr;
    class AstFuncionCallExpr;

    class AstCompoundStmt;
    class AstExprStmt;
    class AstIfStmt;
    class AstForStmt;
    class AstWhileStmt;
    class AstSwitchStmt;
    class AstContinueStmt;
    class AstBreakStmt;
    class AstDiscardStmt;
    class AstReturnStmt;

    class AstExpr : public SyntaxNode
    {
    public:
        AstExpr(ExprOp op) : op_(op)
        {
        }

        auto GetOp() -> ExprOp
        {
            return op_;
        }

    private:
        ExprOp op_;
    };

    using AstStmt = AstExpr;

    template <size_t N> class AstExprImpl : public AstExpr
    {
    public:
        using AstExpr::AstExpr;

        auto GetChildren() -> std::span<AstExpr*>
        {
            return children_;
        }

    protected:
        std::array<AstExpr*, N> children_ = {};
    };
    template <> class AstExprImpl<0> : public AstExpr
    {
    public:
        using AstExpr::AstExpr;

        auto GetChildren() -> std::span<AstExpr*>
        {
            return {};
        }
    };
    class AstCompoundExprImpl : public AstExpr
    {
    public:
        using AstExpr::AstExpr;

        auto GetChildren() -> std::span<AstExpr*>
        {
            return children;
        }

    protected:
        std::vector<AstExpr*> children;
    };

    class AstErrorExpr final : public AstExprImpl<0>
    {
    public:
        AstErrorExpr() : AstExprImpl(ExprOp::Error)
        {
        }

    private:
    };
    class AstConstantExpr final : public AstExprImpl<0>
    {
    public:
        AstConstantExpr(std::string value) : AstExprImpl(ExprOp::Const), value_(std::move(value))
        {
        }

    private:
        std::string value_;
    };
    class AstVarAccessExpr final : public AstExprImpl<0>
    {
    public:
        AstVarAccessExpr(std::string name) : AstExprImpl(ExprOp::VarAccess), varName(std::move(name))
        {
        }

    private:
        std::string varName;
    };

    class AstUnaryExpr final : public AstExprImpl<1>
    {
    public:
        AstUnaryExpr(ExprOp op, AstExpr* operand) : AstExprImpl(op)
        {
            // TODO: assert op is unary
            children_[0] = operand;
        }

        auto GetOperandExpr() -> AstExpr*
        {
            return children_[0];
        }

    private:
    };
    class AstBinaryExpr final : public AstExprImpl<2>
    {
    public:
        AstBinaryExpr(ExprOp op, AstExpr* lhs, AstExpr* rhs) : AstExprImpl(op)
        {
            // TODO: assert op is binary
            children_[0] = lhs;
            children_[1] = rhs;
        }

        auto GetLeftOperandExpr() -> AstExpr*
        {
            return children_[0];
        }
        auto GetRightOperandExpr() -> AstExpr*
        {
            return children_[1];
        }

    private:
    };
    class AstSelectExpr final : public AstExprImpl<3>
    {
    public:
        AstSelectExpr(AstExpr* predicate, AstExpr* positive, AstExpr* negative) : AstExprImpl(ExprOp::Select)
        {
            children_[0] = predicate;
            children_[1] = positive;
            children_[2] = negative;
        }

        auto GetConditionExpr() -> AstExpr*
        {
            return children_[0];
        }
        auto GetPositiveExpr() -> AstExpr*
        {
            return children_[1];
        }
        auto GetNegativeExpr() -> AstExpr*
        {
            return children_[2];
        }

    private:
    };
    class AstFuncionCallExpr final : public AstExprImpl<0>
    {
    public:
    private:
    };

    class AstCompoundStmt final : public AstCompoundExprImpl
    {
    public:
        AstCompoundStmt(std::vector<AstExpr*> children) : AstCompoundExprImpl(ExprOp::CompoundStmt)
        {
            this->children = std::move(children);
        }

    private:
    };
    class AstExprStmt final : public AstExprImpl<1>
    {
    public:
        AstExprStmt(AstExpr* expr) : AstExprImpl(ExprOp::ExprStmt)
        {
            children_[0] = expr;
        }
    };
    // class AstDeclarationStmt final : public AstStmt
    // {
    // public:
    // private:
    // };
    class AstForStmt final : public AstExprImpl<3>
    {
    public:
        AstForStmt() : AstExprImpl(ExprOp::ForStmt)
        {
        }

    private:
    };
    class AstWhileStmt final : public AstExprImpl<2>
    {
    public:
        AstWhileStmt(AstExpr* predicate, AstExpr* loop_body) : AstExprImpl(ExprOp::WhileStmt)
        {
            children_[0] = predicate;
            children_[1] = loop_body;
        }

        auto GetPredicateExpr() -> AstExpr*
        {
            return children_[0];
        }
        auto GetLoopedStmt() -> AstExpr*
        {
            return children_[1];
        }
    };
    class AstIfStmt final : public AstExprImpl<3>
    {
    public:
        AstIfStmt(AstExpr* predicate, AstExpr* positive) : AstExprImpl(ExprOp::IfStmt)
        {
            children_[0] = predicate;
            children_[1] = positive;
            children_[2] = nullptr;
        }
        AstIfStmt(AstExpr* predicate, AstExpr* positive, AstExpr* negative) : AstExprImpl(ExprOp::IfStmt)
        {
            children_[0] = predicate;
            children_[1] = positive;
            children_[2] = negative;
        }

        auto GetPredicateExpr() -> AstExpr*
        {
            return children_[0];
        }
        auto GetPositiveStmt() -> AstStmt*
        {
            return children_[1];
        }
        auto GetNegativeStmt() -> AstStmt*
        {
            return children_[2];
        }
    };
    class AstSwitchStmt final : public AstCompoundExprImpl
    {
    public:
        AstSwitchStmt() : AstCompoundExprImpl(ExprOp::SwitchStmt)
        {
        }

    private:
    };
    class AstContinueStmt final : public AstExprImpl<0>
    {
    public:
        AstContinueStmt() : AstExprImpl(ExprOp::ContinueStmt)
        {
        }

    private:
    };
    class AstBreakStmt final : public AstExprImpl<0>
    {
    public:
        AstBreakStmt() : AstExprImpl(ExprOp::BreakStmt)
        {
        }

    private:
    };
    class AstDiscardStmt final : public AstExprImpl<0>
    {
    public:
        AstDiscardStmt() : AstExprImpl(ExprOp::DiscardStmt)
        {
        }

    private:
    };
    class AstReturnStmt final : public AstExprImpl<1>
    {
    public:
        AstReturnStmt() : AstExprImpl(ExprOp::ReturnStmt)
        {
            children_[0] = nullptr;
        }
        AstReturnStmt(AstExpr* expr) : AstExprImpl(ExprOp::ReturnStmt)
        {
            GLSLD_ASSERT(expr != nullptr);
            children_[0] = expr;
        }

        auto GetReturnedValue() -> AstExpr*
        {
            return children_[0];
        }
    };

    // identifier
    class AstDeclId : public SyntaxNode
    {
    public:
        AstDeclId(std::string id) : id(std::move(id))
        {
        }

        auto GetId() -> const std::string&
        {
            return id;
        }

    private:
        std::string id;
    };

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
    // array sizes
    class AstArraySpec : public SyntaxNode
    {
    public:
        AstArraySpec()
        {
        }
        AstArraySpec(std::vector<AstExpr*> sizes) : sizes(std::move(sizes))
        {
        }

    private:
        std::vector<AstExpr*> sizes;
    };
    class AstQualType : public SyntaxNode
    {
    public:
        AstQualType(std::string_view builtinName, AstTypeQualifierSeq* qualifiers)
            : builtinName(builtinName), qualifiers(qualifiers)
        {
        }
        AstQualType(AstDeclId* name, AstTypeQualifierSeq* qualifiers) : typeName(name), qualifiers(qualifiers)
        {
        }

    private:
        AstTypeQualifierSeq* qualifiers = nullptr;
        AstArraySpec* arraySpec         = nullptr;

        std::string_view builtinName = {};
        AstDeclId* typeName          = nullptr;
    };

    class AstDecl : public SyntaxNode
    {
    };

    struct VariableDeclarator
    {
        AstDeclId* id;
        AstArraySpec* arraySize;
        AstExpr* init;
    };

    struct AstStructMemberDecl : public AstDecl
    {
    private:
        AstQualType* type;
        VariableDeclarator declarator;
    };

    class AstStructDecl : public AstDecl
    {
    private:
        AstDeclId* name;
        std::vector<AstStructMemberDecl*> members;
    };

    // declares variables
    class AstVariableDecl : public AstDecl
    {
    public:
        AstVariableDecl(AstQualType* type, std::vector<VariableDeclarator> decls) : type(type), decls(decls)
        {
        }

    private:
        AstQualType* type;
        std::vector<VariableDeclarator> decls;
    };

    // declares a parameter
    class AstParamDecl : public AstDecl
    {
    public:
        AstParamDecl(AstQualType* type, AstDeclId* id) : type(type), id(id)
        {
        }

    private:
        AstQualType* type;
        AstDeclId* id;
    };

    // declares a function
    class AstFunctionDecl : public AstDecl
    {
    public:
        AstFunctionDecl(AstQualType* returnType, AstDeclId* id, std::vector<AstParamDecl*> params)
            : returnType(returnType), id(id), params(std::move(params)), body(nullptr)
        {
        }

        AstFunctionDecl(AstQualType* returnType, AstDeclId* id, std::vector<AstParamDecl*> params,
                        AstCompoundStmt* body)
            : returnType(returnType), id(id), params(std::move(params)), body(body)
        {
        }

        auto GetFunctionBody() -> AstCompoundStmt*
        {
            return body;
        }

    private:
        AstQualType* returnType;
        AstDeclId* id;
        std::vector<AstParamDecl*> params;
        AstCompoundStmt* body;
    };

    class AstGlobalVariableDecl : public AstDecl
    {
    };

    class AstGlobalBlockDecl : public AstDecl
    {
    };

    class AstTranslationUnit
    {
    public:
    };
} // namespace glsld
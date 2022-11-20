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
    struct SyntaxLocationInfo
    {
        int file;
        int offset;
        int line;
        int column;
    };

    struct SyntaxLocation
    {
    public:
        SyntaxLocation()
        {
        }
        SyntaxLocation(int index) : index(index)
        {
        }

        int GetIndex()
        {
            return index;
        }

    private:
        int index = -1;
    };

    struct SyntaxRange
    {
        SyntaxLocation begin;
        SyntaxLocation end;
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
        TokenKlass klass;

        // string hosted in the atom table
        std::string_view text;
        SyntaxRange range;
    };

    class SyntaxNode
    {
    public:
        virtual ~SyntaxNode()
        {
        }

        auto GetRange() -> SyntaxRange
        {
            return range;
        }

        auto UpdateRange(SyntaxRange range) -> void
        {
            this->range = range;
        }

    private:
        SyntaxRange range;
    };

    class AstExpr;
    class AstErrorExpr;
    class AstConstantExpr;
    class AstVarAccessExpr;
    class AstUnaryExpr;
    class AstBinaryExpr;
    class AstSelectExpr;
    class AstInvokeExpr;

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

    // Identifier of a declared/used symbol
    class AstDeclId : public SyntaxNode
    {
    public:
        AstDeclId(std::string_view id) : id(id)
        {
        }

        auto GetIdentifier() -> std::string_view
        {
            return id;
        }

    private:
        // The actual memory is hosted in the LexContext
        std::string_view id;
    };

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

    template <size_t N>
    class AstExprImpl : public AstExpr
    {
    public:
        using AstExpr::AstExpr;

        auto GetChildren() -> std::span<AstExpr*>
        {
            return children;
        }

    protected:
        std::array<AstExpr*, N> children = {};
    };
    template <>
    class AstExprImpl<0> : public AstExpr
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
        AstConstantExpr(std::string_view value) : AstExprImpl(ExprOp::Const), value(value)
        {
        }

    private:
        // The actual memory is hosted in the LexContext
        std::string_view value;
    };
    class AstVarAccessExpr final : public AstExprImpl<1>
    {
    public:
        AstVarAccessExpr(AstExpr* accessChain, std::string_view accessName)
            : AstExprImpl(ExprOp::VarAccess), accessName(accessName)
        {
        }
        AstVarAccessExpr(std::string_view accessName) : AstExprImpl(ExprOp::VarAccess), accessName(accessName)
        {
        }

        auto GetAccessChain() -> AstExpr*
        {
            return children[0];
        }
        auto GetAccessName() -> std::string_view
        {
            return accessName;
        }

    private:
        std::string_view accessName;
    };

    class AstUnaryExpr final : public AstExprImpl<1>
    {
    public:
        AstUnaryExpr(ExprOp op, AstExpr* operand) : AstExprImpl(op)
        {
            // TODO: assert op is unary
            children[0] = operand;
        }

        auto GetOperandExpr() -> AstExpr*
        {
            return children[0];
        }

    private:
    };
    class AstBinaryExpr final : public AstExprImpl<2>
    {
    public:
        AstBinaryExpr(ExprOp op, AstExpr* lhs, AstExpr* rhs) : AstExprImpl(op)
        {
            // TODO: assert op is binary
            children[0] = lhs;
            children[1] = rhs;
        }

        auto GetLeftOperandExpr() -> AstExpr*
        {
            return children[0];
        }
        auto GetRightOperandExpr() -> AstExpr*
        {
            return children[1];
        }

    private:
    };
    class AstSelectExpr final : public AstExprImpl<3>
    {
    public:
        AstSelectExpr(AstExpr* predicate, AstExpr* positive, AstExpr* negative) : AstExprImpl(ExprOp::Select)
        {
            children[0] = predicate;
            children[1] = positive;
            children[2] = negative;
        }

        auto GetConditionExpr() -> AstExpr*
        {
            return children[0];
        }
        auto GetPositiveExpr() -> AstExpr*
        {
            return children[1];
        }
        auto GetNegativeExpr() -> AstExpr*
        {
            return children[2];
        }

    private:
    };

    enum class InvocationType
    {
        FunctionCall,
        Indexing,
    };
    class AstInvokeExpr final : public AstCompoundExprImpl
    {
    public:
        AstInvokeExpr(InvocationType type, AstExpr* invokedExpr, std::vector<AstExpr*> args)
            : AstCompoundExprImpl(type == InvocationType::FunctionCall ? ExprOp::FunctionCall : ExprOp::IndexAccess)
        {
            children.push_back(invokedExpr);
            std::ranges::copy(args, std::back_inserter(children));
        }

        auto GetInvokedExpr() -> AstExpr*
        {
            return children[0];
        }

        auto GetArguments() -> std::span<AstExpr*>
        {
            return std::span<AstExpr*>(children).subspan(1);
        }

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
            children[0] = expr;
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
            children[0] = predicate;
            children[1] = loop_body;
        }

        auto GetPredicateExpr() -> AstExpr*
        {
            return children[0];
        }
        auto GetLoopedStmt() -> AstExpr*
        {
            return children[1];
        }
    };
    class AstIfStmt final : public AstExprImpl<3>
    {
    public:
        AstIfStmt(AstExpr* predicate, AstExpr* positive) : AstExprImpl(ExprOp::IfStmt)
        {
            children[0] = predicate;
            children[1] = positive;
            children[2] = nullptr;
        }
        AstIfStmt(AstExpr* predicate, AstExpr* positive, AstExpr* negative) : AstExprImpl(ExprOp::IfStmt)
        {
            children[0] = predicate;
            children[1] = positive;
            children[2] = negative;
        }

        auto GetPredicateExpr() -> AstExpr*
        {
            return children[0];
        }
        auto GetPositiveStmt() -> AstStmt*
        {
            return children[1];
        }
        auto GetNegativeStmt() -> AstStmt*
        {
            return children[2];
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
            children[0] = nullptr;
        }
        AstReturnStmt(AstExpr* expr) : AstExprImpl(ExprOp::ReturnStmt)
        {
            GLSLD_ASSERT(expr != nullptr);
            children[0] = expr;
        }

        auto GetReturnedValue() -> AstExpr*
        {
            return children[0];
        }
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
        // Symbol identifier
        AstDeclId* id;

        // Array specifier
        AstArraySpec* arraySize;

        // Initializer
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

        auto GetName() -> AstDeclId*
        {
            return id;
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

        auto GetReturnType() -> AstQualType*
        {
            return returnType;
        }

        auto GetParams() -> std::span<AstParamDecl*>
        {
            return params;
        }

        auto GetName() -> AstDeclId*
        {
            return id;
        }

        auto GetBody() -> AstCompoundStmt*
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
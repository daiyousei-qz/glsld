#pragma once
#include "SyntaxToken.h"
#include "AstExpr.h"
#include "Semantic.h"

namespace glsld
{
    class AstDecl;
    using AstStmt = AstExpr;

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
    // FIXME: how to handle children?
    class AstDeclStmt final : public AstExprImpl<0>
    {
    public:
        // FIXME: give correct ExprOp
        AstDeclStmt() : AstExprImpl(ExprOp::Error)
        {
        }

    private:
        AstDecl* decl;
    };
    class AstForStmt final : public AstExprImpl<4>
    {
    public:
        AstForStmt(AstStmt* initClause, AstStmt* testClause, AstStmt* proceedClause, AstStmt* loopBody)
            : AstExprImpl(ExprOp::ForStmt)
        {
            children[0] = initClause;
            children[1] = testClause;
            children[2] = proceedClause;
            children[3] = loopBody;
        }

        auto GetInitClause() -> AstStmt*
        {
            return children[0];
        }
        auto GetTestClause() -> AstStmt*
        {
            return children[1];
        }
        auto GetProceedClause() -> AstStmt*
        {
            return children[2];
        }
        auto GetLoopBody() -> AstStmt*
        {
            return children[3];
        }

    private:
    };
    class AstWhileStmt final : public AstExprImpl<2>
    {
    public:
        AstWhileStmt(AstExpr* predicate, AstStmt* loopBody) : AstExprImpl(ExprOp::WhileStmt)
        {
            children[0] = predicate;
            children[1] = loopBody;
        }

        auto GetPredicateExpr() -> AstExpr*
        {
            return children[0];
        }
        auto GetLoopBody() -> AstExpr*
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
    class AstLabeledStmt final : public AstExprImpl<1>
    {
    public:
        // FIXME: give correct ExprOp
        AstLabeledStmt(AstStmt* stmt) : AstExprImpl(ExprOp::Error)
        {
            children[0] = stmt;
        }

    private:
        // FIXME: case label/default label
    };
    class AstSwitchStmt final : public AstCompoundExprImpl
    {
    public:
        AstSwitchStmt(std::vector<AstStmt*> body) : AstCompoundExprImpl(ExprOp::SwitchStmt)
        {
            children = std::move(body);
        }

    private:
    };
    class AstJumpStmt final : public AstExprImpl<0>
    {
    public:
        AstJumpStmt(JumpType type) : AstExprImpl(GetJumpOp(type))
        {
        }

    private:
        static constexpr auto GetJumpOp(JumpType type) -> ExprOp
        {
            switch (type) {
            case JumpType::Break:
                return ExprOp::BreakStmt;
            case JumpType::Continue:
                return ExprOp::ContinueStmt;
            case JumpType::Discard:
                return ExprOp::DiscardStmt;
            default:
                GLSLD_UNREACHABLE();
            }
        }
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
} // namespace glsld
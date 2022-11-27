#pragma once
#include "AstBase.h"
#include "AstMisc.h"
#include "Semantic.h"
#include <span>
#include <array>

namespace glsld
{
    class AstStmt : public AstNodeBase
    {
    };

    class AstErrorStmt final : public AstStmt
    {
    public:
        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }
    };

    class AstCompoundStmt final : public AstStmt
    {
    public:
        AstCompoundStmt(std::vector<AstStmt*> children) : children(std::move(children))
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            for (auto stmt : children) {
                visitor.Traverse(*stmt);
            }
        }

    private:
        std::vector<AstStmt*> children;
    };
    class AstExprStmt final : public AstStmt
    {
    public:
        AstExprStmt(AstExpr* expr) : expr(expr)
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*expr);
        }

    private:
        AstExpr* expr;
    };

    class AstDeclStmt final : public AstStmt
    {
    public:
        AstDeclStmt(AstDecl* decl) : decl(decl)
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*decl);
        }

    private:
        AstDecl* decl;
    };
    class AstForStmt final : public AstStmt
    {
    public:
        AstForStmt(AstStmt* initClause, AstStmt* testClause, AstStmt* proceedClause, AstStmt* loopBody)
            : initClause(initClause), testClause(testClause), proceedClause(proceedClause), loopBody(loopBody)
        {
        }

        auto GetInitClause() -> AstStmt*
        {
            return initClause;
        }
        auto GetTestClause() -> AstStmt*
        {
            return testClause;
        }
        auto GetProceedClause() -> AstStmt*
        {
            return proceedClause;
        }
        auto GetLoopBody() -> AstStmt*
        {
            return loopBody;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*initClause);
            visitor.Traverse(*testClause);
            visitor.Traverse(*proceedClause);
            visitor.Traverse(*loopBody);
        }

    private:
        AstStmt* initClause;
        AstStmt* testClause;
        AstStmt* proceedClause;
        AstStmt* loopBody;
    };
    class AstWhileStmt final : public AstStmt
    {
    public:
        AstWhileStmt(AstExpr* predicate, AstStmt* loopBody) : predicate(predicate), loopBody(loopBody)
        {
        }

        auto GetPredicateExpr() -> AstExpr*
        {
            return predicate;
        }
        auto GetLoopBody() -> AstStmt*
        {
            return loopBody;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*predicate);
            visitor.Traverse(*loopBody);
        }

    private:
        AstExpr* predicate;
        AstStmt* loopBody;
    };
    class AstIfStmt final : public AstStmt
    {
    public:
        AstIfStmt(AstExpr* predicate, AstStmt* ifBranch) : predicate(predicate), ifBranch(ifBranch), elseBranch(nullptr)
        {
        }
        AstIfStmt(AstExpr* predicate, AstStmt* ifBranch, AstStmt* elseBranch)
            : predicate(predicate), ifBranch(ifBranch), elseBranch(elseBranch)
        {
        }

        auto GetPredicateExpr() -> AstExpr*
        {
            return predicate;
        }
        auto GetIfBranch() -> AstStmt*
        {
            return ifBranch;
        }
        auto GetElseBranch() -> AstStmt*
        {
            return elseBranch;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*predicate);
            visitor.Traverse(*ifBranch);
            if (elseBranch) {
                visitor.Traverse(*elseBranch);
            }
        }

    private:
        AstExpr* predicate;
        AstStmt* ifBranch;
        AstStmt* elseBranch;
    };
    class AstLabeledStmt final : public AstStmt
    {
    public:
        // FIXME: give correct ExprOp
        AstLabeledStmt(AstStmt* innerStmt) : innerStmt(innerStmt)
        {
        }

        auto GetInnerStmt() -> AstStmt*
        {
            return innerStmt;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            if (innerStmt) {
                visitor.Traverse(*innerStmt);
            }
        }

    private:
        // FIXME: case label/default label
        AstStmt* innerStmt;
    };
    class AstSwitchStmt final : public AstStmt
    {
    public:
        AstSwitchStmt(std::vector<AstStmt*> children) : children(children)
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            for (auto stmt : children) {
                visitor.Traverse(*stmt);
            }
        }

    private:
        std::vector<AstStmt*> children;
    };
    class AstJumpStmt final : public AstStmt
    {
    public:
        AstJumpStmt(JumpType type) : type(type)
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

    private:
        JumpType type;
    };
    class AstReturnStmt final : public AstStmt
    {
    public:
        AstReturnStmt() : returnValue(nullptr)
        {
        }
        AstReturnStmt(AstExpr* returnValue) : returnValue(returnValue)
        {
            GLSLD_ASSERT(returnValue != nullptr);
        }

        auto GetReturnValue() -> AstExpr*
        {
            return returnValue;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*returnValue);
        }

    private:
        AstExpr* returnValue;
    };
} // namespace glsld
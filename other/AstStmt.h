#pragma once
#include "AstImpl.h"
#include "AstMisc.h"
#include "Semantic.h"
#include <array>

namespace glsld
{
    class AstStmt : public AstImpl<AstStmt>
    {
    };

    class AstErrorStmt final : public AstImpl<AstErrorStmt>
    {
    public:
        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }
    };

    class AstCompoundStmt final : public AstImpl<AstCompoundStmt>
    {
    public:
        AstCompoundStmt(std::vector<AstStmt*> children) : children(std::move(children))
        {
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            for (auto stmt : children) {
                visitor.Traverse(*stmt);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        std::vector<AstStmt*> children;
    };
    class AstExprStmt final : public AstImpl<AstExprStmt>
    {
    public:
        AstExprStmt(AstExpr* expr) : expr(expr)
        {
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(*expr);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* expr;
    };

    class AstDeclStmt final : public AstImpl<AstDeclStmt>
    {
    public:
        AstDeclStmt(AstDecl* decl) : decl(decl)
        {
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(*decl);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstDecl* decl;
    };
    class AstForStmt final : public AstImpl<AstForStmt>
    {
    public:
        AstForStmt(AstStmt* initClause, AstExpr* condExpr, AstExpr* iterationExpr, AstStmt* loopBody)
            : initClause(initClause), condExpr(condExpr), iterationExpr(iterationExpr), loopBody(loopBody)
        {
        }

        auto GetInitClause() -> AstStmt*
        {
            return initClause;
        }
        auto GetConditionExpr() -> AstExpr*
        {
            return condExpr;
        }
        auto GetIterationExpr() -> AstExpr*
        {
            return iterationExpr;
        }
        auto GetLoopBody() -> AstStmt*
        {
            return loopBody;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(*initClause);
            visitor.Traverse(condExpr);
            visitor.Traverse(iterationExpr);
            visitor.Traverse(*loopBody);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstStmt* initClause;
        AstExpr* condExpr;
        AstExpr* iterationExpr;
        AstStmt* loopBody;
    };
    class AstDoWhileStmt final : public AstImpl<AstDoWhileStmt>
    {
    public:
        AstDoWhileStmt(AstExpr* predicate, AstStmt* loopBody) : predicate(predicate), loopBody(loopBody)
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

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(*predicate);
            visitor.Traverse(*loopBody);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* predicate;
        AstStmt* loopBody;
    };
    class AstWhileStmt final : public AstImpl<AstWhileStmt>
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

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(*predicate);
            visitor.Traverse(*loopBody);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* predicate;
        AstStmt* loopBody;
    };
    class AstIfStmt final : public AstImpl<AstIfStmt>
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

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(*predicate);
            visitor.Traverse(*ifBranch);
            if (elseBranch) {
                visitor.Traverse(*elseBranch);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* predicate;
        AstStmt* ifBranch;
        AstStmt* elseBranch;
    };
    class AstLabelStmt final : public AstImpl<AstLabelStmt>
    {
    public:
        AstLabelStmt(AstExpr* caseExpr) : caseExpr(caseExpr)
        {
        }

        auto GetCaseExpr() -> AstExpr*
        {
            return caseExpr;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(caseExpr);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        // FIXME: case label/default label
        AstExpr* caseExpr;
    };
    class AstSwitchStmt final : public AstImpl<AstSwitchStmt>
    {
    public:
        AstSwitchStmt(AstExpr* expr, AstStmt* child) : expr(expr), body(child)
        {
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(*expr);
            visitor.Traverse(*body);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* expr;
        AstStmt* body;
    };
    class AstJumpStmt final : public AstImpl<AstJumpStmt>
    {
    public:
        AstJumpStmt(JumpType type) : type(type)
        {
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
        }

        auto DumpNodeData() const -> std::string
        {
            // FIXME: use different AST types
            return "";
        }

    private:
        JumpType type;
    };
    class AstReturnStmt final : public AstImpl<AstReturnStmt>
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

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            visitor.Traverse(returnValue);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* returnValue;
    };
} // namespace glsld
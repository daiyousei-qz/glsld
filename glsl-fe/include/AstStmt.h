#pragma once
#include "AstBase.h"
#include "AstMisc.h"
#include "Semantic.h"
#include <span>
#include <array>

namespace glsld
{
    class MSVC_EMPTY_BASES AstStmt : public AstNodeBase, public AstPayload<AstStmt>
    {
    };

    class MSVC_EMPTY_BASES AstErrorStmt final : public AstStmt, public AstPayload<AstErrorStmt>
    {
    public:
        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }
    };

    class MSVC_EMPTY_BASES AstCompoundStmt final : public AstStmt, public AstPayload<AstCompoundStmt>
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

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        std::vector<AstStmt*> children;
    };
    class MSVC_EMPTY_BASES AstExprStmt final : public AstStmt, public AstPayload<AstExprStmt>
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

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* expr;
    };

    class MSVC_EMPTY_BASES AstDeclStmt final : public AstStmt, public AstPayload<AstDeclStmt>
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

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstDecl* decl;
    };
    class MSVC_EMPTY_BASES AstForStmt final : public AstStmt, public AstPayload<AstForStmt>
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

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
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
    class MSVC_EMPTY_BASES AstDoWhileStmt final : public AstStmt, public AstPayload<AstDoWhileStmt>
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

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
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
    class MSVC_EMPTY_BASES AstWhileStmt final : public AstStmt, public AstPayload<AstWhileStmt>
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

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* predicate;
        AstStmt* loopBody;
    };
    class MSVC_EMPTY_BASES AstIfStmt final : public AstStmt, public AstPayload<AstIfStmt>
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

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* predicate;
        AstStmt* ifBranch;
        AstStmt* elseBranch;
    };
    class MSVC_EMPTY_BASES AstLabelStmt final : public AstStmt, public AstPayload<AstLabelStmt>
    {
    public:
        AstLabelStmt(AstExpr* caseExpr) : caseExpr(caseExpr)
        {
        }

        auto GetCaseExpr() -> AstExpr*
        {
            return caseExpr;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
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
    class MSVC_EMPTY_BASES AstSwitchStmt final : public AstStmt, public AstPayload<AstSwitchStmt>
    {
    public:
        AstSwitchStmt(AstExpr* expr, AstStmt* child) : expr(expr), child(child)
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*expr);
            visitor.Traverse(*child);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* expr;
        AstStmt* child;
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

        auto DumpNodeData() const -> std::string
        {
            // FIXME: use different AST types
            return "";
        }

    private:
        JumpType type;
    };
    class MSVC_EMPTY_BASES AstReturnStmt final : public AstStmt, public AstPayload<AstReturnStmt>
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
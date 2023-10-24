#pragma once
#include "Basic/Common.h"
#include "Ast/Base.h"
#include "Language/Semantic.h"

#include <vector>

namespace glsld
{
    // Base class for all AST nodes that could be used as a statement.
    class AstStmt : public AstNode
    {
    protected:
        AstStmt() = default;

        template <typename Dumper>
        auto DumpPayload(Dumper& d) const -> void
        {
        }
    };

    class AstErrorStmt final : public AstStmt
    {
    public:
        AstErrorStmt()
        {
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
        }
    };

    class AstEmptyStmt final : public AstStmt
    {
    public:
        AstEmptyStmt()
        {
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
        }
    };

    class AstCompoundStmt final : public AstStmt
    {
    private:
        std::vector</*NotNull*/ AstStmt*> children;

    public:
        AstCompoundStmt(std::vector<AstStmt*> children) : children(std::move(children))
        {
        }

        auto GetChildren() const noexcept -> ArrayView<const AstStmt*>
        {
            return children;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            for (auto stmt : children) {
                if (!visitor.Traverse(*stmt)) {
                    return false;
                }
            }
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            for (auto stmt : children) {
                d.DumpChildNode("Child", *stmt);
            }
        }
    };

    class AstExprStmt final : public AstStmt
    {
    private:
        NotNull<AstExpr*> expr;

    public:
        AstExprStmt(AstExpr* expr) : expr(expr)
        {
        }

        auto GetExpr() const noexcept -> const AstExpr*
        {
            return expr;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            return visitor.Traverse(*expr);
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            d.DumpChildNode("Expr", *expr);
        }
    };

    class AstDeclStmt final : public AstStmt
    {
    private:
        NotNull<AstDecl*> decl;

    public:
        AstDeclStmt(AstDecl* decl) : decl(decl)
        {
        }

        auto GetDecl() const noexcept -> const AstDecl*
        {
            return decl;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            return visitor.Traverse(*decl);
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            d.DumpChildNode("Decl", *decl);
        }
    };

    class AstIfStmt final : public AstStmt
    {
    private:
        NotNull<AstExpr*> condition;
        NotNull<AstStmt*> thenStmt;
        AstStmt* elseStmt;

    public:
        AstIfStmt(AstExpr* condition, AstStmt* thenStmt, AstStmt* elseStmt)
            : condition(condition), thenStmt(thenStmt), elseStmt(elseStmt)
        {
        }

        auto GetConditionExpr() const noexcept -> const AstExpr*
        {
            return condition;
        }
        auto GetThenStmt() const noexcept -> const AstStmt*
        {
            return thenStmt;
        }
        auto GetElseStmt() const noexcept -> const AstStmt*
        {
            return elseStmt;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(*condition)) {
                return false;
            }
            if (!visitor.Traverse(*thenStmt)) {
                return false;
            }
            if (elseStmt && !visitor.Traverse(*elseStmt)) {
                return false;
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            d.DumpChildNode("Condition", *condition);
            d.DumpChildNode("Then", *thenStmt);
            if (elseStmt) {
                d.DumpChildNode("Else", *elseStmt);
            }
        }
    };

    class AstForStmt final : public AstStmt
    {
    private:
        NotNull<AstStmt*> init;
        AstExpr* condition;
        AstExpr* iter;
        NotNull<AstStmt*> body;

    public:
        AstForStmt(AstStmt* init, AstExpr* condition, AstExpr* iter, AstStmt* body)
            : init(init), condition(condition), iter(iter), body(body)
        {
        }

        auto GetInitStmt() const noexcept -> const AstStmt*
        {
            return init;
        }
        auto GetConditionExpr() const noexcept -> const AstExpr*
        {
            return condition;
        }
        auto GetIterExpr() const noexcept -> const AstExpr*
        {
            return iter;
        }
        auto GetBody() const noexcept -> const AstStmt*
        {
            return body;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(*init)) {
                return false;
            }
            if (condition && !visitor.Traverse(*condition)) {
                return false;
            }
            if (iter && !visitor.Traverse(*iter)) {
                return false;
            }
            if (!visitor.Traverse(*body)) {
                return false;
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            d.DumpChildNode("Init", *init);
            if (condition) {
                d.DumpChildNode("Condition", *condition);
            }
            if (iter) {
                d.DumpChildNode("Iter", *iter);
            }
            d.DumpChildNode("Body", *body);
        }
    };

    class AstWhileStmt final : public AstStmt
    {
    private:
        NotNull<AstExpr*> condition;
        NotNull<AstStmt*> body;

    public:
        AstWhileStmt(AstExpr* condition, AstStmt* body) : condition(condition), body(body)
        {
        }

        auto GetConditionExpr() const noexcept -> const AstExpr*
        {
            return condition;
        }
        auto GetBody() const noexcept -> const AstStmt*
        {
            return body;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(*condition)) {
                return false;
            }
            if (!visitor.Traverse(*body)) {
                return false;
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            d.DumpChildNode("Condition", *condition);
            d.DumpChildNode("Body", *body);
        }
    };

    class AstDoWhileStmt final : public AstStmt
    {
    private:
        NotNull<AstExpr*> condition;
        NotNull<AstStmt*> body;

    public:
        AstDoWhileStmt(AstExpr* condition, AstStmt* body) : condition(condition), body(body)
        {
        }

        auto GetConditionExpr() const noexcept -> const AstExpr*
        {
            return condition;
        }
        auto GetBody() const noexcept -> const AstStmt*
        {
            return body;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(*condition)) {
                return false;
            }
            if (!visitor.Traverse(*body)) {
                return false;
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            d.DumpChildNode("Condition", *condition);
            d.DumpChildNode("Body", *body);
        }
    };

    class AstLabelStmt final : public AstStmt
    {
    private:
        // nullptr if this is default case
        AstExpr* caseExpr;

    public:
        AstLabelStmt() : caseExpr(nullptr)
        {
        }
        AstLabelStmt(AstExpr* caseExpr) : caseExpr(caseExpr)
        {
        }

        auto GetCaseExpr() const noexcept -> const AstExpr*
        {
            return caseExpr;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (caseExpr && !visitor.Traverse(*caseExpr)) {
                return false;
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            if (caseExpr) {
                d.DumpAttribute("Kind", "CaseLabel");
                d.DumpChildNode("CaseExpr", *caseExpr);
            }
            else {
                d.DumpAttribute("Kind", "DefaultLabel");
            }
        }
    };

    class AstSwitchStmt final : public AstStmt
    {
    private:
        NotNull<AstExpr*> testExpr;
        NotNull<AstStmt*> body;

    public:
        AstSwitchStmt(AstExpr* testExpr, AstStmt* body) : testExpr(testExpr), body(body)
        {
        }

        auto GetTestExpr() const noexcept -> const AstExpr*
        {
            return testExpr;
        }
        auto GetBody() const noexcept -> const AstStmt*
        {
            return body;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(*testExpr)) {
                return false;
            }
            if (!visitor.Traverse(*body)) {
                return false;
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            d.DumpChildNode("TestExpr", *testExpr);
            d.DumpChildNode("Body", *body);
        }
    };

    class AstJumpStmt final : public AstStmt
    {
    private:
        JumpType jumpType;

    public:
        AstJumpStmt(JumpType type) : jumpType(type)
        {
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            d.DumpAttribute("JumpType", JumpTypeToString(jumpType));
        }
    };

    class AstReturnStmt final : public AstStmt
    {
    private:
        AstExpr* expr;

    public:
        AstReturnStmt(AstExpr* expr) : expr(expr)
        {
        }

        auto GetExpr() const noexcept -> const AstExpr*
        {
            return expr;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (expr && !visitor.Traverse(*expr)) {
                return false;
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstStmt::DumpPayload(d);
            if (expr) {
                d.DumpChildNode("Expr", *expr);
            }
        }
    };

} // namespace glsld
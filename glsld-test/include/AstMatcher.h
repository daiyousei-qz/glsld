#pragma once
#include "Ast/AstVisitor.h"
#include "Ast/Base.h"
#include "Ast/Expr.h"
#include "Language/ConstValue.h"

#include <functional>

namespace glsld
{
    struct AstChildCollector : public AstVisitor<AstChildCollector, AstVisitorConfig{.traceTraversalPath = true}>
    {
        std::vector<const AstNode*> children;

        auto EnterAstNode(const AstNode& node) -> AstVisitPolicy
        {
            if (GetTraversalDepth() == 0) {
                return AstVisitPolicy::Traverse;
            }
            else {
                children.push_back(&node);
                return AstVisitPolicy::Leave;
            }
        }
    };

    class AstMatcher
    {
    private:
        struct AstMatcherData
        {
            std::function<bool(const AstNode&)> matchThis;
            std::vector<AstMatcher> matchChildren;
        };

        std::unique_ptr<AstMatcherData> data;

    public:
        template <std::same_as<AstMatcher>... MatcherType>
        AstMatcher(std::function<bool(const AstNode&)> matchThis, MatcherType... matchChild)
        {
            data            = std::make_unique<AstMatcherData>();
            data->matchThis = std::move(matchThis);
            (data->matchChildren.push_back(std::move(matchChild)), ...);
        }

        AstMatcher(const AstMatcher&)                    = delete;
        AstMatcher(AstMatcher&&)                         = default;
        auto operator=(const AstMatcher&) -> AstMatcher& = delete;
        auto operator=(AstMatcher&&) -> AstMatcher&      = default;

        auto Match(const AstNode& node) const -> bool
        {
            if (!data->matchThis(node)) {
                return false;
            }

            AstChildCollector collector;
            collector.Traverse(node);

            if (collector.children.size() != data->matchChildren.size()) {
                return false;
            }

            for (size_t i = 0; i < collector.children.size(); ++i) {
                if (!data->matchChildren[i].Match(*collector.children[i])) {
                    return false;
                }
            }

            return true;
        }
    };

    struct DeclaratorMatcher
    {
        AstMatcher matchArraySpec;
        AstMatcher matchInitializer;
    };

    inline auto Any() -> AstMatcher
    {
        return AstMatcher([](const AstNode&) -> bool { return true; });
    }

#pragma region Initializer

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto InitializerList(MatcherType... matchItems) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstInitializerList>(); },
                          std::move(matchItems)...);
    }

    inline auto ErrorExpr() -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstErrorExpr>(); });
    }

    template <typename T>
    inline auto LiteralExpr(T value) -> AstMatcher
    {
        // FIXME: this is a hack for std::function has a copy ctor. We should use std::move_only_function instead in
        // c++23.
        std::function<bool(const AstNode&)> f =
            [value = std::make_shared<ConstValue>(ConstValue::FromValue(value))](const AstNode& node) -> bool {
            auto expr = node.As<AstLiteralExpr>();
            return expr && expr->GetValue() == *value;
        };
        return AstMatcher(std::move(f));
    }

    inline auto NameAccessExpr(StringView name) -> AstMatcher
    {
        return AstMatcher([name = name.Str()](const AstNode& node) -> bool {
            auto expr = node.As<AstNameAccessExpr>();
            return expr && expr->GetAccessName().text == name;
        });
    }

    inline auto FieldAccessExpr(AstMatcher matchLhs, StringView name) -> AstMatcher
    {
        return AstMatcher(
            [name = name.Str()](const AstNode& node) -> bool {
                auto expr = node.As<AstFieldAccessExpr>();
                return expr && expr->GetAccessName().text == name;
            },
            std::move(matchLhs));
    }

    inline auto SwizzleAccessExpr(AstMatcher matchLhs, StringView swizzle) -> AstMatcher
    {
        return AstMatcher(
            [swizzle = swizzle.Str()](const AstNode& node) -> bool {
                auto expr = node.As<AstSwizzleAccessExpr>();
                return expr && expr->GetSwizzleDesc().ToString() == swizzle;
            },
            std::move(matchLhs));
    }

    inline auto IndexAccessExpr(AstMatcher matchLhs, AstMatcher matchArraySpec) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstIndexAccessExpr>(); },
                          std::move(matchLhs), std::move(matchArraySpec));
    }

    inline auto UnaryExpr(UnaryOp op, AstMatcher matchOperand) -> AstMatcher
    {
        return AstMatcher(
            [op](const AstNode& node) -> bool {
                auto expr = node.As<AstUnaryExpr>();
                return expr && expr->GetOpcode() == op;
            },
            std::move(matchOperand));
    }

    inline auto BinaryExpr(BinaryOp op, AstMatcher matchLhs, AstMatcher matchRhs) -> AstMatcher
    {
        return AstMatcher(
            [op](const AstNode& node) -> bool {
                auto expr = node.As<AstBinaryExpr>();
                return expr && expr->GetOpcode() == op;
            },
            std::move(matchLhs), std::move(matchRhs));
    }

    inline auto SelectExpr(AstMatcher matchCondition, AstMatcher matchTrueExpr, AstMatcher matchFalseExpr) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstSelectExpr>(); },
                          std::move(matchCondition), std::move(matchTrueExpr), std::move(matchFalseExpr));
    }

    inline auto ImplicitCastExpr(AstMatcher matchOperand) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstImplicitCastExpr>(); },
                          std::move(matchOperand));
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto FunctionCallExpr(StringView name, MatcherType... matchArgs) -> AstMatcher
    {
        return AstMatcher(
            [name](const AstNode& node) -> bool {
                auto expr = node.As<AstFunctionCallExpr>();
                return expr && expr->GetFunctionName().text == name;
            },
            std::move(matchArgs)...);
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto ConstructorCallExpr(AstMatcher matchTypeSpec, MatcherType... matchArgs) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstConstructorCallExpr>(); },
                          std::move(matchTypeSpec), std::move(matchArgs)...);
    }
#pragma endregion

#pragma region Stmt

    inline auto ErrorStmt() -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstErrorStmt>(); });
    }

    inline auto EmptyStmt() -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstEmptyStmt>(); });
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto CompoundStmt(MatcherType... matchStmts) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstCompoundStmt>(); },
                          std::move(matchStmts)...);
    }

    inline auto ExprStmt(AstMatcher matchExpr) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstExprStmt>(); }, std::move(matchExpr));
    }

    inline auto DeclStmt(AstMatcher matchDecl) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstDeclStmt>(); }, std::move(matchDecl));
    }

    inline auto IfStmt(AstMatcher matchCondition, AstMatcher matchThenStmt) -> AstMatcher
    {
        return AstMatcher(
            [](const AstNode& node) -> bool {
                auto stmt = node.As<AstIfStmt>();
                return stmt && stmt->GetElseStmt() == nullptr;
            },
            std::move(matchCondition), std::move(matchThenStmt));
    }

    inline auto IfStmt(AstMatcher matchCondition, AstMatcher matchThenStmt, AstMatcher matchElseStmt) -> AstMatcher
    {
        return AstMatcher(
            [](const AstNode& node) -> bool {
                auto stmt = node.As<AstIfStmt>();
                return stmt && stmt->GetElseStmt() != nullptr;
            },
            std::move(matchCondition), std::move(matchThenStmt), std::move(matchElseStmt));
    }

    inline auto ForStmt(AstMatcher matchInit, AstMatcher matchCondition, AstMatcher matchIter, AstMatcher matchBody)
        -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstForStmt>(); }, std::move(matchInit),
                          std::move(matchCondition), std::move(matchIter), std::move(matchBody));
    }

    inline auto WhileStmt(AstMatcher matchCondition, AstMatcher matchBody) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstWhileStmt>(); },
                          std::move(matchCondition), std::move(matchBody));
    }

    inline auto DoWhileStmt(AstMatcher matchBody, AstMatcher matchCondition) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstDoWhileStmt>(); }, std::move(matchBody),
                          std::move(matchCondition));
    }

    inline auto CaseLabelStmt(AstMatcher matchExpr) -> AstMatcher
    {
        return AstMatcher(
            [](const AstNode& node) -> bool {
                auto stmt = node.As<AstLabelStmt>();
                return stmt && stmt->GetCaseExpr() != nullptr;
            },
            std::move(matchExpr));
    }

    inline auto DefaultLabelStmt() -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool {
            auto stmt = node.As<AstLabelStmt>();
            return stmt && stmt->GetCaseExpr() == nullptr;
        });
    }

    inline auto SwitchStmt(AstMatcher matchTestExpr, AstMatcher matchBody) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstSwitchStmt>(); },
                          std::move(matchTestExpr), std::move(matchBody));
    }

    inline auto BreakStmt() -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool {
            auto stmt = node.As<AstJumpStmt>();
            return stmt && stmt->GetJumpType() == JumpType::Break;
        });
    }

    inline auto ContinueStmt() -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool {
            auto stmt = node.As<AstJumpStmt>();
            return stmt && stmt->GetJumpType() == JumpType::Continue;
        });
    }

    inline auto ReturnStmt() -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstReturnStmt>(); });
    }

    inline auto ReturnStmt(AstMatcher matchExpr) -> AstMatcher
    {
        return AstMatcher(
            [](const AstNode& node) -> bool {
                auto stmt = node.As<AstReturnStmt>();
                return stmt && stmt->GetExpr() != nullptr;
            },
            std::move(matchExpr));
    }

#pragma endregion

    inline auto VariableDecl1(AstMatcher matchQualType, AstMatcher initializer) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstVariableDecl>(); },
                          std::move(matchQualType), std::move(initializer));
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto TranslationUnit(MatcherType... matchDecls) -> AstMatcher
    {
        return AstMatcher([](const AstNode& node) -> bool { return node.Is<AstTranslationUnit>(); },
                          std::move(matchDecls)...);
    }

} // namespace glsld
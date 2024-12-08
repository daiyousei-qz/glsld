#pragma once
#include "Ast/AstVisitor.h"
#include "Ast/Base.h"
#include "Ast/Expr.h"
#include "Language/ConstValue.h"

#include <functional>

namespace glsld
{
    class AstMatcher;

    class AstMatchResult
    {
    private:
        const AstNode* failedNode       = nullptr;
        const AstMatcher* failedMatcher = nullptr;

        AstMatchResult(const AstNode* failedNode, const AstMatcher* failedMatcher)
            : failedNode(failedNode), failedMatcher(failedMatcher)
        {
        }

    public:
        bool IsSuccess() const
        {
            return failedNode == nullptr;
        }

        auto GetFailedNode() const -> const AstNode*
        {
            return failedNode;
        }

        auto GetFailedMatcher() const -> const AstMatcher*
        {
            return failedMatcher;
        }

        static auto Success() -> AstMatchResult
        {
            return AstMatchResult{nullptr, nullptr};
        }

        static auto Failure(const AstNode& failedNode, const AstMatcher& failedMatcher) -> AstMatchResult
        {
            return AstMatchResult{&failedNode, &failedMatcher};
        }
    };

    class SyntaxTokenMatcher
    {
    private:
    public:
    };

    class AstMatcher
    {
    private:
        struct AstMatcherData
        {
            std::string desc;
            bool findMatchMode = false;
            std::function<bool(const AstNode&)> matchThis;
            std::vector<AstMatcher> matchChildren;
        };

        std::unique_ptr<AstMatcherData> data;

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

    public:
        template <std::same_as<AstMatcher>... MatcherType>
        AstMatcher(std::string desc, std::function<auto(const AstNode&)->bool> matchThis, MatcherType... matchChild)
        {
            data            = std::make_unique<AstMatcherData>();
            data->desc      = std::move(desc);
            data->matchThis = std::move(matchThis);
            (data->matchChildren.push_back(std::move(matchChild)), ...);
        }

        AstMatcher(std::string desc, AstMatcher finder, AstMatcher matcher)
        {
            data                = std::make_unique<AstMatcherData>();
            data->desc          = std::move(desc);
            data->findMatchMode = true;
            data->matchChildren.push_back(std::move(finder));
            data->matchChildren.push_back(std::move(matcher));
        }

        AstMatcher(const AstMatcher&)                    = delete;
        AstMatcher(AstMatcher&&)                         = default;
        auto operator=(const AstMatcher&) -> AstMatcher& = delete;
        auto operator=(AstMatcher&&) -> AstMatcher&      = default;

        auto Match(const AstNode& node) const -> AstMatchResult
        {
            AstChildCollector collector;
            collector.Traverse(node);

            if (data->findMatchMode) {
                auto& finder  = data->matchChildren[0];
                auto& matcher = data->matchChildren[1];

                for (const auto* child : collector.children) {
                    auto found = finder.Match(*child);
                    if (found.IsSuccess()) {
                        return matcher.Match(*child);
                    }
                }

                return AstMatchResult::Failure(node, *this);
            }
            else {
                if (!data->matchThis(node)) {
                    return AstMatchResult::Failure(node, *this);
                }

                for (size_t i = 0; i < std::min(data->matchChildren.size(), collector.children.size()); ++i) {
                    auto result = data->matchChildren[i].Match(*collector.children[i]);
                    if (!result.IsSuccess()) {
                        return result;
                    }
                }

                return AstMatchResult::Success();
            }
        }
    };

    struct DeclaratorMatcher
    {
        AstMatcher matchArraySpec;
        AstMatcher matchInitializer;
    };

    inline auto Any() -> AstMatcher
    {
        return AstMatcher("Any", [](const AstNode&) -> bool { return true; });
    }
    inline auto AnyInitializer() -> AstMatcher
    {
        return AstMatcher("AnyInitializer", [](const AstNode& node) -> bool { return node.Is<AstInitializer>(); });
    }
    inline auto AnyExpr() -> AstMatcher
    {
        return AstMatcher("AnyExpr", [](const AstNode& node) -> bool { return node.Is<AstExpr>(); });
    }
    inline auto AnyStmt() -> AstMatcher
    {
        return AstMatcher("AnyStmt", [](const AstNode& node) -> bool { return node.Is<AstStmt>(); });
    }
    inline auto AnyDecl() -> AstMatcher
    {
        return AstMatcher("AnyDecl", [](const AstNode& node) -> bool { return node.Is<AstDecl>(); });
    }
    inline auto AnyQualType() -> AstMatcher
    {
        return AstMatcher("AnyTypeSpec", [](const AstNode& node) -> bool { return node.Is<AstQualType>(); });
    }

#pragma region Type

    inline auto BuiltinType(GlslBuiltinType type) -> AstMatcher
    {
        return AstMatcher("BuiltinType", [type](const AstNode& node) -> bool {
            auto qualType = node.As<AstQualType>();
            return qualType && qualType->GetResolvedType() == Type::GetBuiltinType(type);
        });
    }

    inline auto NamedType(StringView name) -> AstMatcher
    {
        return AstMatcher("NamedType", [name = name.Str()](const AstNode& node) -> bool {
            auto qualType = node.As<AstQualType>();
            return qualType && qualType->GetTypeNameTok().IsIdentifier() && qualType->GetTypeNameTok().text == name;
        });
    }

#pragma endregion

#pragma region Misc

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto ArraySpec(MatcherType... matchExpr) -> AstMatcher
    {
        return AstMatcher(
            "ArraySpec",
            [numSpec = sizeof...(matchExpr)](const AstNode& node) -> bool {
                auto arraySpec = node.As<AstArraySpec>();
                return arraySpec && arraySpec->GetSizeList().size() == numSpec;
            },
            std::move(matchExpr)...);
    }

#pragma endregion

#pragma region Initializer

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto InitializerList(MatcherType... matchItems) -> AstMatcher
    {
        return AstMatcher(
            "InitializerList", [](const AstNode& node) -> bool { return node.Is<AstInitializerList>(); },
            std::move(matchItems)...);
    }

    inline auto ErrorExpr() -> AstMatcher
    {
        return AstMatcher("ErrorExpr", [](const AstNode& node) -> bool { return node.Is<AstErrorExpr>(); });
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
        return AstMatcher("LiteralExpr", std::move(f));
    }

    inline auto NameAccessExpr(StringView name) -> AstMatcher
    {
        return AstMatcher("NameAccessExpr", [name = name.Str()](const AstNode& node) -> bool {
            auto expr = node.As<AstNameAccessExpr>();
            return expr && expr->GetAccessName().text == name;
        });
    }

    inline auto FieldAccessExpr(AstMatcher matchLhs, StringView name) -> AstMatcher
    {
        return AstMatcher(
            "FieldAccessExpr",
            [name = name.Str()](const AstNode& node) -> bool {
                auto expr = node.As<AstFieldAccessExpr>();
                return expr && expr->GetAccessName().text == name;
            },
            std::move(matchLhs));
    }

    inline auto SwizzleAccessExpr(AstMatcher matchLhs, StringView swizzle) -> AstMatcher
    {
        return AstMatcher(
            "SwizzleAccessExpr",
            [swizzle = swizzle.Str()](const AstNode& node) -> bool {
                auto expr = node.As<AstSwizzleAccessExpr>();
                return expr && expr->GetSwizzleDesc().ToString() == swizzle;
            },
            std::move(matchLhs));
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto IndexAccessExpr(AstMatcher matchLhs, MatcherType... matchIndices) -> AstMatcher
    {
        return AstMatcher(
            "IndexAccessExpr", [](const AstNode& node) -> bool { return node.Is<AstIndexAccessExpr>(); },
            std::move(matchLhs), ArraySpec(std::move(matchIndices)...));
    }

    inline auto UnaryExpr(UnaryOp op, AstMatcher matchOperand) -> AstMatcher
    {
        return AstMatcher(
            "UnaryExpr",
            [op](const AstNode& node) -> bool {
                auto expr = node.As<AstUnaryExpr>();
                return expr && expr->GetOpcode() == op;
            },
            std::move(matchOperand));
    }

    inline auto BinaryExpr(BinaryOp op, AstMatcher matchLhs, AstMatcher matchRhs) -> AstMatcher
    {
        return AstMatcher(
            "BinaryExpr",
            [op](const AstNode& node) -> bool {
                auto expr = node.As<AstBinaryExpr>();
                return expr && expr->GetOpcode() == op;
            },
            std::move(matchLhs), std::move(matchRhs));
    }

    inline auto SelectExpr(AstMatcher matchCondition, AstMatcher matchTrueExpr, AstMatcher matchFalseExpr) -> AstMatcher
    {
        return AstMatcher(
            "SelectExpr", [](const AstNode& node) -> bool { return node.Is<AstSelectExpr>(); },
            std::move(matchCondition), std::move(matchTrueExpr), std::move(matchFalseExpr));
    }

    inline auto ImplicitCastExpr(AstMatcher matchOperand) -> AstMatcher
    {
        return AstMatcher(
            "ImplicitCastExpr", [](const AstNode& node) -> bool { return node.Is<AstImplicitCastExpr>(); },
            std::move(matchOperand));
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto FunctionCallExpr(StringView name, MatcherType... matchArgs) -> AstMatcher
    {
        return AstMatcher(
            "FunctionCallExpr",
            [name, numArgs = sizeof...(matchArgs)](const AstNode& node) -> bool {
                auto expr = node.As<AstFunctionCallExpr>();
                return expr && expr->GetFunctionName().text == name && expr->GetArgs().size() == numArgs;
            },
            std::move(matchArgs)...);
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto ConstructorCallExpr(AstMatcher matchTypeSpec, MatcherType... matchArgs) -> AstMatcher
    {
        return AstMatcher(
            "ConstructorCallExpr", [](const AstNode& node) -> bool { return node.Is<AstConstructorCallExpr>(); },
            std::move(matchTypeSpec), std::move(matchArgs)...);
    }
#pragma endregion

#pragma region Stmt

    inline auto ErrorStmt() -> AstMatcher
    {
        return AstMatcher("ErrorStmt", [](const AstNode& node) -> bool { return node.Is<AstErrorStmt>(); });
    }

    inline auto EmptyStmt() -> AstMatcher
    {
        return AstMatcher("EmptyStmt", [](const AstNode& node) -> bool { return node.Is<AstEmptyStmt>(); });
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto CompoundStmt(MatcherType... matchStmts) -> AstMatcher
    {
        return AstMatcher(
            "CompoundStmt", [](const AstNode& node) -> bool { return node.Is<AstCompoundStmt>(); },
            std::move(matchStmts)...);
    }

    inline auto ExprStmt(AstMatcher matchExpr) -> AstMatcher
    {
        return AstMatcher(
            "ExprStmt", [](const AstNode& node) -> bool { return node.Is<AstExprStmt>(); }, std::move(matchExpr));
    }

    inline auto DeclStmt(AstMatcher matchDecl) -> AstMatcher
    {
        return AstMatcher(
            "DeclStmt", [](const AstNode& node) -> bool { return node.Is<AstDeclStmt>(); }, std::move(matchDecl));
    }

    inline auto IfStmt(AstMatcher matchCondition, AstMatcher matchThenStmt) -> AstMatcher
    {
        return AstMatcher(
            "IfStmt",
            [](const AstNode& node) -> bool {
                auto stmt = node.As<AstIfStmt>();
                return stmt && stmt->GetElseStmt() == nullptr;
            },
            std::move(matchCondition), std::move(matchThenStmt));
    }

    inline auto IfStmt(AstMatcher matchCondition, AstMatcher matchThenStmt, AstMatcher matchElseStmt) -> AstMatcher
    {
        return AstMatcher(
            "IfStmt",
            [](const AstNode& node) -> bool {
                auto stmt = node.As<AstIfStmt>();
                return stmt && stmt->GetElseStmt() != nullptr;
            },
            std::move(matchCondition), std::move(matchThenStmt), std::move(matchElseStmt));
    }

    inline auto ForStmt(AstMatcher matchInit, AstMatcher matchCondition, AstMatcher matchIter, AstMatcher matchBody)
        -> AstMatcher
    {
        return AstMatcher(
            "ForStmt", [](const AstNode& node) -> bool { return node.Is<AstForStmt>(); }, std::move(matchInit),
            std::move(matchCondition), std::move(matchIter), std::move(matchBody));
    }

    inline auto WhileStmt(AstMatcher matchCondition, AstMatcher matchBody) -> AstMatcher
    {
        return AstMatcher(
            "WhileStmt", [](const AstNode& node) -> bool { return node.Is<AstWhileStmt>(); }, std::move(matchCondition),
            std::move(matchBody));
    }

    inline auto DoWhileStmt(AstMatcher matchBody, AstMatcher matchCondition) -> AstMatcher
    {
        return AstMatcher(
            "DoWhileStmt", [](const AstNode& node) -> bool { return node.Is<AstDoWhileStmt>(); }, std::move(matchBody),
            std::move(matchCondition));
    }

    inline auto CaseLabelStmt(AstMatcher matchExpr) -> AstMatcher
    {
        return AstMatcher(
            "CaseLabelStmt",
            [](const AstNode& node) -> bool {
                auto stmt = node.As<AstLabelStmt>();
                return stmt && stmt->GetCaseExpr() != nullptr;
            },
            std::move(matchExpr));
    }

    inline auto DefaultLabelStmt() -> AstMatcher
    {
        return AstMatcher("DefaultLabelStmt", [](const AstNode& node) -> bool {
            auto stmt = node.As<AstLabelStmt>();
            return stmt && stmt->GetCaseExpr() == nullptr;
        });
    }

    inline auto SwitchStmt(AstMatcher matchTestExpr, AstMatcher matchBody) -> AstMatcher
    {
        return AstMatcher(
            "SwitchStmt", [](const AstNode& node) -> bool { return node.Is<AstSwitchStmt>(); },
            std::move(matchTestExpr), std::move(matchBody));
    }

    inline auto BreakStmt() -> AstMatcher
    {
        return AstMatcher("BreakStmt", [](const AstNode& node) -> bool {
            auto stmt = node.As<AstJumpStmt>();
            return stmt && stmt->GetJumpType() == JumpType::Break;
        });
    }

    inline auto ContinueStmt() -> AstMatcher
    {
        return AstMatcher("ContinueStmt", [](const AstNode& node) -> bool {
            auto stmt = node.As<AstJumpStmt>();
            return stmt && stmt->GetJumpType() == JumpType::Continue;
        });
    }

    inline auto ReturnStmt() -> AstMatcher
    {
        return AstMatcher("ReturnStmt", [](const AstNode& node) -> bool { return node.Is<AstReturnStmt>(); });
    }

    inline auto ReturnStmt(AstMatcher matchExpr) -> AstMatcher
    {
        return AstMatcher(
            "ReturnStmt",
            [](const AstNode& node) -> bool {
                auto stmt = node.As<AstReturnStmt>();
                return stmt && stmt->GetExpr() != nullptr;
            },
            std::move(matchExpr));
    }

#pragma endregion

    inline auto VariableDecl1(AstMatcher matchQualType, AstMatcher initializer) -> AstMatcher
    {
        return AstMatcher(
            "VariableDecl1", [](const AstNode& node) -> bool { return node.Is<AstVariableDecl>(); },
            std::move(matchQualType), std::move(initializer));
    }

    inline auto FunctionDecl(AstMatcher matchQualType, AstMatcher matchParams, AstMatcher matchBody) -> AstMatcher
    {
        return AstMatcher(
            "FunctionDecl", [](const AstNode& node) -> bool { return node.Is<AstFunctionDecl>(); },
            std::move(matchQualType), std::move(matchParams), std::move(matchBody));
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto TranslationUnit(MatcherType... matchDecls) -> AstMatcher
    {
        return AstMatcher(
            "TranslationUnit", [](const AstNode& node) -> bool { return node.Is<AstTranslationUnit>(); },
            std::move(matchDecls)...);
    }

    inline auto FindMatch(AstMatcher finder, AstMatcher matcher) -> AstMatcher
    {
        return AstMatcher("FindMatch", std::move(finder), std::move(matcher));
    }

} // namespace glsld
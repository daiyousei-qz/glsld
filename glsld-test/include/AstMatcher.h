#pragma once
#include "TokenMatcher.h"

#include "Ast/AstVisitor.h"
#include "Ast/Base.h"
#include "Ast/Expr.h"
#include "Ast/Eval.h"
#include "Language/ConstValue.h"

#include <functional>
#include <memory>

namespace glsld
{
    class AstMatcher;

    class AstMatchResult
    {
    private:
        const AstNode* failedNode = nullptr;
        std::vector<std::string> errorTraces;

        AstMatchResult(const AstNode* failedNode) : failedNode(failedNode)
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

        auto GetErrorTrace() const -> std::string
        {
            std::string result;
            for (const auto& trace : errorTraces) {
                result += trace;
                result += '\n';
            }
            return result;
        }

        auto AddErrorTrace(std::string trace) -> void
        {
            errorTraces.push_back(std::move(trace));
        }

        static auto Success() -> AstMatchResult
        {
            return AstMatchResult{nullptr};
        }

        static auto Failure(const AstNode& failedNode) -> AstMatchResult
        {
            return AstMatchResult{&failedNode};
        }
    };

    class AstMatcher
    {
    private:
        struct AstMatcherData
        {
            // Description of this matcher. Used for composing error message.
            std::string desc;
            bool findMatchMode = false;
            // Callback to match the current node.
            std::move_only_function<auto(const AstNode&)->AstMatchResult> matchCallback;
            // Additional checkers for the deduced type of AST expression.
            std::move_only_function<auto(const Type&)->bool> typeChecker;
            // Additional checkers for the evaluated value of AST expression.
            std::move_only_function<auto(const ConstValue&)->bool> valueChecker;

            // Matchers for children nodes.
            std::vector<AstMatcher> childrenMatcher;
            // The matched node will be captured to this pointer.
            const AstNode** capturePtr = nullptr;
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
        AstMatcher(std::string desc, std::move_only_function<auto(const AstNode&)->AstMatchResult> matchCallback)
        {
            data                = std::make_unique<AstMatcherData>();
            data->desc          = std::move(desc);
            data->matchCallback = std::move(matchCallback);
        }

        AstMatcher(std::string desc, AstMatcher finder, AstMatcher matcher)
        {
            data                = std::make_unique<AstMatcherData>();
            data->desc          = std::move(desc);
            data->findMatchMode = true;
            data->childrenMatcher.push_back(std::move(finder));
            data->childrenMatcher.push_back(std::move(matcher));
        }

        AstMatcher(const AstMatcher&)                    = delete;
        AstMatcher(AstMatcher&&)                         = default;
        auto operator=(const AstMatcher&) -> AstMatcher& = delete;
        auto operator=(AstMatcher&&) -> AstMatcher&      = default;

        auto Capture(const AstNode*& capture) && -> AstMatcher&&
        {
            data->capturePtr = &capture;
            return std::move(*this);
        }

        auto CheckType(std::move_only_function<auto(const Type&)->bool> typeChecker) && -> AstMatcher&&
        {
            data->typeChecker = std::move(typeChecker);
            return std::move(*this);
        }

        auto CheckValue(std::move_only_function<auto(const ConstValue&)->bool> valueChecker) && -> AstMatcher&&
        {
            data->valueChecker = std::move(valueChecker);
            return std::move(*this);
        }

        auto Match(const AstNode& node) const -> AstMatchResult
        {
            AstChildCollector collector;
            collector.Traverse(node);

            if (data->findMatchMode) {
                auto& finder  = data->childrenMatcher[0];
                auto& matcher = data->childrenMatcher[1];

                for (const auto* child : collector.children) {
                    auto found = finder.Match(*child);
                    if (found.IsSuccess()) {
                        return matcher.Match(*child);
                    }
                }

                return AstMatchResult::Failure(node);
            }
            else {
                if (data->matchCallback) {
                    auto matchResult = data->matchCallback(node);
                    if (!matchResult.IsSuccess()) {
                        matchResult.AddErrorTrace(data->desc);
                        return matchResult;
                    }
                }

                if (auto expr = node.As<AstExpr>(); expr) {
                    if (data->typeChecker && !data->typeChecker(*expr->GetDeducedType())) {
                        return AstMatchResult::Failure(node);
                    }
                    if (data->valueChecker && !data->valueChecker(EvalAstExpr(*expr))) {
                        return AstMatchResult::Failure(node);
                    }
                }

                if (data->capturePtr) {
                    *data->capturePtr = &node;
                }
                return AstMatchResult::Success();
            }
        }

        // Collect multiple matchers into a vector.
        // This is useful as matchers are move-only and thus not allowed in initializer list.
        template <std::same_as<AstMatcher>... MatcherType>
        static auto CollectToVec(MatcherType... matchers) -> std::vector<AstMatcher>
        {
            std::vector<AstMatcher> result;
            result.reserve(sizeof...(matchers));
            (result.push_back(std::move(matchers)), ...);

            return result;
        }

        static auto MatchAll(std::initializer_list<std::pair<const AstNode*, const AstMatcher*>> requests)
            -> AstMatchResult
        {
            for (const auto& [node, matcher] : requests) {
                if (auto result = matcher->Match(*node); !result.IsSuccess()) {
                    return result;
                }
            }

            return AstMatchResult::Success();
        }

        template <std::derived_from<AstNode> AstType>
        static auto MatchAll(ArrayView<const AstType*> nodes, ArrayView<AstMatcher> matchers) -> AstMatchResult
        {
            GLSLD_REQUIRE(nodes.size() == matchers.size());

            for (size_t i = 0; i < nodes.size(); ++i) {
                if (AstMatchResult result = matchers[i].Match(*nodes[i]); !result.IsSuccess()) {
                    return result;
                }
            }

            return AstMatchResult::Success();
        }
    };

    struct DeclaratorMatcher
    {
        AstMatcher matchArraySpec;
        AstMatcher matchInitializer;
    };

    inline auto AnyAst() -> AstMatcher
    {
        return AstMatcher("AnyAst", [](const AstNode&) -> AstMatchResult { return AstMatchResult::Success(); });
    }
    inline auto AnyInitializer() -> AstMatcher
    {
        return AstMatcher("AnyInitializer", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstInitializer>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    inline auto AnyExpr() -> AstMatcher
    {
        return AstMatcher("AnyExpr", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstExpr>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    inline auto AnyStmt() -> AstMatcher
    {
        return AstMatcher("AnyStmt", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstStmt>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    inline auto AnyDecl() -> AstMatcher
    {
        return AstMatcher("AnyDecl", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstDecl>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    inline auto AnyQualType() -> AstMatcher
    {
        return AstMatcher("AnyQualType", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstQualType>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }

#pragma region Type

    inline auto BuiltinType(GlslBuiltinType type) -> AstMatcher
    {
        return AstMatcher("BuiltinType", [type](const AstNode& node) -> AstMatchResult {
            auto qualType = node.As<AstQualType>();
            if (!qualType || qualType->GetResolvedType() != Type::GetBuiltinType(type)) {
                return AstMatchResult::Failure(node);
            }

            return AstMatchResult::Success();
        });
    }

    inline auto NamedType(TokenMatcher nameMatcher) -> AstMatcher
    {
        return AstMatcher("NamedType", [nameMatcher = std::move(nameMatcher)](const AstNode& node) -> AstMatchResult {
            auto qualType = node.As<AstQualType>();
            if (!qualType || !nameMatcher.Match(qualType->GetTypeNameTok())) {
                return AstMatchResult::Failure(node);
            }

            return AstMatchResult::Success();
        });
    }

    inline auto NamedType(StringView name) -> AstMatcher
    {
        return NamedType(IdTok(name));
    }

#pragma endregion

#pragma region Misc

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto ArraySpec(MatcherType... indexMatchers) -> AstMatcher
    {
        return AstMatcher("ArraySpec",
                          [indexMatchers = AstMatcher::CollectToVec(std::move(indexMatchers)...)](
                              const AstNode& node) -> AstMatchResult {
                              auto arraySpec = node.As<AstArraySpec>();
                              if (!arraySpec || arraySpec->GetSizeList().size() != indexMatchers.size()) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll(arraySpec->GetSizeList(), indexMatchers);
                          });
    }

#pragma endregion

#pragma region Initializer

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto InitializerList(MatcherType... itemMatchers) -> AstMatcher
    {
        return AstMatcher("InitializerList",
                          [itemMatchers = AstMatcher::CollectToVec(std::move(itemMatchers)...)](
                              const AstNode& node) -> AstMatchResult {
                              auto initList = node.As<AstInitializerList>();
                              if (!initList || initList->GetItems().size() != itemMatchers.size()) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll(initList->GetItems(), itemMatchers);
                          });
    }

    inline auto ErrorExpr() -> AstMatcher
    {
        return AstMatcher("ErrorExpr", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstErrorExpr>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }

    // Scalar literal
    template <typename T>
    inline auto LiteralExpr(T value) -> AstMatcher
    {
        return AstMatcher("LiteralExpr",
                          [value = ConstValue::CreateScalar(value)](const AstNode& node) -> AstMatchResult {
                              auto expr = node.As<AstLiteralExpr>();
                              if (!expr || expr->GetValue() != value) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatchResult::Success();
                          });
    }

    inline auto NameAccessExpr(StringView name) -> AstMatcher
    {
        return AstMatcher("NameAccessExpr", [name = name.Str()](const AstNode& node) -> AstMatchResult {
            auto expr = node.As<AstNameAccessExpr>();
            if (!expr || expr->GetAccessName().text != name) {
                return AstMatchResult::Failure(node);
            }

            return AstMatchResult::Success();
        });
    }

    inline auto FieldAccessExpr(AstMatcher lhsMatcher, StringView name) -> AstMatcher
    {
        return AstMatcher("FieldAccessExpr",
                          [lhsMatcher = std::move(lhsMatcher), name = name](const AstNode& node) -> AstMatchResult {
                              auto expr = node.As<AstFieldAccessExpr>();
                              if (!expr || expr->GetAccessName().text != name) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll({{expr->GetLhsExpr(), &lhsMatcher}});
                          });
    }

    inline auto SwizzleAccessExpr(AstMatcher lhsMatcher, StringView swizzle) -> AstMatcher
    {
        return AstMatcher(
            "SwizzleAccessExpr",
            [lhsMatcher = std::move(lhsMatcher), swizzle = swizzle.Str()](const AstNode& node) -> AstMatchResult {
                auto expr = node.As<AstSwizzleAccessExpr>();
                if (!expr || expr->GetSwizzleDesc().ToString() != swizzle) {
                    return AstMatchResult::Failure(node);
                }

                return AstMatcher::MatchAll({{expr->GetLhsExpr(), &lhsMatcher}});
            });
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto IndexAccessExpr(AstMatcher lhsMatcher, MatcherType... indexMatchers) -> AstMatcher
    {
        return AstMatcher(
            "IndexAccessExpr",
            [lhsMatcher = std::move(lhsMatcher), indexMatchers = AstMatcher::CollectToVec(std::move(indexMatchers)...)](
                const AstNode& node) -> AstMatchResult {
                auto expr = node.As<AstIndexAccessExpr>();
                if (!expr || expr->GetIndices()->GetSizeList().size() != indexMatchers.size()) {
                    return AstMatchResult::Failure(node);
                }
                if (auto lhsResult = lhsMatcher.Match(*expr->GetBaseExpr()); !lhsResult.IsSuccess()) {
                    return lhsResult;
                }

                return AstMatcher::MatchAll(expr->GetIndices()->GetSizeList(), indexMatchers);
            });
    }

    inline auto UnaryExpr(UnaryOp op, AstMatcher operandMatcher) -> AstMatcher
    {
        return AstMatcher("UnaryExpr",
                          [op, operandMatcher = std::move(operandMatcher)](const AstNode& node) -> AstMatchResult {
                              auto expr = node.As<AstUnaryExpr>();
                              if (!expr || expr->GetOpcode() != op) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll({{expr->GetOperand(), &operandMatcher}});
                          });
    }

    inline auto BinaryExpr(BinaryOp op, AstMatcher lhsMatcher, AstMatcher rhsMatcher) -> AstMatcher
    {
        return AstMatcher("BinaryExpr",
                          [op, lhsMatcher = std::move(lhsMatcher),
                           rhsMatcher = std::move(rhsMatcher)](const AstNode& node) -> AstMatchResult {
                              auto expr = node.As<AstBinaryExpr>();
                              if (!expr || expr->GetOpcode() != op) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll(
                                  {{expr->GetLhsOperand(), &lhsMatcher}, {expr->GetRhsOperand(), &rhsMatcher}});
                          });
    }

    inline auto SelectExpr(AstMatcher condMatcher, AstMatcher trueExprMatcher, AstMatcher falseExprMatcher)
        -> AstMatcher
    {
        return AstMatcher("SelectExpr",
                          [condMatcher = std::move(condMatcher), trueExprMatcher = std::move(trueExprMatcher),
                           falseExprMatcher = std::move(falseExprMatcher)](const AstNode& node) -> AstMatchResult {
                              auto expr = node.As<AstSelectExpr>();
                              if (!expr) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll({{expr->GetCondition(), &condMatcher},
                                                           {expr->GetTrueExpr(), &trueExprMatcher},
                                                           {expr->GetFalseExpr(), &falseExprMatcher}});
                          });
    }

    inline auto ImplicitCastExpr(AstMatcher matchOperand) -> AstMatcher
    {
        return AstMatcher("ImplicitCastExpr",
                          [matchOperand = std::move(matchOperand)](const AstNode& node) -> AstMatchResult {
                              auto expr = node.As<AstImplicitCastExpr>();
                              if (!expr) {
                                  return AstMatchResult::Failure(node);
                              }
                              return matchOperand.Match(*expr->GetOperand());
                          });
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto FunctionCallExpr(TokenMatcher nameMatcher, MatcherType... argMatchers) -> AstMatcher
    {
        return AstMatcher(
            "FunctionCallExpr",
            [nameMatcher = std::move(nameMatcher),
             argMatchers = AstMatcher::CollectToVec(std::move(argMatchers)...)](const AstNode& node) -> AstMatchResult {
                auto expr = node.As<AstFunctionCallExpr>();
                if (!expr || !nameMatcher.Match(expr->GetFunctionName()) ||
                    expr->GetArgs().size() != argMatchers.size()) {
                    return AstMatchResult::Failure(node);
                }

                return AstMatcher::MatchAll(expr->GetArgs(), argMatchers);
            });
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto FunctionCallExpr(StringView name, MatcherType... argMatchers) -> AstMatcher
    {
        return FunctionCallExpr(IdTok(name), std::move(argMatchers)...);
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto ConstructorCallExpr(AstMatcher typeMatcher, MatcherType... argMatchers) -> AstMatcher
    {
        return AstMatcher(
            "ConstructorCallExpr",
            [typeMatcher = std::move(typeMatcher),
             argMatchers = AstMatcher::CollectToVec(std::move(argMatchers)...)](const AstNode& node) -> AstMatchResult {
                auto expr = node.As<AstConstructorCallExpr>();
                if (!expr || expr->GetArgs().size() != argMatchers.size()) {
                    return AstMatchResult::Failure(node);
                }
                if (auto typeResult = typeMatcher.Match(*expr->GetConstructedType()); !typeResult.IsSuccess()) {
                    return typeResult;
                }

                return AstMatcher::MatchAll(expr->GetArgs(), argMatchers);
            });
    }

#pragma endregion

#pragma region Stmt

    inline auto ErrorStmt() -> AstMatcher
    {
        return AstMatcher("ErrorStmt", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstErrorStmt>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }

    inline auto EmptyStmt() -> AstMatcher
    {
        return AstMatcher("EmptyStmt", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstEmptyStmt>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto CompoundStmt(MatcherType... stmtMatchers) -> AstMatcher
    {
        return AstMatcher("CompoundStmt",
                          [stmtMatchers = AstMatcher::CollectToVec(std::move(stmtMatchers)...)](
                              const AstNode& node) -> AstMatchResult {
                              auto stmt = node.As<AstCompoundStmt>();
                              if (!stmt || stmt->GetChildren().size() != stmtMatchers.size()) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll(stmt->GetChildren(), stmtMatchers);
                          });
    }

    inline auto ExprStmt(AstMatcher exprMatcher) -> AstMatcher
    {
        return AstMatcher("ExprStmt", [exprMatcher = std::move(exprMatcher)](const AstNode& node) -> AstMatchResult {
            auto stmt = node.As<AstExprStmt>();
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }
            return exprMatcher.Match(*stmt->GetExpr());
        });
    }

    inline auto DeclStmt(AstMatcher declMatcher) -> AstMatcher
    {
        return AstMatcher("DeclStmt", [declMatcher = std::move(declMatcher)](const AstNode& node) -> AstMatchResult {
            auto stmt = node.As<AstDeclStmt>();
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }
            return declMatcher.Match(*stmt->GetDecl());
        });
    }

    inline auto IfStmt(AstMatcher condMatcher, AstMatcher thenStmtMatcher) -> AstMatcher
    {
        return AstMatcher("IfStmt",
                          [condMatcher     = std::move(condMatcher),
                           thenStmtMatcher = std::move(thenStmtMatcher)](const AstNode& node) -> AstMatchResult {
                              auto stmt = node.As<AstIfStmt>();
                              if (!stmt || stmt->GetElseStmt() != nullptr) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll(
                                  {{stmt->GetConditionExpr(), &condMatcher}, {stmt->GetThenStmt(), &thenStmtMatcher}});
                          });
    }

    inline auto IfStmt(AstMatcher condMatcher, AstMatcher thenStmtMatcher, AstMatcher elseStmtMatcher) -> AstMatcher
    {
        return AstMatcher("IfStmt",
                          [condMatcher = std::move(condMatcher), thenStmtMatcher = std::move(thenStmtMatcher),
                           elseStmtMatcher = std::move(elseStmtMatcher)](const AstNode& node) -> AstMatchResult {
                              auto stmt = node.As<AstIfStmt>();
                              if (!stmt || stmt->GetElseStmt() == nullptr) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll({{stmt->GetConditionExpr(), &condMatcher},
                                                           {stmt->GetThenStmt(), &thenStmtMatcher},
                                                           {stmt->GetElseStmt(), &elseStmtMatcher}});
                          });
    }

    inline auto ForStmt(AstMatcher initExprMatcher, AstMatcher condExprMatcher, AstMatcher iterExprMatcher,
                        AstMatcher bodyMatcher) -> AstMatcher
    {
        return AstMatcher("ForStmt",
                          [initExprMatcher = std::move(initExprMatcher), conditionMatcher = std::move(condExprMatcher),
                           iterExprMatcher = std::move(iterExprMatcher),
                           bodyMatcher     = std::move(bodyMatcher)](const AstNode& node) -> AstMatchResult {
                              auto stmt = node.As<AstForStmt>();
                              if (!stmt) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll({{stmt->GetInitStmt(), &initExprMatcher},
                                                           {stmt->GetConditionExpr(), &conditionMatcher},
                                                           {stmt->GetIterExpr(), &iterExprMatcher},
                                                           {stmt->GetBody(), &bodyMatcher}});
                          });
    }

    inline auto WhileStmt(AstMatcher condMatcher, AstMatcher bodyMatcher) -> AstMatcher
    {
        return AstMatcher("WhileStmt",
                          [condMatcher = std::move(condMatcher),
                           bodyMatcher = std::move(bodyMatcher)](const AstNode& node) -> AstMatchResult {
                              auto stmt = node.As<AstWhileStmt>();
                              if (!stmt) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll(
                                  {{stmt->GetConditionExpr(), &condMatcher}, {stmt->GetBody(), &bodyMatcher}});
                          });
    }

    inline auto DoWhileStmt(AstMatcher bodyMatcher, AstMatcher condMatcher) -> AstMatcher
    {
        return AstMatcher("DoWhileStmt",
                          [bodyMatcher = std::move(bodyMatcher),
                           condMatcher = std::move(condMatcher)](const AstNode& node) -> AstMatchResult {
                              auto stmt = node.As<AstDoWhileStmt>();
                              if (!stmt) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll(
                                  {{stmt->GetBody(), &bodyMatcher}, {stmt->GetConditionExpr(), &condMatcher}});
                          });
    }

    inline auto CaseLabelStmt(AstMatcher exprMatcher) -> AstMatcher
    {
        return AstMatcher("CaseLabelStmt",
                          [exprMatcher = std::move(exprMatcher)](const AstNode& node) -> AstMatchResult {
                              auto stmt = node.As<AstLabelStmt>();
                              if (!stmt || stmt->GetCaseExpr() == nullptr) {
                                  return AstMatchResult::Failure(node);
                              }

                              return exprMatcher.Match(*stmt->GetCaseExpr());
                          });
    }

    inline auto DefaultLabelStmt() -> AstMatcher
    {
        return AstMatcher("DefaultLabelStmt", [](const AstNode& node) -> AstMatchResult {
            auto stmt = node.As<AstLabelStmt>();
            return stmt && stmt->GetCaseExpr() == nullptr ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }

    inline auto SwitchStmt(AstMatcher testExprMatcher, AstMatcher bodyMatcher) -> AstMatcher
    {
        return AstMatcher("SwitchStmt",
                          [testExprMatcher = std::move(testExprMatcher),
                           bodyMatcher     = std::move(bodyMatcher)](const AstNode& node) -> AstMatchResult {
                              auto stmt = node.As<AstSwitchStmt>();
                              if (!stmt) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll(
                                  {{stmt->GetTestExpr(), &testExprMatcher}, {stmt->GetBody(), &bodyMatcher}});
                          });
    }

    inline auto BreakStmt() -> AstMatcher
    {
        return AstMatcher("BreakStmt", [](const AstNode& node) -> AstMatchResult {
            auto stmt = node.As<AstJumpStmt>();
            return stmt && stmt->GetJumpType() == JumpType::Break ? AstMatchResult::Success()
                                                                  : AstMatchResult::Failure(node);
        });
    }

    inline auto ContinueStmt() -> AstMatcher
    {
        return AstMatcher("ContinueStmt", [](const AstNode& node) -> AstMatchResult {
            auto stmt = node.As<AstJumpStmt>();
            return stmt && stmt->GetJumpType() == JumpType::Continue ? AstMatchResult::Success()
                                                                     : AstMatchResult::Failure(node);
        });
    }

    inline auto ReturnStmt() -> AstMatcher
    {
        return AstMatcher("ReturnStmt", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstReturnStmt>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }

    inline auto ReturnStmt(AstMatcher exprMatcher) -> AstMatcher
    {
        return AstMatcher("ReturnStmt", [exprMatcher = std::move(exprMatcher)](const AstNode& node) -> AstMatchResult {
            auto stmt = node.As<AstReturnStmt>();
            if (!stmt || stmt->GetExpr() == nullptr) {
                return AstMatchResult::Failure(node);
            }
            return exprMatcher.Match(*stmt->GetExpr());
        });
    }

#pragma endregion

    inline auto EmptyDecl() -> AstMatcher
    {
        return AstMatcher("EmptyDecl", [](const AstNode& node) -> AstMatchResult {
            return node.Is<AstEmptyDecl>() ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }

    inline auto VariableDecl1(AstMatcher qualTypeMatcher, TokenMatcher nameMatcher, AstMatcher initMatcher)
        -> AstMatcher
    {
        return AstMatcher("VariableDecl1",
                          [qualTypeMatcher = std::move(qualTypeMatcher), nameMatcher = std::move(nameMatcher),
                           initMatcher = std::move(initMatcher)](const AstNode& node) -> AstMatchResult {
                              auto decl = node.As<AstVariableDecl>();
                              if (!decl || decl->GetDeclarators().size() != 1) {
                                  return AstMatchResult::Failure(node);
                              }
                              if (auto nameMatchResult = nameMatcher.Match(decl->GetDeclarators()[0].declTok);
                                  !nameMatchResult) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll({
                                  {decl->GetQualType(), &qualTypeMatcher},
                                  {decl->GetDeclarators()[0].initializer, &initMatcher},
                              });
                          });
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto FunctionDecl(AstMatcher returnTypeMatcher, TokenMatcher nameMatcher, AstMatcher bodyMatcher,
                             MatcherType... paramMatchers) -> AstMatcher
    {
        return AstMatcher("FunctionDecl",
                          [returnTypeMatcher = std::move(returnTypeMatcher), nameMatcher = std::move(nameMatcher),
                           bodyMatcher   = std::move(bodyMatcher),
                           paramMatchers = AstMatcher::CollectToVec(std::move(paramMatchers)...)](
                              const AstNode& node) -> AstMatchResult {
                              auto decl = node.As<AstFunctionDecl>();
                              if (!decl || !nameMatcher.Match(decl->GetDeclTok()) ||
                                  decl->GetParams().size() != paramMatchers.size()) {
                                  return AstMatchResult::Failure(node);
                              }
                              if (auto paramResult = AstMatcher::MatchAll(decl->GetParams(), paramMatchers);
                                  !paramResult.IsSuccess()) {
                                  return paramResult;
                              }

                              return AstMatcher::MatchAll({
                                  {decl->GetReturnType(), &returnTypeMatcher},
                                  {decl->GetBody(), &bodyMatcher},
                              });
                          });
    }

    template <std::same_as<AstMatcher>... MatcherType>
    inline auto TranslationUnit(MatcherType... declMatchers) -> AstMatcher
    {
        return AstMatcher("TranslationUnit",
                          [declMatchers = AstMatcher::CollectToVec(std::move(declMatchers)...)](
                              const AstNode& node) -> AstMatchResult {
                              auto tu = node.As<AstTranslationUnit>();
                              if (!tu || tu->GetGlobalDecls().size() != declMatchers.size()) {
                                  return AstMatchResult::Failure(node);
                              }

                              return AstMatcher::MatchAll(tu->GetGlobalDecls(), declMatchers);
                          });
    }

    inline auto FindMatch(AstMatcher finder, AstMatcher matcher) -> AstMatcher
    {
        return AstMatcher("FindMatch", std::move(finder), std::move(matcher));
    }

} // namespace glsld
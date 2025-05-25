#pragma once
#include "TokenMatcher.h"

#include "Ast/AstVisitor.h"
#include "Ast/Base.h"
#include "Ast/Expr.h"
#include "Ast/Eval.h"
#include "Language/ConstValue.h"

#include <functional>

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

        static auto Failure(const AstNode* failedNode) -> AstMatchResult
        {
            return AstMatchResult{failedNode};
        }

        static auto Failure(const AstNode* failedNode, std::string message) -> AstMatchResult
        {
            auto result = AstMatchResult{failedNode};
            result.AddErrorTrace(std::move(message));
            return result;
        }
    };

    class AstMatcher
    {
    private:
        // Description of this matcher. Used for composing error message.
        std::string desc;
        bool findMatchMode = false;
        // Callback to match the current node.
        std::move_only_function<auto(const AstNode*)->AstMatchResult> matchCallback;
        // Additional checkers for the deduced type of AST expression.
        std::move_only_function<auto(const Type&)->bool> typeChecker;
        // Additional checkers for the evaluated value of AST expression.
        std::move_only_function<auto(const ConstValue&)->bool> valueChecker;

        // Matchers for children nodes.
        std::vector<AstMatcher*> childrenMatcher;

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
        AstMatcher(std::string desc, std::move_only_function<auto(const AstNode*)->AstMatchResult> matchCallback)
            : desc(std::move(desc)), matchCallback(std::move(matchCallback))
        {
        }

        AstMatcher(std::string desc, AstMatcher* finder, AstMatcher* matcher)
            : desc(std::move(desc)), findMatchMode(true), childrenMatcher({finder, matcher})
        {
        }

        AstMatcher(const AstMatcher&)                    = delete;
        AstMatcher(AstMatcher&&)                         = default;
        auto operator=(const AstMatcher&) -> AstMatcher& = delete;
        auto operator=(AstMatcher&&) -> AstMatcher&      = default;

        auto CheckType(std::move_only_function<auto(const Type&)->bool> typeChecker) -> AstMatcher*
        {
            typeChecker = std::move(typeChecker);
            return this;
        }

        auto CheckValue(std::move_only_function<auto(const ConstValue&)->bool> valueChecker) -> AstMatcher*
        {
            valueChecker = std::move(valueChecker);
            return this;
        }

        auto Match(const AstNode* node) -> AstMatchResult
        {
            if (findMatchMode) {
                AstChildCollector collector;
                if (node) {
                    collector.Traverse(*node);
                }

                auto& finder  = childrenMatcher[0];
                auto& matcher = childrenMatcher[1];

                for (const auto* child : collector.children) {
                    auto found = finder->Match(child);
                    if (found.IsSuccess()) {
                        return matcher->Match(child);
                    }
                }

                return AstMatchResult::Failure(node);
            }
            else {
                if (matchCallback) {
                    auto matchResult = matchCallback(node);
                    if (!matchResult.IsSuccess()) {
                        matchResult.AddErrorTrace(desc);
                        return matchResult;
                    }
                }

                if (auto expr = node ? node->As<AstExpr>() : nullptr; expr) {
                    if (typeChecker && !typeChecker(*expr->GetDeducedType())) {
                        return AstMatchResult::Failure(node);
                    }
                    if (valueChecker && !valueChecker(EvalAstExpr(*expr))) {
                        return AstMatchResult::Failure(node);
                    }
                }

                return AstMatchResult::Success();
            }
        }
    };

    struct DeclaratorMatcher
    {
        TokenMatcher* nameMatcher      = nullptr;
        AstMatcher* arraySpecMatcher   = nullptr;
        AstMatcher* initializerMatcher = nullptr;

        auto Match(const AstDecl* decl, Declarator declarator) const -> AstMatchResult
        {
            if (!nameMatcher->Match(declarator.nameToken)) {
                return AstMatchResult::Failure(decl);
            }

            if (auto result = arraySpecMatcher->Match(declarator.arraySpec); !result.IsSuccess()) {
                return result;
            }

            if (auto result = initializerMatcher->Match(declarator.initializer); !result.IsSuccess()) {
                return result;
            }

            return AstMatchResult::Success();
        }
    };
} // namespace glsld
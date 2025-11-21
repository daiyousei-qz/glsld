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
            return errorTraces.empty();
        }

        auto GetFailedNode() const -> const AstNode*
        {
            return failedNode;
        }

        auto GetErrorTrace() const -> std::string
        {
            std::string result;
            for (const auto& trace : errorTraces) {
                result += "  - ";
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

        template <typename... Args>
        static auto Failure(const AstNode* failedNode, fmt::format_string<Args...> fmt, Args&&... args)
            -> AstMatchResult
        {
            auto result = AstMatchResult{failedNode};
            result.AddErrorTrace(fmt::format(std::move(fmt), std::forward<Args>(args)...));
            return result;
        }
        static auto FailWithUnknown(const AstNode* failedNode) -> AstMatchResult
        {
            return Failure(failedNode, "Unknown failure");
        }

        static auto FailWithUnexpectedAstNode(const AstNode* failedNode, StringView expectedAstTypeName)
            -> AstMatchResult
        {
            return Failure(failedNode, "Unexpected AST node, expecting {}, got {}", expectedAstTypeName,
                           failedNode ? EnumToString(failedNode->GetTag()) : "null");
        }
    };

    class AstMatcher
    {
    private:
        // Name of this matcher. Used for composing error message.
        std::string name;
        bool findMatchMode = false;
        // Callback to match the current node.
        std::move_only_function<auto(const AstNode*)->AstMatchResult> matchCallback;
        // Additional checkers for the deduced type of AST expression.
        std::move_only_function<auto(const Type&)->bool> typeChecker;
        // Additional checkers for the evaluated value of AST expression.
        std::move_only_function<auto(const ConstValue&)->bool> valueChecker;

        // Matchers for children nodes.
        std::vector<AstMatcher*> childrenMatcher;

        struct AstChildCollector : public AstVisitor<AstChildCollector, std::vector<const AstNode*>,
                                                     AstVisitorConfig{.traceTraversalPath = true}>
        {
            std::vector<const AstNode*> children;

            auto Finish() -> std::vector<const AstNode*> GLSLD_AST_VISITOR_OVERRIDE
            {
                return std::move(children);
            }

            auto EnterAstNode(const AstNode& node) -> AstVisitPolicy GLSLD_AST_VISITOR_OVERRIDE
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
        AstMatcher(std::string name, std::move_only_function<auto(const AstNode*)->AstMatchResult> matchCallback)
            : name(std::move(name)), matchCallback(std::move(matchCallback))
        {
        }

        AstMatcher(std::string name, AstMatcher* finder, AstMatcher* matcher)
            : name(std::move(name)), findMatchMode(true), childrenMatcher({finder, matcher})
        {
        }

        AstMatcher(const AstMatcher&)                    = delete;
        AstMatcher(AstMatcher&&)                         = default;
        auto operator=(const AstMatcher&) -> AstMatcher& = delete;
        auto operator=(AstMatcher&&) -> AstMatcher&      = default;

        auto CheckType(std::move_only_function<auto(const Type&)->bool> callback) -> AstMatcher*
        {
            typeChecker = std::move(callback);
            return this;
        }

        auto CheckType(GlslBuiltinType type) -> AstMatcher*
        {
            return CheckType([type](const Type& t) { return t.IsSameWith(type); });
        }

        auto CheckValue(std::move_only_function<auto(const ConstValue&)->bool> callback) -> AstMatcher*
        {
            valueChecker = std::move(callback);
            return this;
        }

        auto Match(const AstNode* node) -> AstMatchResult
        {
            if (findMatchMode) {
                std::vector<const AstNode*> children;
                if (node) {
                    children = TraverseAst(AstChildCollector{}, *node);
                }

                auto& finder  = childrenMatcher[0];
                auto& matcher = childrenMatcher[1];

                for (const auto* child : children) {
                    auto found = finder->Match(child);
                    if (found.IsSuccess()) {
                        return matcher->Match(child);
                    }
                }

                return AstMatchResult::FailWithUnknown(node);
            }
            else {
                if (matchCallback) {
                    auto matchResult = matchCallback(node);
                    if (!matchResult.IsSuccess()) {
                        matchResult.AddErrorTrace("See matcher: " + name);
                        return matchResult;
                    }
                }

                if (auto expr = node ? node->As<AstExpr>() : nullptr; expr) {
                    if (typeChecker && !typeChecker(*expr->GetDeducedType())) {
                        return AstMatchResult::Failure(node, "Type checker failed for {}. Deduced type is '{}'", name,
                                                       expr->GetDeducedType()->GetDebugName());
                    }
                    if (valueChecker && !valueChecker(EvalAstExpr(*expr))) {
                        return AstMatchResult::Failure(node, "Value checker failed for {}", name);
                    }
                }
                else {
                    if (typeChecker) {
                        return AstMatchResult::Failure(node, "Type checker is not applicable to non-expression");
                    }
                    if (valueChecker) {
                        return AstMatchResult::Failure(node, "Value checker is not applicable to non-expression");
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
                return AstMatchResult::FailWithUnknown(decl);
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
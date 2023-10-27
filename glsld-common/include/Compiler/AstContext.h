#pragma once
#include "Basic/Common.h"
#include "Ast/Base.h"
#include "Ast/Dispatch.h"
#include "Compiler/SyntaxToken.h"

#include <vector>
#include <type_traits>

namespace glsld
{
    // This class holds all AST nodes in a translation unit
    class AstContext
    {
    private:
        // FIXME: optimize memory layout
        std::vector<AstNode*> nodes;

        const AstTranslationUnit* translationUnit;

    public:
        AstContext() = default;

        ~AstContext()
        {
            for (auto node : nodes) {
                InvokeAstDispatched(*node, [](auto& dispatchedNode) { delete &dispatchedNode; });
            }
        }

        AstContext(const AstContext&)                    = delete;
        AstContext(AstContext&&)                         = delete;
        auto operator=(const AstContext&) -> AstContext& = delete;
        auto operator=(AstContext&&) -> AstContext&      = delete;

        auto SetTranslationUnit(const AstTranslationUnit* tu)
        {
            GLSLD_ASSERT(!translationUnit && "Translation unit is already set.");
            translationUnit = tu;
        }
        auto GetTranslationUnit() const noexcept
        {
            return translationUnit;
        }

        template <typename T, typename... Args>
            requires AstNodeTrait<T>::isLeafNode
        auto CreateAstNode(AstSyntaxRange range, Args&&... args) -> T*
        {
            auto result = new T(std::forward<Args>(args)...);
            result->Initialize(AstNodeTrait<T>::tag, range);
            nodes.push_back(result);
            return result;
        }
    };
} // namespace glsld
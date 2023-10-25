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

        auto AddGlobalDecl(AstDecl* decl) -> void
        {
            globalDecls.push_back(decl);
            if (auto p = decl->As<AstFunctionDecl>()) {
                functionDecls.push_back(p);
            }
            if (auto p = decl->As<AstVariableDecl>()) {
                variableDecls.push_back(p);
            }
        }

        auto GetGlobalDecls() const -> ArrayView<AstDecl*>
        {
            return globalDecls;
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

        std::vector<AstDecl*> globalDecls;
        std::vector<AstVariableDecl*> variableDecls;
        std::vector<AstFunctionDecl*> functionDecls;

        // FIXME: optimize memory layout
        std::vector<AstNode*> nodes;
    };
} // namespace glsld
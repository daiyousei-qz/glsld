#pragma once
#include "Common.h"
#include "Ast.h"
#include "Typing.h"
#include <type_traits>

namespace glsld
{
    // This class holds all AST nodes in a translation unit
    class AstContext
    {
    public:
        friend class Parser;

        ~AstContext()
        {
            for (auto node : nodes) {
                node->DispatchInvoke([](auto& dispatchedNode) { delete &dispatchedNode; });
            }
        }

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

        template <typename T, typename... Args>
            requires AstNodeTrait<T>::isLeafNode
        auto CreateAstNode(SyntaxRange range, Args&&... args) -> T*
        {
            auto result = new T(std::forward<Args>(args)...);
            result->Initialize(AstNodeTrait<T>::tag, range);
            return result;
        }

        auto CreateStructType(AstStructDecl* decl) -> const TypeDesc*
        {
            return GetErrorTypeDesc();
        }

        auto CreateStructType(AstInterfaceBlockDecl* decl) -> const TypeDesc*
        {
            return GetErrorTypeDesc();
        }

        auto GetFunctionType(ArrayView<const TypeDesc*> params) -> const TypeDesc*
        {
            return GetErrorTypeDesc();
        }

        // std::unordered_map<std::vector<const TypeDesc*>, const TypeDesc*>

        std::vector<AstDecl*> globalDecls;
        std::vector<AstVariableDecl*> variableDecls;
        std::vector<AstFunctionDecl*> functionDecls;

        // FIXME: optimize memory layout
        std::vector<AstNodeBase*> nodes;
    };
} // namespace glsld
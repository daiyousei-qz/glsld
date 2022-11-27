#pragma once
#include "Ast.h"
#include <type_traits>

namespace glsld
{
    // This class holds all AST nodes in a translation unit
    class AstContext
    {
    public:
        friend class Parser;

        auto AddGlobalDecl(AstDecl* decl) -> void
        {
            globalDecls.push_back(decl);
            if (auto p = dynamic_cast<AstFunctionDecl*>(decl)) {
                functionDecls.push_back(p);
            }
            if (auto p = dynamic_cast<AstVariableDecl*>(decl)) {
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

        std::vector<AstDecl*> globalDecls;
        std::vector<AstVariableDecl*> variableDecls;
        std::vector<AstFunctionDecl*> functionDecls;
    };
} // namespace glsld
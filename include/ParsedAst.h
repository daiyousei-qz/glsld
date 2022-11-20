#pragma once

#include "SyntaxTree.h"

#include <vector>

namespace glsld
{
    class ParsedAst
    {
    public:
        auto AddGlobalVariable(AstVariableDecl* decl) -> void
        {
            globalDecls.push_back(decl);
            variableDecls.push_back(decl);
        }

        auto AddFunction(AstFunctionDecl* decl) -> void
        {
            globalDecls.push_back(decl);
            functionDecls.push_back(decl);
        }

    private:
        std::vector<AstDecl*> globalDecls;
        std::vector<AstVariableDecl*> variableDecls;
        std::vector<AstFunctionDecl*> functionDecls;
    };

} // namespace glsld
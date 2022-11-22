#pragma once

#include "Ast.h"

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

        // FIXME: for debug
        auto GetVarSymbols() const -> std::vector<std::string>
        {
            std::vector<std::string> result;
            for (auto var : variableDecls) {
                for (auto declarator : var->GetDeclarators()) {
                    result.push_back(std::string{declarator.id->GetIdentifier()});
                }
            }

            return result;
        }

        auto GetFuncSymbols() const -> std::vector<std::string>
        {
            std::vector<std::string> result;
            for (auto f : functionDecls) {
                result.push_back(std::string{f->GetName()->GetIdentifier()});
            }

            return result;
        }

    private:
        std::vector<AstDecl*> globalDecls;
        std::vector<AstVariableDecl*> variableDecls;
        std::vector<AstFunctionDecl*> functionDecls;
    };

} // namespace glsld
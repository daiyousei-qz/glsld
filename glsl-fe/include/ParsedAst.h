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
        }

        auto Add(AstDecl* decl) -> void
        {
            globalDecls.push_back(decl);
            if (auto p = dynamic_cast<AstFunctionDecl*>(decl)) {
                functionDecls.push_back(p);
            }
            if (auto p = dynamic_cast<AstVariableDecl*>(decl)) {
                variableDecls.push_back(p);
            }
        }

        // FIXME: for debug
        auto GetVarSymbols() const -> std::vector<std::string>
        {
            std::vector<std::string> result;
            for (auto var : variableDecls) {
                for (auto declarator : var->GetDeclarators()) {
                    result.push_back(declarator.declTok.text.Str());
                }
            }

            return result;
        }

        auto GetFuncSymbols() const -> std::vector<std::string>
        {
            std::vector<std::string> result;
            for (auto f : functionDecls) {
                result.push_back(f->GetName().text.Str());
            }

            return result;
        }

    private:
        std::vector<AstDecl*> globalDecls;
        std::vector<AstVariableDecl*> variableDecls;
        std::vector<AstFunctionDecl*> functionDecls;
    };

} // namespace glsld
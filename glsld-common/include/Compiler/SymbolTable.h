#pragma once
#include "Basic/Common.h"
#include "Ast/Decl.h"
#include "Language/Typing.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <ranges>

namespace glsld
{
    class SymbolTable
    {
    private:
        const SymbolTable* parent = nullptr;

        struct FunctionSymbolEntry
        {
            const AstFunctionDecl* decl;

            std::vector<const Type*> paramTypes;
        };

        std::unordered_multimap<std::string, FunctionSymbolEntry> funcDeclLookup;
        std::unordered_map<std::string, DeclView> declLookup;

    public:
        SymbolTable()
        {
        }
        SymbolTable(const SymbolTable* parent) : parent(parent)
        {
        }

        // Add a function declaration to the symbol table
        // This requires the SymbolTable to be in the global scope since glsl does not support nested functions
        auto AddFunctionDecl(AstFunctionDecl& decl) -> void;

        // Add a struct declaration to the symbol table
        auto AddStructDecl(AstStructDecl& decl) -> void;

        // Add an interface block declaration to the symbol table, aka. UBO, SSBO, etc.
        auto AddInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void;

        // Add a variable declaration to the symbol table
        auto AddVariableDecl(AstVariableDecl& decl) -> void;

        // Add a parameter declaration to the symbol table
        auto AddParamDecl(AstParamDecl& decl) -> void;

        // Find a function declaration by name and argument types
        auto FindFunction(const std::string& name, const std::vector<const Type*>& argTypes) const
            -> const AstFunctionDecl*;

        // Find a declaration by name
        auto FindSymbol(const std::string& name) const -> DeclView;

    private:
        auto TryAddSymbol(SyntaxToken declToken, const AstDecl& decl) -> bool;
    };
} // namespace glsld
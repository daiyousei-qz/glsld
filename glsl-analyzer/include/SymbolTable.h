#pragma once
#include "Common.h"
#include "AstDecl.h"
#include "Typing.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <ranges>

namespace glsld
{
    class SymbolTable
    {
    public:
        SymbolTable()
        {
        }
        SymbolTable(const SymbolTable* parent) : parent(parent)
        {
        }

        // Add a function declaration to the symbol table
        // This requires the SymbolTable to be in the global scope since glsl does not support nested functions
        auto AddFunction(AstFunctionDecl& decl) -> void;

        // Add a struct declaration to the symbol table
        auto AddStructDecl(AstStructDecl& decl) -> void;

        // Add an interface block declaration to the symbol table, aka. UBO, SSBO, etc.
        auto AddInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void;

        // Add a variable declaration to the symbol table
        auto AddVariableDecl(AstVariableDecl& decl) -> void;

        // Add a parameter declaration to the symbol table
        auto AddParamDecl(AstParamDecl& decl) -> void;

        // Find a function declaration by name and argument types
        auto FindFunction(const std::string& name, const std::vector<const Type*>& argTypes) const -> AstFunctionDecl*;

        // Find a declaration by name
        auto FindSymbol(const std::string& name) const -> DeclView;

    private:
        // FIXME: avoid allocation
        auto TryAddSymbol(SyntaxToken declToken, AstDecl& decl) -> bool
        {
            // FIXME: avoid string allocation
            auto name = declToken.text.Str();
            if (name.empty()) {
                return false;
            }

            auto& entry = declLookup[std::move(name)];
            if (!entry.IsValid()) {
                entry = DeclView{&decl};
                return true;
            }
            else {
                return false;
            }
        }

        const SymbolTable* parent = nullptr;

        std::unordered_multimap<std::string, AstFunctionDecl*> funcDeclLookup;
        std::unordered_map<std::string, DeclView> declLookup;
    };
} // namespace glsld
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
            PushScope();
        }

        auto PushScope() -> void
        {
            levels.push_back(SymbolTableLevel{});
        }

        auto PopScope() -> void
        {
            // global scope cannot be popped
            GLSLD_ASSERT(levels.size() > 1);
            levels.pop_back();
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
            GLSLD_ASSERT(!levels.empty());

            // FIXME: avoid string allocation
            auto name = declToken.text.Str();
            if (name.empty()) {
                return false;
            }

            auto& entry = levels.back().declLookup[std::move(name)];
            if (!entry.IsValid()) {
                entry = DeclView{&decl};
                return true;
            }
            else {
                return false;
            }
        }

        struct FunctionEntry
        {
            std::vector<const Type*> paramTypes;
            AstFunctionDecl* decl;
        };

        struct SymbolTableLevel
        {
            std::unordered_map<std::string, DeclView> declLookup;
        };

        std::unordered_multimap<std::string, FunctionEntry> funcDeclLookup;
        std::vector<SymbolTableLevel> levels;
    };
} // namespace glsld
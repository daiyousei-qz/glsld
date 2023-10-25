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
    struct FunctionSymbolEntry
    {
        const AstFunctionDecl* decl;

        std::vector<const Type*> paramTypes;
    };

    class SymbolTableLevel
    {
    private:
        // Lookup table for function declarations
        std::unordered_multimap<std::string, FunctionSymbolEntry> funcDeclLookup;

        // Lookup table for all other declarations
        std::unordered_map<std::string, DeclView> declLookup;

        DeclScope scope;

        bool freezed = false;

        using FunctionCandidateRange =
            std::pair<std::unordered_multimap<std::string, FunctionSymbolEntry>::const_iterator,
                      std::unordered_multimap<std::string, FunctionSymbolEntry>::const_iterator>;

    public:
        SymbolTableLevel(DeclScope scope) : scope(scope)
        {
        }

        auto GetScope() const noexcept -> DeclScope
        {
            return scope;
        }

        auto IsGlobalScope() const noexcept -> bool
        {
            return scope == DeclScope::Global;
        }

        auto IsFreezed() const noexcept -> bool
        {
            return freezed;
        }

        // Add a function declaration to the symbol table
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
        auto FindFunctionCandidate(const std::string& name) const
        {
            auto [itBegin, itEnd] = funcDeclLookup.equal_range(name);
            return std::ranges::subrange{itBegin, itEnd} |
                   std::views::transform([](const auto& entry) { return &entry.second; });
        }

        // Find a declaration by name
        auto FindSymbol(const std::string& name) const -> DeclView
        {
            if (auto it = declLookup.find(name); it != declLookup.end()) {
                return it->second;
            }
            else {
                return DeclView{};
            }
        }

    private:
        auto TryAddSymbol(SyntaxToken declToken, const AstDecl& decl) -> bool;
    };

    class SymbolTable
    {
    private:
        size_t importedLevelCount = 0;
        std::vector<SymbolTableLevel*> levels;

    public:
        SymbolTable(ArrayView<SymbolTableLevel*> importedLevels) : importedLevelCount(importedLevels.size())
        {
            assert(std::ranges::all_of(importedLevels, [](const SymbolTableLevel* level) {
                return level->IsFreezed() && level->IsGlobalScope();
            }));

            levels.insert(levels.end(), importedLevels.begin(), importedLevels.end());
            levels.push_back(new SymbolTableLevel(DeclScope::Global));
        }
        SymbolTable() : SymbolTable(ArrayView<SymbolTableLevel*>{})
        {
        }
        ~SymbolTable()
        {
            assert(levels.size() == importedLevelCount + 1);
            for (size_t i = importedLevelCount; i < levels.size(); ++i) {
                delete levels[i];
            }
        }

        auto GetCurrentLevel() const -> SymbolTableLevel*
        {
            assert(!levels.empty());
            return levels.back();
        }

        auto GetGlobalLevels() const -> ArrayView<SymbolTableLevel*>
        {
            return {levels.data(), importedLevelCount + 1};
        }

        auto PushLevel(DeclScope scope) -> void
        {
            assert(scope != DeclScope::Global && "Global scope is not allowed to be pushed");
            levels.push_back(new SymbolTableLevel(scope));
        }

        auto PopLevel() -> void
        {
            if (levels.size() > GetGlobalLevels().size()) {
                delete levels.back();
                levels.pop_back();
            }
            else {
                assert(false && "Poping global scope is not allowed");
            }
        }

        // Find a declaration by name
        auto FindSymbol(const std::string& name) const -> DeclView;

        // Find a function declaration by name and argument types
        auto FindFunction(const std::string& name, const std::vector<const Type*>& argTypes) const
            -> const AstFunctionDecl*;
    };
} // namespace glsld
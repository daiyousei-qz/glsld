#pragma once
#include "Basic/Common.h"
#include "Support/StringView.h"
#include "Support/StringMap.h"
#include "Ast/Decl.h"

#include <algorithm>
#include <vector>
#include <ranges>

namespace glsld
{
    struct FunctionParamSymbolEntry
    {
        const Type* type;
        bool isInput;
        bool isOutput;
    };

    struct FunctionSymbolEntry
    {
        const AstFunctionDecl* decl;

        std::vector<FunctionParamSymbolEntry> paramEntries;
    };

    class SymbolTableLevel
    {
    private:
        // Lookup table for function declarations
        UnorderedStringMultiMap<FunctionSymbolEntry> funcDeclLookup;

        // Lookup table for all other declarations
        UnorderedStringMap<const AstDecl*> declLookup;

        DeclScope scope;

        bool freezed = false;

        using FunctionCandidateRange = std::pair<UnorderedStringMultiMap<FunctionSymbolEntry>::ConstIteratorType,
                                                 UnorderedStringMultiMap<FunctionSymbolEntry>::ConstIteratorType>;

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
        auto IsFunctionScope() const noexcept -> bool
        {
            return scope == DeclScope::Function;
        }
        auto IsBlockScope() const noexcept -> bool
        {
            return scope == DeclScope::Block;
        }
        auto IsLocalScope() const noexcept -> bool
        {
            return scope == DeclScope::Function || scope == DeclScope::Block;
        }
        auto IsStructScope() const noexcept -> bool
        {
            return scope == DeclScope::Struct;
        }

        auto IsFreezed() const noexcept -> bool
        {
            return freezed;
        }

        // Add a function declaration to the symbol table
        auto AddFunctionDecl(AstFunctionDecl& decl) -> void;

        // Add an interface block declaration to the symbol table, aka. UBO, SSBO, etc.
        auto AddInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void;

        // Add a variable declaration to the symbol table, including struct declaration
        auto AddVariableDecl(AstVariableDecl& decl) -> void;

        // Add a parameter declaration to the symbol table
        auto AddParamDecl(AstParamDecl& decl) -> void;

        // Find a function declaration by name and argument types
        auto FindFunctionCandidate(StringView name) const
        {
            auto [itBegin, itEnd] = funcDeclLookup.EqualRange(name);
            return std::ranges::subrange{itBegin, itEnd} |
                   std::views::transform([](const auto& entry) { return &entry.second; });
        }

        // Find a declaration by name
        auto FindSymbol(StringView name) const -> const AstDecl*
        {
            if (auto it = declLookup.Find(name); it != declLookup.end()) {
                return it->second;
            }
            else {
                return nullptr;
            }
        }

    private:
        auto TryAddSymbol(AstSyntaxToken nameToken, const AstDecl& decl) -> bool;
    };

    class SymbolTable
    {
    private:
        size_t importedLevelCount = 0;
        std::vector<SymbolTableLevel*> levels;

    public:
        SymbolTable(ArrayView<SymbolTableLevel*> importedLevels) : importedLevelCount(importedLevels.size())
        {
            GLSLD_ASSERT(std::ranges::all_of(importedLevels, [](const SymbolTableLevel* level) {
                // FIXME: should be freezed
                // return level->IsFreezed() && level->IsGlobalScope();
                return level->IsGlobalScope();
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

        auto PushLevel(DeclScope scope) -> void;
        auto PopLevel() -> void;

        // Find a declaration by name
        auto FindSymbol(StringView name) const -> const AstDecl*;

        // Find a function declaration by name and argument types
        auto FindFunction(StringView name, const std::vector<const Type*>& argTypes, bool requireExactMatch) const
            -> const AstFunctionDecl*;
    };
} // namespace glsld
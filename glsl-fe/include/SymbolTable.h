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

        auto AddFunction(AstFunctionDecl& decl) -> void;

        auto AddStructDecl(AstStructDecl& decl) -> void;

        auto AddInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void;

        auto AddVariableDecl(AstVariableDecl& decl) -> void;

        auto AddParamDecl(AstParamDecl& decl) -> void;

        auto FindFunction(const std::string& name, const std::vector<const TypeDesc*>& argTypes) const
            -> AstFunctionDecl*;

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
            std::vector<const TypeDesc*> paramTypes;
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
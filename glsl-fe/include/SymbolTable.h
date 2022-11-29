#pragma once
#include "Common.h"
#include "AstDecl.h"
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

        auto AddSymbol(std::string name, AstDecl* decl)
        {
            GLSLD_ASSERT(!levels.empty());
            levels.back().declLookup[name] = decl;
        }

        auto FindSymbol(const std::string& name) -> AstDecl*
        {
            GLSLD_ASSERT(!levels.empty());
            for (const auto& level : levels | std::views::reverse) {
                if (auto it = level.declLookup.find(name); it != level.declLookup.end()) {
                    return it->second;
                }
            }

            return nullptr;
        }

    private:
        struct SymbolTableLevel
        {
            std::unordered_map<std::string, AstDecl*> declLookup;
        };

        std::vector<SymbolTableLevel> levels;
    };
} // namespace glsld
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

        auto AddFunction(AstFunctionDecl& decl) -> void
        {
            GLSLD_ASSERT(levels.size() == 1);
            // deduplicate?
            auto name = decl.GetName().text.Str();
            if (!name.empty()) {
                funcDeclLookup.insert({decl.GetName().text.Str(), &decl});
            }
        }

        auto FindFunction(const std::string& name, const std::vector<const TypeDesc*>& argTypes) -> AstFunctionDecl*
        {
            // FIXME: impl correct resolution
            auto [itBegin, itEnd] = funcDeclLookup.equal_range(name);
            for (auto it = itBegin; it != itEnd; ++it) {
                auto funcDecl = it->second;
                std::vector<const TypeDesc*> paramTypes;
                for (auto paramDecl : funcDecl->GetParams()) {
                    paramTypes.push_back(paramDecl->GetType()->GetTypeDesc());
                }

                if (paramTypes == argTypes) {
                    return it->second;
                }
            }

            return nullptr;
        }

        auto AddSymbol(std::string name, AstDecl* decl) -> void
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

        std::unordered_multimap<std::string, AstFunctionDecl*> funcDeclLookup;
        std::vector<SymbolTableLevel> levels;
    };
} // namespace glsld
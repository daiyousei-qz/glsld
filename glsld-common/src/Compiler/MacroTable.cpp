#include "Compiler/MacroTable.h"

namespace glsld
{
    auto MacroTable::DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void
    {
        macroLookup.insert(std::make_pair(
            defToken.text, MacroDefinition::CreateObjectLikeMacro(defToken, std::move(expansionTokens))));
    }

    auto MacroTable::DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                             std::vector<PPToken> expansionTokens) -> void
    {
        macroLookup.insert(
            std::make_pair(defToken.text, MacroDefinition::CreateFunctionLikeMacro(defToken, std::move(paramTokens),
                                                                                   std::move(expansionTokens))));
    }

    auto MacroTable::UndefineMacro(AtomString macroName) -> void
    {
        macroLookup.erase(macroName);
    }

    auto MacroTable::IsMacroDefined(AtomString macroName) const -> bool
    {
        return macroLookup.find(macroName) != macroLookup.end();
    }

    auto MacroTable::FindMacroDefinition(AtomString macroName) const -> const MacroDefinition*
    {
        auto it = macroLookup.find(macroName);
        if (it != macroLookup.end()) {
            return &it->second;
        }
        return nullptr;
    }

    auto MacroTable::FindEnabledMacroDefinition(AtomString macroName) -> MacroDefinition*
    {
        auto it = macroLookup.find(macroName);
        if (it != macroLookup.end()) {
            if (it->second.IsEnabled()) {
                return &it->second;
            }
        }
        return nullptr;
    }
} // namespace glsld
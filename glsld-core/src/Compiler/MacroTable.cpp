#include "Compiler/MacroTable.h"

namespace glsld
{
    auto MacroTable::DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void
    {
        macroLookup.insert(std::make_pair(
            defToken.text, MacroDefinition::CreateObjectLikeMacro(defToken, std::move(expansionTokens))));
        // FIXME: report error for redefinition
    }

    auto MacroTable::DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                             std::vector<PPToken> expansionTokens) -> void
    {
        macroLookup.insert(
            std::make_pair(defToken.text, MacroDefinition::CreateFunctionLikeMacro(defToken, std::move(paramTokens),
                                                                                   std::move(expansionTokens))));
        // FIXME: report error for redefinition
    }

    auto MacroTable::DefineFeatureMacro(AtomString macroName, AtomString number) -> void
    {
        auto [_, inserted] =
            macroLookup.insert(std::make_pair(macroName, MacroDefinition::CreateFeatureMacro(macroName, number)));
        GLSLD_ASSERT(inserted);
    }

    auto MacroTable::UndefineMacro(AtomString macroName) -> void
    {
        auto it = macroLookup.find(macroName);
        if (it != macroLookup.end()) {
            if (!it->second.IsCompilerDefined()) {
                macroLookup.erase(it);
            }
            // FIXME: report error for effort to undefine compiler defined macro
        }
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
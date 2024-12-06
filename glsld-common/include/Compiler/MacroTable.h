#pragma once
#include "Basic/AtomTable.h"
#include "Compiler/MacroDefinition.h"

namespace glsld
{
    class MacroTable
    {
    private:
        std::unordered_map<AtomString, MacroDefinition> macroLookup;

    public:
        auto DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void;

        auto DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                     std::vector<PPToken> expansionTokens) -> void;

        auto UndefineMacro(AtomString macroName) -> void;

        auto IsMacroDefined(AtomString macroName) const -> bool;

        auto FindMacroDefinition(AtomString macroName) const -> const MacroDefinition*;

        auto FindEnabledMacroDefinition(AtomString macroName) -> MacroDefinition*;
    };
}; // namespace glsld
#pragma once
#include "Basic/AtomTable.h"
#include "Compiler/SyntaxToken.h"

namespace glsld
{
    struct MacroDefinition final
    {
        // A compiler defined macro is a macro that is defined by the compiler itself.
        // Therefore, it cannot be redefined or undefined by the user.
        bool isCompilerDefined = false;

        // If the macro is function-like. This is only valid if `isUndef` is false.
        bool isFunctionLike;

        // The line number of which the macro is defined in the main source file.
        // If the macro is defined in an included file, this is the line number of the `#include` directive.
        // int mainFileLine = 0;

        // The token of the macro name
        // e.g. `#define FOO` -> `FOO`
        PPToken defToken;

        // The tokens of the macro parameters
        // e.g. `#define FOO(a, b)` -> `a` and `b`
        std::vector<PPToken> paramTokens;

        // The tokens of the macro expansion
        // e.g. `#define FOO(a, b) a + b` -> `a + b`
        std::vector<PPToken> expansionTokens;
    };

    // Macro table maintains the currently defined macros. When a macro is undefined, it is dropped from the table.
    class MacroTable final
    {
    private:
        std::unordered_map<AtomString, MacroDefinition> macroLookup;

    public:
        // Note that caller should make sure the imported macro table has longer lifetime than this macro table.
        MacroTable(const MacroTable* preambleMacroTable);

        // Get a view to currently defined macros.
        auto GetMacroDefinitions() const
        {
            return macroLookup | std::views::values;
        }

        auto DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void;

        auto DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                     std::vector<PPToken> expansionTokens) -> void;

        auto DefineFeatureMacro(AtomString macroName, AtomString one) -> void;

        auto UndefineMacro(AtomString macroName) -> void;

        auto IsMacroDefined(AtomString macroName) const -> bool;

        auto FindMacroDefinition(AtomString macroName) const -> const MacroDefinition*;
    };
}; // namespace glsld
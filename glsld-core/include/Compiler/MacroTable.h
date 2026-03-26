#pragma once
#include "Basic/AtomTable.h"
#include "Compiler/SyntaxToken.h"

namespace glsld
{
    class MacroDefinition final
    {
    private:
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

        MacroDefinition(bool isCompilerDefined, bool isFunctionLike, PPToken defToken, std::vector<PPToken> paramTokens,
                        std::vector<PPToken> expansionTokens)
            : isCompilerDefined(isCompilerDefined), isFunctionLike(isFunctionLike), defToken(defToken),
              paramTokens(std::move(paramTokens)), expansionTokens(std::move(expansionTokens))
        {
        }

    public:
        static auto CreateObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> MacroDefinition
        {
            return MacroDefinition{
                false, false, defToken, {}, std::move(expansionTokens),
            };
        }

        static auto CreateFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                            std::vector<PPToken> expansionTokens) -> MacroDefinition
        {
            return MacroDefinition{
                false, true, defToken, std::move(paramTokens), std::move(expansionTokens),
            };
        }

        static auto CreateFeatureMacro(AtomString name, AtomString number) -> MacroDefinition
        {
            auto defToken = PPToken{
                .klass                = TokenKlass::Identifier,
                .spelledFile          = FileID::SystemPreamble(),
                .spelledRange         = {},
                .text                 = name,
                .isFirstTokenOfLine   = false,
                .hasLeadingWhitespace = false,
            };
            auto oneToken = PPToken{
                .klass                = TokenKlass::NumberLiteral,
                .spelledFile          = FileID::SystemPreamble(),
                .spelledRange         = {},
                .text                 = number,
                .isFirstTokenOfLine   = false,
                .hasLeadingWhitespace = false,
            };
            return MacroDefinition{
                true, false, defToken, {}, {oneToken},
            };
        }

        auto IsCompilerDefined() const noexcept -> bool
        {
            return isCompilerDefined;
        }

        auto IsFunctionLike() const noexcept -> bool
        {
            return isFunctionLike;
        }

        auto GetDefToken() const noexcept -> const PPToken&
        {
            return defToken;
        }

        auto GetParamTokens() const noexcept -> ArrayView<PPToken>
        {
            return paramTokens;
        }

        auto GetExpansionTokens() const noexcept -> ArrayView<PPToken>
        {
            return expansionTokens;
        }
    };

    class MacroTable
    {
    private:
        std::unordered_map<AtomString, MacroDefinition> macroLookup;

    public:
        // Note that caller should make sure the imported macro table has longer lifetime than this macro table.
        MacroTable(const MacroTable* preambleMacroTable);

        auto DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void;

        auto DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                     std::vector<PPToken> expansionTokens) -> void;

        auto DefineFeatureMacro(AtomString macroName, AtomString one) -> void;

        auto UndefineMacro(AtomString macroName) -> void;

        auto IsMacroDefined(AtomString macroName) const -> bool;

        auto FindMacroDefinition(AtomString macroName) const -> const MacroDefinition*;
    };
}; // namespace glsld
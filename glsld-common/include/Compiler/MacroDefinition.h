#pragma once

#include "Compiler/SyntaxToken.h"

namespace glsld
{
    class MacroDefinition final
    {
    public:
        static auto CreateObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> MacroDefinition
        {
            return MacroDefinition{
                false,
                defToken,
                {},
                std::move(expansionTokens),
            };
        }

        static auto CreateFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                            std::vector<PPToken> expansionTokens) -> MacroDefinition
        {
            return MacroDefinition{
                true,
                defToken,
                std::move(paramTokens),
                std::move(expansionTokens),
            };
        }

        auto IsEnabled() const noexcept -> bool
        {
            return !isDisabled;
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

        auto Enable() -> void
        {
            GLSLD_ASSERT(isDisabled);
            isDisabled = false;
        }

        auto Disable() -> void
        {
            GLSLD_ASSERT(!isDisabled);
            isDisabled = true;
        }

    private:
        MacroDefinition(bool isFunctionLike, PPToken defToken, std::vector<PPToken> paramTokens,
                        std::vector<PPToken> expansionTokens)
            : isFunctionLike(isFunctionLike), defToken(defToken), paramTokens(std::move(paramTokens)),
              expansionTokens(std::move(expansionTokens))
        {
        }

        // This is mutable during the compilation process. We may need to disable a macro temporarily
        // when it is currently being expanded to avoid infinite recursion.
        bool isDisabled = false;

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
} // namespace glsld
#pragma once
#include "SyntaxToken.h"

#include <vector>

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

    // This class maintains the global states shared across different preprocessor instances.
    // NOTE a new instance of Preprocessor could be created for each included source file.
    class PreprocessContext final
    {
    public:
        auto GetIncludeDepth() const noexcept -> int
        {
            return includeDepth;
        }

        auto EnterIncludeFile() -> void
        {
            includeDepth += 1;
        }

        auto ExitIncludeFile() -> void
        {
            includeDepth -= 1;
        }

        auto DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void
        {
            macroLookup.insert(std::make_pair(
                defToken.text, MacroDefinition::CreateObjectLikeMacro(defToken, std::move(expansionTokens))));
        }

        auto DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                     std::vector<PPToken> expansionTokens) -> void
        {
            macroLookup.insert(
                std::make_pair(defToken.text, MacroDefinition::CreateFunctionLikeMacro(defToken, std::move(paramTokens),
                                                                                       std::move(expansionTokens))));
        }

        auto UndefineMacro(AtomString macroName) -> void
        {
            macroLookup.erase(macroName);
        }

        auto FindMacroDefinition(AtomString macroName) const -> const MacroDefinition*
        {
            auto it = macroLookup.find(macroName);
            if (it != macroLookup.end()) {
                return &it->second;
            }
            return nullptr;
        }

        auto FindEnabledMacroDefinition(AtomString macroName) -> MacroDefinition*
        {
            auto it = macroLookup.find(macroName);
            if (it != macroLookup.end()) {
                if (it->second.IsEnabled()) {
                    return &it->second;
                }
            }
            return nullptr;
        }

    private:
        int includeDepth = 0;

        std::unordered_map<AtomString, MacroDefinition> macroLookup;
    };

} // namespace glsld
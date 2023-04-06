#pragma once
#include "SyntaxToken.h"

#include <vector>

namespace glsld
{
    class MacroDefinition final
    {
    public:
        static auto CreateObjectLikeMacro(PPTokenData defToken, std::vector<PPTokenData> expansionTokens)
            -> MacroDefinition
        {
            return MacroDefinition{
                false,
                defToken,
                {},
                std::move(expansionTokens),
            };
        }

        static auto CreateFunctionLikeMacro(PPTokenData defToken, std::vector<PPTokenData> paramTokens,
                                            std::vector<PPTokenData> expansionTokens) -> MacroDefinition
        {
            return MacroDefinition{
                true,
                defToken,
                std::move(paramTokens),
                std::move(expansionTokens),
            };
        }

        auto IsDisabled() const noexcept -> bool
        {
            return isDisabled;
        }

        auto IsFunctionLike() const noexcept -> bool
        {
            return isFunctionLike;
        }

        auto GetDefToken() const noexcept -> const PPTokenData&
        {
            return defToken;
        }

        auto GetParamTokens() const noexcept -> ArrayView<PPTokenData>
        {
            return paramTokens;
        }

        auto GetExpansionTokens() const noexcept -> ArrayView<PPTokenData>
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
        MacroDefinition(bool isFunctionLike, PPTokenData defToken, std::vector<PPTokenData> paramTokens,
                        std::vector<PPTokenData> expansionTokens)
            : isFunctionLike(isFunctionLike), defToken(defToken), paramTokens(std::move(paramTokens)),
              expansionTokens(std::move(expansionTokens))
        {
        }

        //
        bool isDisabled = false;

        // If the macro is function-like. This is only valid if `isUndef` is false.
        bool isFunctionLike;

        // The token of the macro name
        // e.g. `#define FOO` -> `FOO`
        PPTokenData defToken;

        // The tokens of the macro parameters
        // e.g. `#define FOO(a, b)` -> `a` and `b`
        std::vector<PPTokenData> paramTokens;

        // The tokens of the macro expansion
        // e.g. `#define FOO(a, b) a + b` -> `a + b`
        std::vector<PPTokenData> expansionTokens;
    };

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

        auto DefineObjectLikeMacro(PPTokenData defToken, std::vector<PPTokenData> expansionTokens) -> void
        {
            macroLookup.insert(std::make_pair(
                defToken.text, MacroDefinition::CreateObjectLikeMacro(defToken, std::move(expansionTokens))));
        }

        auto DefineFunctionLikeMacro(PPTokenData defToken, std::vector<PPTokenData> paramTokens,
                                     std::vector<PPTokenData> expansionTokens) -> void
        {
            macroLookup.insert(
                std::make_pair(defToken.text, MacroDefinition::CreateFunctionLikeMacro(defToken, std::move(paramTokens),
                                                                                       std::move(expansionTokens))));
        }

        auto UndefineMacro(LexString macroName) -> void
        {
            macroLookup.erase(macroName);
        }

        auto FindMacroDefinition(LexString macroName) const -> const MacroDefinition*
        {
            auto it = macroLookup.find(macroName);
            if (it != macroLookup.end()) {
                return &it->second;
            }
            return nullptr;
        }

        auto FindEnabledMacroDefinition(LexString macroName) -> MacroDefinition*
        {
            auto it = macroLookup.find(macroName);
            if (it != macroLookup.end()) {
                if (!it->second.IsDisabled()) {
                    return &it->second;
                }
            }
            return nullptr;
        }

    private:
        int includeDepth = 0;

        std::unordered_map<LexString, MacroDefinition> macroLookup;
    };

} // namespace glsld
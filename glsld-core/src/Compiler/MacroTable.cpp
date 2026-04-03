#include "Compiler/MacroTable.h"

namespace glsld
{
    MacroTable::MacroTable(const MacroTable* preambleMacroTable)
    {
        if (preambleMacroTable) {
            macroLookup = preambleMacroTable->macroLookup;
        }
    }

    auto MacroTable::DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void
    {
        macroLookup.insert(std::make_pair(defToken.text, MacroDefinition{
                                                             .isCompilerDefined = false,
                                                             .isFunctionLike    = false,
                                                             .defToken          = defToken,
                                                             .paramTokens       = {},
                                                             .expansionTokens   = std::move(expansionTokens),
                                                         }));
        // FIXME: report error for redefinition
    }

    auto MacroTable::DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                             std::vector<PPToken> expansionTokens) -> void
    {
        macroLookup.insert(std::make_pair(defToken.text, MacroDefinition{
                                                             .isCompilerDefined = false,
                                                             .isFunctionLike    = true,
                                                             .defToken          = defToken,
                                                             .paramTokens       = std::move(paramTokens),
                                                             .expansionTokens   = std::move(expansionTokens),
                                                         }));
        // FIXME: report error for redefinition
    }

    auto MacroTable::DefineFeatureMacro(AtomString macroName, AtomString number) -> void
    {
        auto defToken = PPToken{
            .klass                = TokenKlass::Identifier,
            .spelledFile          = FileID::SystemPreamble(),
            .spelledRange         = {},
            .text                 = macroName,
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
        auto [_, inserted] = macroLookup.insert(std::make_pair(macroName, MacroDefinition{
                                                                              .isCompilerDefined = true,
                                                                              .isFunctionLike    = false,
                                                                              .defToken          = defToken,
                                                                              .paramTokens       = {},
                                                                              .expansionTokens   = {oneToken},
                                                                          }));
        GLSLD_ASSERT(inserted);
    }

    auto MacroTable::UndefineMacro(AtomString macroName) -> void
    {
        auto it = macroLookup.find(macroName);
        if (it != macroLookup.end()) {
            if (!it->second.isCompilerDefined) {
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
} // namespace glsld
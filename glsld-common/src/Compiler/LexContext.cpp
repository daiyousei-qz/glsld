#include "Compiler/LexContext.h"
#include "Compiler/MacroExpansion.h"

#include <algorithm>

namespace glsld
{
    namespace
    {
        auto GetLanguageAtomTable() -> const AtomTable&
        {
            static AtomTable languageAtomTable = []() {
                AtomTable result;

#define DECL_KEYWORD(KEYWORD) result.GetAtom(#KEYWORD);
#include "GlslKeywords.inc"
#undef DECL_KEYWORD

                return result;
            }();

            return languageAtomTable;
        }

        // Our lexer doesn't know about GLSL keywords and treats them as identifiers.
        // So we have to fix the token class before adding it to the LexContext.
        auto FixKeywordTokenKlass(TokenKlass klass, AtomString text) -> TokenKlass
        {
            static std::unordered_map<AtomString, TokenKlass> keywordLookup = []() {
                std::unordered_map<AtomString, TokenKlass> result;

#define DECL_KEYWORD(KEYWORD) result[GetLanguageAtomTable().GetAtom(#KEYWORD)] = TokenKlass::K_##KEYWORD;
#include "GlslKeywords.inc"
#undef DECL_KEYWORD

                return result;
            }();

            if (klass == TokenKlass::Identifier) {
                if (auto it = keywordLookup.find(text); it != keywordLookup.end()) {
                    return it->second;
                }
            }

            return klass;
        }

    } // namespace

    LexContext::LexContext(const LexContext* preambleContext) : CompilerContextBase(preambleContext)
    {
        if (preambleContext) {
            atomTable.Import(preambleContext->atomTable);
            tokenIndexOffset = preambleContext->tokenIndexOffset + preambleContext->tokens.size();
            macroLookup      = preambleContext->macroLookup;
        }
        else {
            atomTable.Import(GetLanguageAtomTable());
        }
    }
    LexContext::~LexContext()
    {
    }

    auto LexContext::AddToken(const PPToken& token, TextRange expandedRange) -> void
    {
        GLSLD_ASSERT(tokens.empty() || tokens.back().klass != TokenKlass::Eof);

        if (token.klass != TokenKlass::Comment) {
            tokens.push_back(RawSyntaxTokenEntry{
                .klass         = FixKeywordTokenKlass(token.klass, token.text),
                .spelledFile   = token.spelledFile,
                .spelledRange  = token.spelledRange,
                .expandedRange = expandedRange,
                .text          = token.text,
            });
        }
    }

    auto LexContext::GetLastTUToken() const -> SyntaxToken
    {
        return SyntaxToken{
            .index = static_cast<SyntaxTokenIndex>(tokenIndexOffset + tokens.size() - 1),
            .klass = tokens.back().klass,
            .text  = tokens.back().text,
        };
    }

    auto LexContext::GetTUToken(SyntaxTokenIndex tokIndex) const -> SyntaxToken
    {
        GLSLD_ASSERT(tokIndex >= tokenIndexOffset && tokIndex - tokenIndexOffset < tokens.size());

        return SyntaxToken{
            .index = tokIndex,
            .klass = tokens[tokIndex - tokenIndexOffset].klass,
            .text  = tokens[tokIndex - tokenIndexOffset].text,
        };
    }

    auto LexContext::GetTUTokenSafe(SyntaxTokenIndex tokIndex) const -> SyntaxToken
    {
        GLSLD_ASSERT(tokIndex >= tokenIndexOffset);

        if (tokIndex < tokenIndexOffset + tokens.size()) {
            return GetTUToken(tokIndex);
        }
        else {
            return GetLastTUToken();
        }
    }

    auto LexContext::FindTokenByTextPosition(TextPosition position) const -> SyntaxToken
    {
        auto it = std::ranges::lower_bound(tokens, position, {},
                                           [](const RawSyntaxTokenEntry& tok) { return tok.expandedRange.start; });
        if (it != tokens.end() && it != tokens.begin()) {
            SyntaxTokenIndex index = std::distance(tokens.begin(), it) - 1;
            return SyntaxToken{
                .index = index,
                .klass = tokens[index].klass,
                .text  = tokens[index].text,
            };
        }
        else {
            return GetLastTUToken();
        }
    }

    auto LexContext::LookupSpelledFile(SyntaxTokenIndex tokIndex) const -> FileID
    {
        if (tokIndex < tokenIndexOffset) {
            GLSLD_ASSERT(GetPreambleContext());
            return GetPreambleContext()->LookupSpelledFile(tokIndex);
        }

        GLSLD_ASSERT(tokIndex < tokenIndexOffset + tokens.size());
        return tokens[tokIndex - tokenIndexOffset].spelledFile;
    }

    auto LexContext::LookupSpelledTextRange(SyntaxTokenIndex tokIndex) const -> FileTextRange
    {
        if (tokIndex < tokenIndexOffset) {
            GLSLD_ASSERT(GetPreambleContext());
            return GetPreambleContext()->LookupSpelledTextRange(tokIndex);
        }

        GLSLD_ASSERT(tokIndex < tokenIndexOffset + tokens.size());
        return FileTextRange{
            .fileID = tokens[tokIndex - tokenIndexOffset].spelledFile,
            .range  = tokens[tokIndex - tokenIndexOffset].spelledRange,
        };
    }

    auto LexContext::LookupExpandedTextRange(SyntaxTokenIndex tokIndex) const -> TextRange
    {
        if (tokIndex < tokenIndexOffset) {
            GLSLD_ASSERT(GetPreambleContext());
            if (GetPreambleContext()->GetTUMainFileID() == GetTUMainFileID()) {
                return GetPreambleContext()->LookupExpandedTextRange(tokIndex);
            }
            else {
                // If the preamble has a different main file, we assume it's expanded to the beginning of this
                // translation unit.
                return TextRange{};
            }
        }

        GLSLD_ASSERT(tokIndex < tokenIndexOffset + tokens.size());
        return tokens[tokIndex - tokenIndexOffset].expandedRange;
    }

    auto LexContext::EnterIncludeFile() -> void
    {
        includeDepth += 1;
    }

    auto LexContext::ExitIncludeFile() -> void
    {
        includeDepth -= 1;
    }

    auto LexContext::DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void
    {
        macroLookup.insert(std::make_pair(
            defToken.text, MacroDefinition::CreateObjectLikeMacro(defToken, std::move(expansionTokens))));
    }

    auto LexContext::DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                             std::vector<PPToken> expansionTokens) -> void
    {
        macroLookup.insert(
            std::make_pair(defToken.text, MacroDefinition::CreateFunctionLikeMacro(defToken, std::move(paramTokens),
                                                                                   std::move(expansionTokens))));
    }

    auto LexContext::UndefineMacro(AtomString macroName) -> void
    {
        macroLookup.erase(macroName);
    }

    auto LexContext::FindMacroDefinition(AtomString macroName) const -> const MacroDefinition*
    {
        auto it = macroLookup.find(macroName);
        if (it != macroLookup.end()) {
            return &it->second;
        }
        return nullptr;
    }

    auto LexContext::FindEnabledMacroDefinition(AtomString macroName) -> MacroDefinition*
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

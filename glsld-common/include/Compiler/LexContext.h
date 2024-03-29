#pragma once
#include "Compiler/CompilerContextBase.h"
#include "Compiler/SyntaxToken.h"

#include <vector>

namespace glsld
{
    class MacroDefinition;

    // This class manages everything related to lexing/preprocessing of a translation unit.
    class LexContext final : CompilerContextBase<LexContext>
    {
    private:
        AtomTable atomTable;

        // The first token index of this translation unit in the global token stream.
        // When compiling with a preamble, the first token in this translation unit doesn't have the id 0.
        SyntaxTokenIndex tokenIndexOffset = 0;

        // Tokens of the translation unit lexed from the source text, including those from included files and macro
        // expansion. It is guaranteed that the last token is always an EOF token spelled in the main file.
        std::vector<RawSyntaxTokenEntry> tokens;
        // std::vector<RawSyntaxToken> commentTokens;

        std::unordered_map<AtomString, MacroDefinition> macroLookup;

    public:
        LexContext(const LexContext* preambleContext);
        ~LexContext();

        // Get the AtomString of the given text in the atom table.
        auto GetAtomString(StringView text) -> AtomString
        {
            return atomTable.GetAtom(text);
        }

        // Add a new token to the token stream of this translation unit.
        auto AddToken(const PPToken& token, TextRange expandedRange) -> void;

        // Get the first token index of this translation unit in the global token stream.
        auto GetTUTokenIndexOffset() const noexcept -> SyntaxTokenIndex
        {
            return tokenIndexOffset;
        }

        // Get the number of tokens belonging to this translation unit in the global token stream.
        auto GetTUTokenCount() const noexcept -> size_t
        {
            return tokens.size();
        }

        auto GetTUMainFileID() const noexcept -> FileID
        {
            return tokens.back().spelledFile;
        }

        // Get the number of tokens in the global token stream.
        auto GetTotalTokenCount() const noexcept -> size_t
        {
            return tokenIndexOffset + tokens.size();
        }

        auto GetToken(SyntaxTokenIndex tokIndex) const -> SyntaxToken
        {
            if (tokIndex < tokenIndexOffset) {
                return GetPreambleContext()->GetToken(tokIndex);
            }

            return GetTUTokenSafe(tokIndex);
        }

        // Get the last syntax token from this translation unit.
        auto GetLastTUToken() const -> SyntaxToken;

        // Get a syntax token by index from this translation unit. The index must be valid.
        auto GetTUToken(SyntaxTokenIndex tokIndex) const -> SyntaxToken;

        // Get a syntax token by index from this translation unit.
        // If the index is invalid, the last token in the stream is returned.
        auto GetTUTokenSafe(SyntaxTokenIndex tokIndex) const -> SyntaxToken;

        // Find the last token that comes before the position or contains the position
        // 1. " ... | Tokan A |  ^  | Token B | ..."
        // 2. " ... | Tokan^ A | | Token B | ..."
        // Both token A should be returned
        auto FindTokenByTextPosition(TextPosition position) const -> SyntaxToken;

        // Find out the file that the specified token is spelled in.
        auto LookupSpelledFile(SyntaxTokenIndex tokIndex) const -> FileID;

        // Find out the file and text range that the specified token is spelled in.
        auto LookupSpelledTextRange(SyntaxTokenIndex tokIndex) const -> FileTextRange;

        // Find out the file and text range that the specified token is expanded to.
        auto LookupExpandedTextRange(SyntaxTokenIndex tokIndex) const -> TextRange;

        auto DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void;

        auto DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                     std::vector<PPToken> expansionTokens) -> void;

        auto UndefineMacro(AtomString macroName) -> void;

        auto FindMacroDefinition(AtomString macroName) const -> const MacroDefinition*;

        auto FindEnabledMacroDefinition(AtomString macroName) -> MacroDefinition*;
    };
} // namespace glsld
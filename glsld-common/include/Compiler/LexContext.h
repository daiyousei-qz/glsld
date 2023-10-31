#pragma once
#include "Compiler/CompilerContextBase.h"
#include "Compiler/SyntaxToken.h"

#include <vector>
#include <ranges>

namespace glsld
{
    class MacroDefinition;

    // This class manages everything related to lexing/preprocessing of a translation unit.
    class LexContext final : CompilerContextBase<LexContext>
    {
    private:
        AtomTable atomTable;

        // The first token index of this translation unit in the global token stream.
        SyntaxTokenIndex tokenIndexOffset = 0;

        // The token stream that is lexed from the source text, including from included files and macro expansion.
        std::vector<RawSyntaxTokenEntry> tokens;
        // std::vector<RawSyntaxToken> commentTokens;

        int includeDepth = 0;

        std::unordered_map<AtomString, MacroDefinition> macroLookup;

    public:
        LexContext(const LexContext* preambleContext);
        ~LexContext();

        // Get the AtomString of the given text in the atom table.
        auto GetAtomString(StringView text) -> AtomString
        {
            return atomTable.GetAtom(text);
        }

        // Get the first token index of this translation unit in the global token stream.
        auto GetTokenIndexOffset() const noexcept -> SyntaxTokenIndex
        {
            return tokenIndexOffset;
        }

        // Get the number of tokens belonging to this translation unit in the global token stream.
        auto GetTUTokenCount() const noexcept -> size_t
        {
            return tokens.size();
        }

        // Get the number of tokens in the global token stream.
        auto GetTotalTokenCount() const noexcept -> size_t
        {
            return tokenIndexOffset + tokens.size();
        }

        // Add a new token to the token stream of this translation unit.
        auto AddToken(const PPToken& token, TextRange expandedRange) -> void;

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

        auto LookupSpelledFile(const SyntaxToken& tok) const -> FileID;

        auto LookupSpelledFile(SyntaxTokenIndex tokIndex) const -> FileID;

        auto LookupSpelledTextRange(const SyntaxToken& tok) const -> FileTextRange;

        auto LookupSpelledTextRange(SyntaxTokenIndex tokIndex) const -> FileTextRange;

        auto LookupExpandedTextRange(const SyntaxToken& tok) const -> FileTextRange;

        auto LookupExpandedTextRange(SyntaxTokenIndex tokIndex) const -> FileTextRange;

        // NOTE we could only get the expanded range of an AstSyntaxRange because tokens could be spelled across
        // different files.
        auto LookupExpandedTextRange(AstSyntaxRange range) const -> std::optional<FileTextRange>;

        auto GetIncludeDepth() const noexcept -> int
        {
            return includeDepth;
        }

        auto EnterIncludeFile() -> void;

        auto ExitIncludeFile() -> void;

        auto DefineObjectLikeMacro(PPToken defToken, std::vector<PPToken> expansionTokens) -> void;

        auto DefineFunctionLikeMacro(PPToken defToken, std::vector<PPToken> paramTokens,
                                     std::vector<PPToken> expansionTokens) -> void;

        auto UndefineMacro(AtomString macroName) -> void;

        auto FindMacroDefinition(AtomString macroName) const -> const MacroDefinition*;

        auto FindEnabledMacroDefinition(AtomString macroName) -> MacroDefinition*;
    };
} // namespace glsld
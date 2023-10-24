#pragma once
#include "Compiler/SyntaxToken.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <ranges>

namespace glsld
{
    // This class manages everything about the source texts and tokens
    class LexContext
    {
    public:
        LexContext();
        ~LexContext();

        LexContext(const LexContext&)                    = delete;
        auto operator=(const LexContext&) -> LexContext& = delete;
        LexContext(LexContext&&)                         = delete;
        auto operator=(LexContext&&) -> LexContext&      = delete;

        // Add a new token to the token stream of this translation unit.
        auto AddToken(const PPToken& token, TextRange expandedRange) -> void;

        // Get a syntax token by index. The index must be valid.
        auto GetToken(SyntaxTokenIndex tokIndex) const -> SyntaxToken;

        // Get a syntax token by index. If the index is invalid, the last token in the stream is returned.
        auto GetTokenSafe(SyntaxTokenIndex tokIndex) const -> SyntaxToken;

        auto GetTokenCount() const noexcept -> size_t
        {
            return tokens.size();
        }

        // Get a view of all tokens in the stream.
        auto GetAllTokenView() const
        {
            return std::views::iota(SyntaxTokenIndex{0}, static_cast<SyntaxTokenIndex>(tokens.size())) |
                   std::views::transform([this](SyntaxTokenIndex index) { return GetToken(index); });
        }

        // Get a view of all tokens in the range
        auto GetRangedTokenView(AstSyntaxRange range) const
        {
            return std::views::iota(range.startTokenIndex, range.endTokenIndex) |
                   std::views::transform([this](SyntaxTokenIndex index) { return GetToken(index); });
        }

        // Find the last token that comes before the position or contains the position
        // 1. " ... | Tokan A |  ^  | Token B | ..."
        // 2. " ... | Tokan^ A | | Token B | ..."
        // Both token A should be returned
        auto FindTokenByTextPosition(TextPosition position) const -> SyntaxToken;

        auto LookupSpelledFile(const SyntaxToken& tok) const -> FileID
        {
            return LookupSpelledFile(tok.index);
        }

        auto LookupSpelledFile(SyntaxTokenIndex tokIndex) const -> FileID
        {
            GLSLD_ASSERT(tokIndex < tokens.size());
            return tokens[tokIndex].spelledFile;
        }

        auto LookupSpelledTextRange(const SyntaxToken& tok) const -> const TextRange&
        {
            return LookupSpelledTextRange(tok.index);
        }

        auto LookupSpelledTextRange(SyntaxTokenIndex tokIndex) const -> const TextRange&
        {
            GLSLD_ASSERT(tokIndex < tokens.size());
            return tokens[tokIndex].spelledRange;
        }

        auto LookupExpandedTextRange(const SyntaxToken& tok) const -> const TextRange&
        {
            return LookupExpandedTextRange(tok.index);
        }

        auto LookupExpandedTextRange(SyntaxTokenIndex tokIndex) const -> const TextRange&
        {
            GLSLD_ASSERT(tokIndex < tokens.size());
            return tokens[tokIndex].expandedRange;
        }

        // NOTE we could only get the expanded range of an AstSyntaxRange because tokens could be spelled across
        // different files.
        auto LookupExpandedTextRange(AstSyntaxRange range) const -> TextRange
        {
            GLSLD_ASSERT(range.endTokenIndex < tokens.size());
            if (range.endTokenIndex > range.startTokenIndex) {
                return TextRange{
                    tokens[range.startTokenIndex].expandedRange.start,
                    tokens[range.endTokenIndex - 1].expandedRange.end,
                };
            }
            else {
                return TextRange{
                    tokens[range.startTokenIndex].expandedRange.start,
                    tokens[range.startTokenIndex].expandedRange.start,
                };
            }
        }

        // Get the AtomString of the given text in the atom table.
        auto GetLexString(StringView text) -> AtomString
        {
            return atomTable.GetAtom(text);
        }

    private:
        AtomTable atomTable;

        // The token stream that is lexed from the source text, including from included files and macro expansion.
        std::vector<RawSyntaxTokenEntry> tokens;
        // std::vector<RawSyntaxToken> commentTokens;
    };
} // namespace glsld
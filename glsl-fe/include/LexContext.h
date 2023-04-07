#pragma once
#include "SyntaxToken.h"

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

        // If the context is finalized, no more tokens/macros can be added
        auto IsFinalized() const noexcept -> bool
        {
            if (!tokens.empty() && tokens.back().klass == TokenKlass::Eof) {
                return true;
            }

            return false;
        }

        // Add a new token to the token stream of this translation unit.
        auto AddToken(FileID fileID, const PPToken& token) -> void;

        // Get a syntax token by index
        auto GetToken(SyntaxTokenIndex tokIndex) const -> SyntaxToken;

        // Get a syntax token by index or EOF if the index is out of range
        auto GetTokenOrEof(SyntaxTokenIndex tokIndex) const -> SyntaxToken;

        // Get a view of all tokens
        auto GetAllTokenView() const
        {
            return std::views::iota(SyntaxTokenIndex{0}, static_cast<SyntaxTokenIndex>(tokens.size())) |
                   std::views::transform([this](SyntaxTokenIndex index) { return GetToken(index); });
        }

        // Get a view of all tokens in the range
        auto GetRangedTokenView(SyntaxTokenRange range) const
        {
            return std::views::iota(range.startTokenIndex, range.endTokenIndex) |
                   std::views::transform([this](SyntaxTokenIndex index) { return GetToken(index); });
        }

        // Find the last token that comes before the position or contains the position
        // 1. " ... | Tokan A |  ^  | Token B | ..."
        // 2. " ... | Tokan^ A | | Token B | ..."
        // Both token A should be returned
        auto FindTokenByTextPosition(TextPosition position) const -> SyntaxToken;

        auto LookupFile(SyntaxTokenIndex tokIndex) const -> FileID
        {
            GLSLD_ASSERT(tokIndex < tokens.size());
            return tokens[tokIndex].file;
        }

        // Compute the text range of the token with the given index
        auto LookupTextRange(SyntaxTokenIndex tokIndex) const -> TextRange;

        // Compute the text range of the token
        auto LookupTextRange(SyntaxToken token) const -> TextRange;

        // Compute the text range of all tokens in the range
        auto LookupTextRange(SyntaxTokenRange range) const -> TextRange;

        auto LookupFirstTextPosition(SyntaxTokenRange range) const -> TextPosition
        {
            return tokens[range.startTokenIndex].range.start;
        }
        auto LookupLastTextPosition(SyntaxTokenRange range) const -> TextPosition
        {
            return tokens[range.endTokenIndex].range.end;
        }

        // Get the AtomString of the given text in the atom table.
        auto GetLexString(StringView text) -> AtomString
        {
            return atomTable.GetAtom(text);
        }

    private:
        AtomTable atomTable;

        // The token stream that is lexed from the source text, including from included files and macro expansion.
        std::vector<RawSyntaxToken> tokens;
        std::vector<RawSyntaxToken> commentTokens;
    };
} // namespace glsld
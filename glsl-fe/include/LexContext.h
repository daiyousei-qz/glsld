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
        LexContext()
        {
            InitializeKeywordLookup();
        }

        ~LexContext()
        {
            for (StringView s : atomTable) {
                delete[] s.Data();
            }
        }

        LexContext(const LexContext&)                    = delete;
        auto operator=(const LexContext&) -> LexContext& = delete;

        LexContext(LexContext&&)                    = default;
        auto operator=(LexContext&&) -> LexContext& = default;

        // If the context is finalized, no more tokens/macros can be added
        auto IsFinalized() const noexcept -> bool
        {
            if (!tokens.empty() && tokens.back().klass == TokenKlass::Eof) {
                return true;
            }

            return false;
        }

        // Add a new token to the context
        auto AddToken(TokenKlass klass, TextRange range, LexString lexText) -> void;

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

        // Get the LexString of the given text in the atom table.
        auto GetLexString(StringView text) -> LexString
        {
            LexString atom;
            if (auto it = atomTable.find(text); it != atomTable.end()) {
                atom = LexString{it->Data()};
            }
            else {
                char* data = new char[text.Size() + 1];
                std::ranges::copy(text.StdStrView(), data);
                data[text.Size()] = '\0';

                atom = LexString{atomTable.insert(StringView(data, text.Size() + 1)).first->Data()};
            }

            return atom;
        }

    private:
        auto InitializeKeywordLookup() -> void;
        auto FixKeywordTokenKlass(TokenKlass klass, LexString text) const noexcept -> TokenKlass;

        // This StringView actually owns the string data, so we need to delete it when the context is destroyed
        // FIXME: optimize memory layout
        std::unordered_set<StringView> atomTable;

        // Lookup table for keywords. This is needed because our tokenizer only recognizes identifiers.
        // We need to fix up the token kind for keywords during token registration.
        std::unordered_map<LexString, TokenKlass> keywordLookup;

        // The token stream that is lexed from the source text, including from included files and macro expansion.
        std::vector<RawSyntaxToken> tokens;
        std::vector<RawSyntaxToken> commentTokens;
    };
} // namespace glsld
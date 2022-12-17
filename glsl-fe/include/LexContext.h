#pragma once
#include "SyntaxToken.h"

#include <algorithm>
#include <string>
#include <vector>
#include <unordered_set>
#include <ranges>

namespace glsld
{
    // This class manages everything about the source texts and tokens
    // FIXME: support token stream other than std::string
    // FIXME: support dynamic lexing
    class LexContext
    {
    public:
        LexContext()
        {
        }

        LexContext(const LexContext&)                    = delete;
        auto operator=(const LexContext&) -> LexContext& = delete;

        LexContext(LexContext&&)                    = default;
        auto operator=(LexContext&&) -> LexContext& = default;

        auto AddCommentToken(TextRange range, const std::string& lexText) -> void
        {
            GLSLD_ASSERT(tokens.empty() || tokens.back().klass != TokenKlass::Eof);

            LexString tokText;
            if (auto it = atomTable.find(lexText); it != atomTable.end()) {
                tokText = LexString{it->c_str()};
            }
            else {
                tokText = LexString{atomTable.insert(std::string{lexText}).first->c_str()};
            }

            commentTokens.push_back(SyntaxTokenInfo{
                .file  = 0,
                .klass = TokenKlass::Comment,
                .text  = tokText,
                .range = range,
            });
        }

        auto AddToken(TokenKlass klass, TextRange range, const std::string& lexText) -> SyntaxToken
        {
            GLSLD_ASSERT(tokens.empty() || tokens.back().klass != TokenKlass::Eof);
            GLSLD_ASSERT(klass != TokenKlass::Comment && klass != TokenKlass::Eof);

            LexString tokText;
            if (auto it = atomTable.find(lexText); it != atomTable.end()) {
                tokText = LexString{it->c_str()};
            }
            else {
                tokText = LexString{atomTable.insert(std::string{lexText}).first->c_str()};
            }

            auto& tokenContainer = klass != TokenKlass::Comment ? tokens : commentTokens;
            tokens.push_back(SyntaxTokenInfo{
                .file  = 0,
                .klass = klass,
                .text  = tokText,
                .range = range,
            });
            return GetToken(tokens.size() - 1);
        }

        auto AddEof(TextRange range) -> SyntaxToken
        {
            if (tokens.empty() || tokens.back().klass != TokenKlass::Eof) {
                tokens.push_back(SyntaxTokenInfo{
                    .file  = 0,
                    .klass = TokenKlass::Eof,
                    .text  = LexString{},
                    .range = range,
                });
            }

            return GetToken(tokens.size() - 1);
        }

        auto GetAllTokenView() const
        {
            return std::views::iota(SyntaxTokenIndex{0}, static_cast<SyntaxTokenIndex>(tokens.size())) |
                   std::views::transform([this](SyntaxTokenIndex index) { return GetToken(index); });
        }

        auto GetRangedTokenView(SyntaxTokenRange range) const
        {
            return std::views::iota(range.startTokenIndex, range.endTokenIndex) |
                   std::views::transform([this](SyntaxTokenIndex index) { return GetToken(index); });
        }

        auto GetToken(SyntaxTokenIndex tokIndex) const -> SyntaxToken
        {
            GLSLD_ASSERT(tokIndex < tokens.size());
            return SyntaxToken{
                .index = tokIndex,
                .klass = tokens[tokIndex].klass,
                .text  = tokens[tokIndex].text,
            };
        }

        auto GetTokenSafe(SyntaxTokenIndex tokIndex) const -> SyntaxToken
        {
            if (tokIndex >= tokens.size()) {
                return GetToken(tokens.size() - 1);
            }
            else {
                return GetToken(tokIndex);
            }
        }

        // Find last token that comes before the position or contains the position
        // 1. " ... | Tokan A |  ^  | Token B | ..."
        // 2. " ... | Tokan^ A | | Token B | ..."
        // Both token A should be returned
        auto FindTokenByPosition(TextPosition position) const -> SyntaxToken
        {
            auto it = std::ranges::lower_bound(tokens, position, {},
                                               [](const SyntaxTokenInfo& tok) { return tok.range.start; });
            if (it != tokens.end() && it != tokens.begin()) {
                SyntaxTokenIndex index = std::distance(tokens.begin(), it) - 1;
                return SyntaxToken{
                    .index = index,
                    .klass = tokens[index].klass,
                    .text  = tokens[index].text,
                };
            }
            else {
                return SyntaxToken{};
            }
        }

        auto LookupTextRange(SyntaxTokenIndex tokIndex) const -> TextRange
        {
            GLSLD_ASSERT(tokIndex < tokens.size());
            return tokens[tokIndex].range;
        }
        auto LookupTextRange(SyntaxToken token) const -> TextRange
        {
            if (token.IsValid()) {
                GLSLD_ASSERT(token.index < tokens.size());
                return tokens[token.index].range;
            }
            else {
                return TextRange{};
            }
        }
        auto LookupTextRange(SyntaxTokenRange range) const -> TextRange
        {
            GLSLD_ASSERT(range.endTokenIndex < tokens.size());
            if (range.endTokenIndex > range.startTokenIndex) {
                return TextRange{
                    tokens[range.startTokenIndex].range.start,
                    tokens[range.endTokenIndex - 1].range.end,
                };
            }
            else {
                return TextRange{
                    tokens[range.startTokenIndex].range.start,
                    tokens[range.startTokenIndex].range.start,
                };
            }
        }

        auto LookupFirstTextPosition(SyntaxTokenRange range) const -> TextPosition
        {
            return tokens[range.startTokenIndex].range.start;
        }
        auto LookupLastTextPosition(SyntaxTokenRange range) const -> TextPosition
        {
            return tokens[range.endTokenIndex].range.end;
        }

    private:
        // FIXME: optimize memory layout
        std::unordered_set<std::string> atomTable;

        std::vector<SyntaxTokenInfo> tokens;
        std::vector<SyntaxTokenInfo> commentTokens;
    };
} // namespace glsld
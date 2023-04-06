#include "LexContext.h"

namespace glsld
{
    auto LexContext::AddToken(TokenKlass klass, TextRange range, LexString text) -> void
    {
        GLSLD_ASSERT(tokens.empty() || tokens.back().klass != TokenKlass::Eof);

        if (klass != TokenKlass::Comment) {
            tokens.push_back(RawSyntaxToken{
                .file  = 0,
                .klass = FixKeywordTokenKlass(klass, text),
                .text  = text,
                .range = range,
            });
        }
    }

    auto LexContext::GetToken(SyntaxTokenIndex tokIndex) const -> SyntaxToken
    {
        GLSLD_ASSERT(tokIndex < tokens.size());
        return SyntaxToken{
            .index = tokIndex,
            .klass = tokens[tokIndex].klass,
            .text  = tokens[tokIndex].text,
        };
    }

    auto LexContext::GetTokenOrEof(SyntaxTokenIndex tokIndex) const -> SyntaxToken
    {
        if (tokIndex >= tokens.size()) {
            return GetToken(tokens.size() - 1);
        }
        else {
            return GetToken(tokIndex);
        }
    }

    auto LexContext::FindTokenByTextPosition(TextPosition position) const -> SyntaxToken
    {
        auto it =
            std::ranges::lower_bound(tokens, position, {}, [](const RawSyntaxToken& tok) { return tok.range.start; });
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

    auto LexContext::LookupTextRange(SyntaxTokenIndex tokIndex) const -> TextRange
    {
        GLSLD_ASSERT(tokIndex < tokens.size());
        return tokens[tokIndex].range;
    }

    auto LexContext::LookupTextRange(SyntaxToken token) const -> TextRange
    {
        if (token.IsValid()) {
            GLSLD_ASSERT(token.index < tokens.size());
            return tokens[token.index].range;
        }
        else {
            return TextRange{};
        }
    }

    auto LexContext::LookupTextRange(SyntaxTokenRange range) const -> TextRange
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

    auto LexContext::InitializeKeywordLookup() -> void
    {
#define DECL_KEYWORD(KEYWORD) keywordLookup[GetLexString(#KEYWORD)] = TokenKlass::K_##KEYWORD;
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
    }

    auto LexContext::FixKeywordTokenKlass(TokenKlass klass, LexString text) const noexcept -> TokenKlass
    {
        if (klass == TokenKlass::Identifier) {
            if (auto it = keywordLookup.find(text); it != keywordLookup.end()) {
                return it->second;
            }
        }

        return klass;
    }
} // namespace glsld

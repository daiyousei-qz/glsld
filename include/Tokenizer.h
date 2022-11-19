#pragma once
#include "Common.h"
#include "SyntaxTree.h"
#include <unordered_map>
#include <string_view>
#include <optional>

namespace glsld
{
    class RawSourceView
    {
    public:
        RawSourceView(std::string_view source_text)
            : line_(0), colomn_(0), cursor_(source_text.data()), begin_(source_text.data()),
              end_(source_text.data() + source_text.size())
        {
        }

        auto Eof() -> bool
        {
            return cursor_ == end_;
        }

        auto HeadCursor() -> const char*
        {
            return begin_;
        }

        auto LastCursor() -> const char*
        {
            GLSLD_ASSERT(cursor_ != begin_);
            return cursor_ - 1;
        }

        auto CurrentCursor() -> const char*
        {
            return cursor_;
        }

        auto Peek() -> char
        {
            GLSLD_ASSERT(!Eof());
            return *cursor_;
        }

        auto Consume() -> char
        {
            GLSLD_ASSERT(!Eof());

            char ch = *cursor_;
            if (ch == '\n')
            {
                line_ += 1;
                colomn_ = 0;
            }
            else
            {
                colomn_ += 1;
            }

            ++cursor_;
            return ch;
        }

    private:
        int line_;
        int colomn_;
        const char* cursor_;
        const char* begin_;
        const char* end_;
    };

    class Tokenizer
    {
    public:
        Tokenizer(std::string_view source) : source_view_(source)
        {
        }

        auto NextToken() -> SyntaxToken
        {
            while (!source_view_.Eof() && IsWhitespace(source_view_.Peek()))
            {
                source_view_.Consume();
            }

            auto beginCursor = source_view_.CurrentCursor();
            if (source_view_.Eof())
            {
                return SyntaxToken{
                    .range = {}, .klass = TokenKlass::Eof, .text = std::string_view{beginCursor, beginCursor}};
            }

            char firstChar   = source_view_.Consume();
            TokenKlass klass = TokenKlass::Error;
            if (firstChar == '\0')
            {
                // eof
                klass = TokenKlass::Eof;
            }
            else if (IsAlpha(firstChar))
            {
                // keyword or identifier
                klass = ParseIdentifier(firstChar);
            }
            else if (IsDigit(firstChar))
            {
                // number literal
                klass = ParseNumberLiteral(firstChar);
            }
            else
            {
                // punctuation or comment
                klass = ParsePunctuation(firstChar);
            }

            auto endCursor = source_view_.CurrentCursor();
            return SyntaxToken{.range = {}, .klass = klass, .text = std::string_view{beginCursor, endCursor}};
        }

    private:
        auto ParseIdentifier(char firstChar) -> TokenKlass
        {
            std::string s{firstChar};
            while (true)
            {
                if (source_view_.Eof())
                {
                    break;
                }

                char ch = source_view_.Peek();
                if (IsAlphanum(ch) || ch == '_')
                {
                    s.push_back(source_view_.Consume());
                }
                else
                {
                    break;
                }
            }

            static std::unordered_map<std::string_view, TokenKlass> keywordLookup = {
#define DECL_KEYWORD(KEYWORD) {#KEYWORD, TokenKlass::K_##KEYWORD},
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
            };
            if (auto it = keywordLookup.find(s); it != keywordLookup.end())
            {
                return it->second;
            }
            else
            {
                return TokenKlass::Identifier;
            }
        }
        auto ParseNumberLiteral(char firstChar) -> TokenKlass
        {
            while (true)
            {
                if (source_view_.Eof())
                {
                    break;
                }

                char ch = source_view_.Peek();
                if (IsDigit(ch))
                {
                    source_view_.Consume();
                }
                else
                {
                    break;
                }
            }

            return TokenKlass::IntegerConstant;
        }
        auto ParsePunctuation(char firstChar) -> TokenKlass
        {
            static std::unordered_map<std::string_view, TokenKlass> punctLookup = {
#define DECL_PUNCT(PUNCT_NAME, PUNCT_TEXT) {PUNCT_TEXT, TokenKlass::PUNCT_NAME},
#include "GlslPunctuation.inc"
#undef DECL_PUNCT
            };

            std::string s{firstChar};
            TokenKlass result = TokenKlass::Error;
            if (auto it = punctLookup.find(s); it != punctLookup.end())
            {
                // test one-char puncts
                result = it->second;
            }

            s.push_back(source_view_.Peek());
            if (auto it = punctLookup.find(s); it != punctLookup.end())
            {
                // test two-char puncts
                source_view_.Consume();
                result = it->second;
            }

            return result;
        }

        auto IsWhitespace(char ch) -> bool
        {
            switch (ch)
            {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                return true;
            default:
                return false;
            }
        }
        auto IsAlpha(char ch) -> bool
        {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
        }
        auto IsDigit(char ch) -> bool
        {
            return ch >= '0' && ch <= '9';
        }
        auto IsAlphanum(char ch) -> bool
        {
            return IsAlpha(ch) || IsDigit(ch);
        }

        RawSourceView source_view_;
    };
} // namespace glsld
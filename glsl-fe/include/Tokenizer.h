#pragma once
#include "Common.h"
#include "SyntaxToken.h"

#include <unordered_map>
#include <string_view>
#include <optional>

namespace glsld
{
    struct TokenInfo
    {
        TokenKlass klass;

        int rawOffset;
        int rawSize;

        // NOTE these are all 0-based
        int lineBegin;
        int lineEnd;
        int columnBegin;
        int columnEnd;
    };

    class RawSourceView
    {
    public:
        RawSourceView(std::string_view source_text)
            : line(0), column(0), cursor(source_text.data()), begin(source_text.data()),
              end(source_text.data() + source_text.size())
        {
        }

        auto Eof() -> bool
        {
            return cursor == end;
        }

        auto GetLineNum() -> int
        {
            return line;
        }

        auto GetColumnNum() -> int
        {
            return column;
        }

        auto HeadCursor() -> const char*
        {
            return begin;
        }

        auto LastCursor() -> const char*
        {
            GLSLD_ASSERT(cursor != begin);
            return cursor - 1;
        }

        auto CurrentCursor() -> const char*
        {
            return cursor;
        }

        auto Peek() -> char
        {
            GLSLD_ASSERT(!Eof());
            return *cursor;
        }

        auto Peek(int lookahead) -> char
        {
            if (end - cursor > lookahead) {
                return cursor[lookahead];
            }

            return '\0';
        }

        auto Consume() -> char
        {
            GLSLD_ASSERT(!Eof());

            char ch = *cursor;
            if (ch == '\n') {
                line += 1;
                column = 0;
            }
            else {
                column += 1;
            }

            ++cursor;
            return ch;
        }

    private:
        int line;
        int column;
        const char* cursor;
        const char* begin;
        const char* end;
    };

    // FIXME: tokenizer may issue error?
    class Tokenizer
    {
    public:
        Tokenizer(std::string_view source) : sourceView(source)
        {
        }

        auto NextToken(std::string& buf) -> TokenInfo
        {
            // skip whitespace
            while (!sourceView.Eof() && IsWhitespace(sourceView.Peek())) {
                sourceView.Consume();
            }

            // tokenize
            TokenInfo result;
            result.rawOffset   = sourceView.CurrentCursor() - sourceView.HeadCursor();
            result.lineBegin   = sourceView.GetLineNum();
            result.columnBegin = sourceView.GetColumnNum();

            auto beginCursor = sourceView.CurrentCursor();
            if (sourceView.Eof()) {
                result.klass     = TokenKlass::Eof;
                result.rawSize   = 0;
                result.lineEnd   = result.lineBegin;
                result.columnEnd = result.columnBegin;
                return result;
            }

            char firstChar   = sourceView.Consume();
            TokenKlass klass = TokenKlass::Error;
            if (firstChar == '\0') {
                // eof
                klass = TokenKlass::Eof;
            }
            else if (IsAlpha(firstChar)) {
                // keyword or identifier
                klass = ParseIdentifier(firstChar);
            }
            else if (IsDigit(firstChar)) {
                // number literal
                klass = ParseNumberLiteral(firstChar);
            }
            else if (firstChar == '/' && sourceView.Peek(0) == '/') {
                // line comment
                klass = ParseLineComment();
            }
            else if (firstChar == '/' && sourceView.Peek(0) == '*') {
                klass = ParseBlockComment();
            }
            else {
                // punctuation
                klass = ParsePunctuation(firstChar);
            }

            auto endCursor = sourceView.CurrentCursor();
            result.klass   = klass;
            result.rawSize = endCursor - beginCursor;
            // FIXME: this is actually one character ahead of the token
            result.lineEnd   = sourceView.GetLineNum();
            result.columnEnd = sourceView.GetColumnNum();

            buf.append(beginCursor, endCursor);
            return result;
        }

    private:
        auto ParseLineComment() -> TokenKlass
        {
            sourceView.Consume();
            sourceView.Consume();

            while (!sourceView.Eof() && sourceView.Peek() != '\n') {
                sourceView.Consume();
            }

            return TokenKlass::Comment;
        }

        auto ParseBlockComment() -> TokenKlass
        {
            sourceView.Consume();
            sourceView.Consume();

            while (!sourceView.Eof() && (sourceView.Peek() != '*' && sourceView.Peek(1) != '/')) {
                sourceView.Consume();
            }

            if (!sourceView.Eof()) {
                sourceView.Consume();
                sourceView.Consume();
                return TokenKlass::Comment;
            }
            else {
                return TokenKlass::Error;
            }
        }

        auto ParseIdentifier(char firstChar) -> TokenKlass
        {
            std::string s{firstChar};
            while (true) {
                if (sourceView.Eof()) {
                    break;
                }

                char ch = sourceView.Peek();
                if (IsAlphanum(ch) || ch == '_') {
                    s.push_back(sourceView.Consume());
                }
                else {
                    break;
                }
            }

            static std::unordered_map<std::string_view, TokenKlass> keywordLookup = {
#define DECL_KEYWORD(KEYWORD) {#KEYWORD, TokenKlass::K_##KEYWORD},
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
            };
            if (auto it = keywordLookup.find(s); it != keywordLookup.end()) {
                return it->second;
            }
            else {
                return TokenKlass::Identifier;
            }
        }
        auto ParseNumberLiteral(char firstChar) -> TokenKlass
        {
            while (true) {
                if (sourceView.Eof()) {
                    break;
                }

                char ch = sourceView.Peek();
                if (IsDigit(ch)) {
                    sourceView.Consume();
                }
                else {
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
            if (auto it = punctLookup.find(s); it != punctLookup.end()) {
                // test one-char puncts
                result = it->second;
            }

            if (!sourceView.Eof()) {
                s.push_back(sourceView.Peek());
                if (auto it = punctLookup.find(s); it != punctLookup.end()) {
                    // test two-char puncts
                    sourceView.Consume();
                    result = it->second;
                }
            }

            return result;
        }

        auto IsWhitespace(char ch) -> bool
        {
            switch (ch) {
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

        RawSourceView sourceView;
    };
} // namespace glsld
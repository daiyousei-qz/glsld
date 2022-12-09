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

    public:
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

        auto NextToken(std::string& buf) -> TokenInfo;

    private:
        auto ParsePPComment() -> TokenKlass
        {
            sourceView.Consume();

            while (!sourceView.Eof() && sourceView.Peek() != '\n') {
                sourceView.Consume();
            }

            return TokenKlass::Comment;
        }

        auto ParseLineComment() -> TokenKlass
        {
            // Parse second '/'
            sourceView.Consume();

            while (!sourceView.Eof() && sourceView.Peek() != '\n') {
                sourceView.Consume();
            }

            return TokenKlass::Comment;
        }

        auto ParseBlockComment() -> TokenKlass
        {
            // Parse '*'
            sourceView.Consume();

            while (!sourceView.Eof()) {
                if (sourceView.Peek(0) == '*' && sourceView.Peek(1) == '/') {
                    break;
                }
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
#include "Tokenizer.h"
#include "Compiler.h"
#include "LexContext.h"
#include "Preprocessor.h"
#include "CompilerTrace.h"

#include "Tokenize.Generated.inc"

namespace glsld
{
    Tokenizer::Tokenizer(CompilerObject& compilerObject, Preprocessor& preprocessor, FileID sourceFile,
                         StringView sourceText)
        : compilerObject(compilerObject), preprocessor(preprocessor), sourceFile(sourceFile)
    {
        srcScanner = SourceScanner{sourceText.Data(), sourceText.Data() + sourceText.Size()};
    }

    auto Tokenizer::DoTokenize() -> void
    {
        GLSLD_ASSERT(preprocessor.GetState() == PreprocessorState::Default);
        while (true) {
            PPToken token = LexPPToken();

            if (preprocessor.InActiveRegion()) {
                GLSLD_TRACE_TOKEN_LEXED(token);
                preprocessor.IssueToken(token);
            }
            else {
                // FIXME: We are in an inactive branch of preprocessor. Fast scan for # instead of expensive lexing.
                if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
                    GLSLD_TRACE_TOKEN_LEXED(token);
                    preprocessor.IssueToken(token);
                }
            }

            if (token.klass == TokenKlass::Eof) {
                break;
            }
        }
    }

    auto Tokenizer::LexPPToken() -> PPToken
    {
        bool skippedWhitespace = false;
        bool skippedNewLine    = srcScanner.CursorAtBegin();
        srcScanner.SkipWhitespace(skippedWhitespace, skippedNewLine);

        if (srcScanner.CursorAtEnd()) {
            // NOTE we always regard an EOF in a new line
            return PPToken{
                .klass                = TokenKlass::Eof,
                .spelledFile          = sourceFile,
                .spelledRange         = TextRange{srcScanner.GetTextPosition()},
                .text                 = {},
                .isFirstTokenOfLine   = true,
                .hasLeadingWhitespace = true,
            };
        }

        tokenTextBuffer.clear();
        TokenKlass klass      = TokenKlass::Error;
        TextPosition beginPos = srcScanner.GetTextPosition();

        if (srcScanner.PeekChar() == '/' && srcScanner.PeekChar(1) == '/') {
            klass = ParseLineComment();
        }
        else if (srcScanner.PeekChar() == '/' && srcScanner.PeekChar(1) == '*') {
            klass = ParseBlockComment();
        }
        else if (preprocessor.ShouldLexHeaderName() && srcScanner.PeekChar() == '"') {
            klass = ParseQuotedString();
        }
        else if (preprocessor.ShouldLexHeaderName() && srcScanner.PeekChar() == '<') {
            // klass = ParseAngleString();
        }
        else {
            klass = detail::Tokenize(srcScanner, tokenTextBuffer);
        }

        AtomString text     = compilerObject.GetLexContext().GetLexString(StringView{tokenTextBuffer});
        TextPosition endPos = srcScanner.GetTextPosition();

        return PPToken{
            .klass                = klass,
            .spelledFile          = sourceFile,
            .spelledRange         = TextRange{beginPos, endPos},
            .text                 = text,
            .isFirstTokenOfLine   = skippedNewLine,
            .hasLeadingWhitespace = skippedWhitespace,
        };
    }

    auto Tokenizer::ParseLineComment() -> TokenKlass
    {
        GLSLD_ASSERT(srcScanner.PeekChar() == '/' && srcScanner.PeekChar(1) == '/');

        // Consume "//"
        srcScanner.SkipChar(2);

        while (!srcScanner.CursorAtEnd()) {
            auto ch = srcScanner.PeekChar();

            if (ch != '\n') {
                srcScanner.ConsumeChar();
            }
            else {
                break;
            }
        }

        return TokenKlass::Comment;
    }

    auto Tokenizer::ParseBlockComment() -> TokenKlass
    {
        GLSLD_ASSERT(srcScanner.PeekChar() == '/' && srcScanner.PeekChar(1) == '*');

        // Consume "/*"
        srcScanner.SkipChar(2);

        while (!srcScanner.CursorAtEnd()) {
            auto ch = srcScanner.PeekChar();

            if (ch != '*' || srcScanner.PeekChar(1) != '/') {
                srcScanner.ConsumeChar();
            }
            else {
                // Consume "*/"
                srcScanner.SkipChar(2);
                return TokenKlass::Comment;
            }
        }

        return TokenKlass::Error;
    }

    auto Tokenizer::ParseQuotedString() -> TokenKlass
    {
        // FIXME: a quoted string shouldn't contain a newline

        GLSLD_ASSERT(srcScanner.PeekChar() == '"');

        // Consume '"'
        srcScanner.SkipChar(1);
        tokenTextBuffer.push_back('"');

        while (!srcScanner.CursorAtEnd()) {
            auto ch = srcScanner.PeekChar();
            tokenTextBuffer.push_back(ch);

            if (ch != '"') {
                srcScanner.ConsumeChar();
            }
            else {
                // Consume '"'
                srcScanner.SkipChar(1);
                return TokenKlass::QuotedString;
            }
        }

        return TokenKlass::Error;
    }

    auto Tokenizer::ParseAngleString() -> TokenKlass
    {
        GLSLD_ASSERT(srcScanner.PeekChar() == '<');

        // FIXME: a quoted string shouldn't contain a newline
        GLSLD_NO_IMPL();
    }

} // namespace glsld
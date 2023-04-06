#include "Tokenizer.h"
#include "Compiler.h"
#include "LexContext.h"
#include "Preprocessor.h"
#include "CompilerTrace.h"

#include "Tokenize.Generated.inc"

namespace glsld
{
    Tokenizer::Tokenizer(CompilerObject& compilerObject, Preprocessor& preprocessor, StringView sourceText)
        : compilerObject(compilerObject), preprocessor(preprocessor)
    {
        srcView = SourceScanner{sourceText.Data(), sourceText.Data() + sourceText.Size()};
    }

    auto Tokenizer::DoTokenize() -> void
    {
        GLSLD_ASSERT(preprocessor.GetState() == PreprocessorState::Default);
        while (true) {
            LoadNextPPToken();
            if (preprocessor.InActiveRegion()) {
                TracePPTokenLexed(GetCurrentPPToken());
                preprocessor.IssueToken(GetCurrentPPToken());
            }
            else {
                // FIXME: We are in an inactive branch of preprocessor. Fast scan for # instead of expensive lexing.
                if (GetCurrentPPToken().klass == TokenKlass::Hash && GetCurrentPPToken().isFirstTokenOfLine) {
                    TracePPTokenLexed(GetCurrentPPToken());
                    preprocessor.IssueToken(GetCurrentPPToken());
                }
            }

            if (GetCurrentPPToken().klass == TokenKlass::Eof) {
                break;
            }
        }
    }

    auto Tokenizer::LoadNextPPToken() -> void
    {
        bool skippedWhitespace = false;
        bool skippedNewLine    = srcView.CursorAtBegin();
        srcView.SkipWhitespace(skippedWhitespace, skippedNewLine);

        if (srcView.CursorAtEnd()) {
            // NOTE we always regard an EOF in a new line
            currentToken = PPTokenData{.klass                = TokenKlass::Eof,
                                       .isFirstTokenOfLine   = true,
                                       .hasLeadingWhitespace = skippedWhitespace,
                                       .text                 = {},
                                       .range                = {}};
            return;
        }

        tokenTextBuffer.clear();
        TokenKlass klass      = TokenKlass::Error;
        TextPosition beginPos = srcView.GetTextPosition();

        if (srcView.PeekChar() == '/' && srcView.PeekChar(1) == '/') {
            klass = ParseLineComment();
        }
        else if (srcView.PeekChar() == '/' && srcView.PeekChar(1) == '*') {
            klass = ParseBlockComment();
        }
        else if (preprocessor.ShouldLexHeaderName() && srcView.PeekChar() == '"') {
            klass = ParseQuotedString();
        }
        else if (preprocessor.ShouldLexHeaderName() && srcView.PeekChar() == '<') {
            // klass = ParseAngleString();
        }
        else {
            klass = detail::Tokenize(srcView, tokenTextBuffer);
        }

        LexString text      = compilerObject.GetLexContext().GetLexString(StringView{tokenTextBuffer});
        TextPosition endPos = srcView.GetTextPosition();

        currentToken = PPTokenData{
            .klass                = klass,
            .isFirstTokenOfLine   = skippedNewLine,
            .hasLeadingWhitespace = skippedWhitespace,
            .text                 = text,
            .range                = TextRange{beginPos, endPos},
        };
    }

    auto Tokenizer::ParseLineComment() -> TokenKlass
    {
        GLSLD_ASSERT(srcView.PeekChar() == '/' && srcView.PeekChar(1) == '/');

        // Consume "//"
        srcView.SkipChar(2);

        while (!srcView.CursorAtEnd()) {
            auto ch = srcView.PeekChar();

            if (ch != '\n') {
                srcView.ConsumeChar();
            }
            else {
                break;
            }
        }

        return TokenKlass::Comment;
    }

    auto Tokenizer::ParseBlockComment() -> TokenKlass
    {
        GLSLD_ASSERT(srcView.PeekChar() == '/' && srcView.PeekChar(1) == '*');

        // Consume "/*"
        srcView.SkipChar(2);

        while (!srcView.CursorAtEnd()) {
            auto ch = srcView.PeekChar();

            if (ch != '*' || srcView.PeekChar(1) != '/') {
                srcView.ConsumeChar();
            }
            else {
                // Consume "*/"
                srcView.SkipChar(2);
                return TokenKlass::Comment;
            }
        }

        return TokenKlass::Error;
    }

    auto Tokenizer::ParseQuotedString() -> TokenKlass
    {
        // FIXME: a quoted string shouldn't contain a newline

        GLSLD_ASSERT(srcView.PeekChar() == '"');

        // Consume '"'
        srcView.SkipChar(1);
        tokenTextBuffer.push_back('"');

        while (!srcView.CursorAtEnd()) {
            auto ch = srcView.PeekChar();
            tokenTextBuffer.push_back(ch);

            if (ch != '"') {
                srcView.ConsumeChar();
            }
            else {
                // Consume '"'
                srcView.SkipChar(1);
                return TokenKlass::QuotedString;
            }
        }

        return TokenKlass::Error;
    }

} // namespace glsld
#include "Tokenizer.h"
#include "Compiler.h"
#include "LexContext.h"
#include "Preprocessor.h"
#include "CompilerTrace.h"

namespace glsld::detail
{
    // Defined in generated file "Tokenize.generated.cpp"
    auto Tokenize(SourceScanner& srcView, std::vector<char>& buffer) -> TokenKlass;
} // namespace glsld::detail

namespace glsld
{
    Tokenizer::Tokenizer(CompilerObject& compilerObject, Preprocessor& preprocessor, FileID sourceFile,
                         StringView sourceText)
        : compilerObject(compilerObject), preprocessor(preprocessor), sourceFile(sourceFile)
    {
        // FIXME: expose countUtf16 as options
        srcScanner = SourceScanner{sourceText.data(), sourceText.data() + sourceText.Size(), true};
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

        // We handle comments and header names manually instead of using generated lexer. Those are only places where
        // unicode is allowed.
        if (srcScanner.TryConsumeAsciiText("//")) {
            klass = ParseLineComment();
        }
        else if (srcScanner.TryConsumeAsciiText("/*")) {
            klass = ParseBlockComment();
        }
        else if (preprocessor.ShouldLexHeaderName() && srcScanner.TryConsumeAsciiChar('"')) {
            klass = ParseQuotedString();
        }
        // else if (preprocessor.ShouldLexHeaderName() && srcScanner.TryConsumeAsciiChar('<')) {
        //     klass = ParseAngleString();
        // }
        else {
            klass = detail::Tokenize(srcScanner, tokenTextBuffer);
            if (klass == TokenKlass::Error) [[unlikely]] {
                // At least consume one character to avoid infinite loop
                srcScanner.ConsumeChar(tokenTextBuffer);
            }
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
        // Assuming "//" is already consumed
        while (!srcScanner.CursorAtEnd()) {
            if (srcScanner.PeekCodeUnit() == '\n') {
                break;
            }

            srcScanner.ConsumeChar();
        }

        return TokenKlass::Comment;
    }

    auto Tokenizer::ParseBlockComment() -> TokenKlass
    {
        // Assuming "/*" is already consumed
        while (!srcScanner.CursorAtEnd()) {
            if (srcScanner.TryConsumeAsciiText("*/")) {
                return TokenKlass::Comment;
            }

            srcScanner.ConsumeChar();
        }

        return TokenKlass::Error;
    }

    auto Tokenizer::ParseQuotedString() -> TokenKlass
    {
        // Assuming '"' is already consumed
        tokenTextBuffer.push_back('"');

        while (!srcScanner.CursorAtEnd()) {
            if (srcScanner.TryConsumeAsciiChar('"')) {
                tokenTextBuffer.push_back('"');
                return TokenKlass::QuotedString;
            }
            else {
                srcScanner.ConsumeChar(tokenTextBuffer);
            }
        }

        return TokenKlass::Error;
    }

    auto Tokenizer::ParseAngleString() -> TokenKlass
    {
        GLSLD_NO_IMPL();
    }

} // namespace glsld
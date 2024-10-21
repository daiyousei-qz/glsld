#include "Compiler/Tokenizer.h"
#include "Compiler/CompilerObject.h"
#include "Compiler/LexContext.h"
#include "Compiler/Preprocessor.h"
#include "Compiler/CompilerTrace.h"

namespace glsld::detail
{
    // Defined in generated file "Tokenize.generated.cpp"
    auto Tokenize(SourceScanner& srcView, std::vector<char>& buffer) -> TokenKlass;
} // namespace glsld::detail

namespace glsld
{
    Tokenizer::Tokenizer(CompilerObject& compilerObject, AtomTable& atomTable, Preprocessor& preprocessor,
                         FileID sourceFileId, bool countUtf16Characters)
        : compilerObject(compilerObject), atomTable(atomTable), preprocessor(preprocessor), sourceFileId(sourceFileId)
    {
        auto sourceText = compilerObject.GetSourceContext().GetSourceText(sourceFileId);
        srcScanner      = SourceScanner{sourceText.data(), sourceText.data() + sourceText.Size(), countUtf16Characters};
    }

    auto Tokenizer::DoTokenize() -> void
    {
        GLSLD_ASSERT(preprocessor.GetState() == PreprocessorState::Default);
        std::vector<PPToken> ppLineBuffer;
        while (!preprocessor.ShouldHaltLexing()) {
            PPToken token = LexPPToken();

            if (token.klass == TokenKlass::Eof) {
                preprocessor.IssuePPToken(token);
                break;
            }

            if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
                preprocessor.IssuePPToken(token);
            }

            if (preprocessor.InActiveRegion()) {
                GLSLD_TRACE_TOKEN_LEXED(token);
                preprocessor.IssuePPToken(token);
            }
            else {
                // FIXME: We are in an inactive branch of preprocessor. Fast scan for # instead of expensive lexing.
                if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
                    GLSLD_TRACE_TOKEN_LEXED(token);
                    preprocessor.IssuePPToken(token);
                }
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
                .spelledFile          = sourceFileId,
                .spelledRange         = TextRange{srcScanner.GetTextPosition()},
                .text                 = {},
                .isFirstTokenOfLine   = true,
                .hasLeadingWhitespace = true,
            };
        }

        tokenTextBuffer.clear();
        TokenKlass klass      = TokenKlass::Unknown;
        TextPosition beginPos = srcScanner.GetTextPosition();

        // We handle comments and header names manually instead of using generated lexer. Those are only places where
        // unicode is allowed.
        // TODO: also handle comment text for better intellisesnse feature
        if (srcScanner.TryConsumeAsciiText("//")) {
            klass = ParseLineComment();
        }
        else if (srcScanner.TryConsumeAsciiText("/*")) {
            klass = ParseBlockComment();
        }
        else if (preprocessor.ShouldLexHeaderName() && srcScanner.TryConsumeAsciiChar('"')) {
            klass = ParseHeaderName('"', '"', TokenKlass::UserHeaderName);
        }
        else if (preprocessor.ShouldLexHeaderName() && srcScanner.TryConsumeAsciiChar('<')) {
            klass = ParseHeaderName('<', '>', TokenKlass::SystemHeaderName);
        }
        else {
            klass = detail::Tokenize(srcScanner, tokenTextBuffer);
            if (klass == TokenKlass::Unknown) [[unlikely]] {
                // At least consume one character to avoid infinite loop
                srcScanner.ConsumeChar(tokenTextBuffer);
            }
        }

        AtomString text     = atomTable.GetAtom(StringView{tokenTextBuffer});
        TextPosition endPos = srcScanner.GetTextPosition();

        return PPToken{
            .klass                = klass,
            .spelledFile          = sourceFileId,
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

        return TokenKlass::Unknown;
    }

    auto Tokenizer::ParseHeaderName(char quoteStart, char quoteEnd, TokenKlass klass) -> TokenKlass
    {
        // Assuming `quoteStart` is already consumed
        tokenTextBuffer.push_back(quoteStart);

        while (!srcScanner.CursorAtEnd()) {
            if (srcScanner.PeekCodeUnit() == '\n') {
                break;
            }
            if (srcScanner.TryConsumeAsciiChar(quoteEnd)) {
                tokenTextBuffer.push_back(quoteEnd);
                return klass;
            }
            else {
                srcScanner.ConsumeChar(tokenTextBuffer);
            }
        }

        return TokenKlass::Unknown;
    }

} // namespace glsld
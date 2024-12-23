#include "Compiler/Tokenizer.h"
#include "Basic/StringView.h"
#include "Compiler/Preprocessor.h"
#include "Compiler/CompilerTrace.h"

namespace glsld::detail
{
    // Defined in generated file "Tokenize.generated.cpp"
    auto Tokenize(SourceScanner& srcView, std::vector<char>& buffer) -> TokenKlass;
} // namespace glsld::detail

namespace glsld
{
    auto Tokenizer::DoTokenize() -> void
    {
        GLSLD_ASSERT(pp.GetState() == PreprocessorState::Default);
        std::vector<PPToken> ppLineBuffer;
        while (!pp.ShouldHaltLexing()) {
            PPToken token = LexPPToken();
#if defined(GLSLD_DEBUG)
            compiler.GetCompilerTrace().TracePPTokenLexed(token);
#endif

            if (token.klass == TokenKlass::Eof) {
                pp.FeedPPToken(token);
                break;
            }

            if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
                pp.FeedPPToken(token);
            }

            if (pp.InActiveRegion()) {
                pp.FeedPPToken(token);
            }
            else {
                // FIXME: We are in an inactive branch of preprocessor. Fast scan for # instead of expensive lexing.
                if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
                    pp.FeedPPToken(token);
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
                .spelledFile          = sourceFile,
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
        else if (pp.ShouldLexHeaderName() && srcScanner.TryConsumeAsciiChar('"')) {
            klass = ParseHeaderName('"', '"', TokenKlass::UserHeaderName);
        }
        else if (pp.ShouldLexHeaderName() && srcScanner.TryConsumeAsciiChar('<')) {
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
        // FIXME: add a toggle
        tokenTextBuffer.push_back('/');
        tokenTextBuffer.push_back('/');

        while (!srcScanner.CursorAtEnd()) {
            if (srcScanner.PeekCodeUnit() == '\n') {
                tokenTextBuffer.push_back('\n');
                break;
            }

            srcScanner.ConsumeChar(tokenTextBuffer);
        }

        return TokenKlass::Comment;
    }

    auto Tokenizer::ParseBlockComment() -> TokenKlass
    {
        // Assuming "/*" is already consumed
        // FIXME: add a toggle
        tokenTextBuffer.push_back('/');
        tokenTextBuffer.push_back('*');

        while (!srcScanner.CursorAtEnd()) {
            if (srcScanner.TryConsumeAsciiText("*/")) {
                tokenTextBuffer.push_back('*');
                tokenTextBuffer.push_back('/');
                return TokenKlass::Comment;
            }

            srcScanner.ConsumeChar(tokenTextBuffer);
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

    auto TokenizeOnce(StringView text) -> std::tuple<TokenKlass, StringView, StringView>
    {
        std::vector<char> tokenTextBuffer;
        tokenTextBuffer.reserve(text.Size());

        SourceScanner srcScanner{text, false};
        TokenKlass klass = detail::Tokenize(srcScanner, tokenTextBuffer);
        return {klass, srcScanner.GetScannedText(), srcScanner.GetRemainingText()};
    }

} // namespace glsld
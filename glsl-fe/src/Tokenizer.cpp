#include "Tokenizer.h"
#include "Tokenize.Generated.inc"

namespace glsld
{
    auto Tokenizer::NextToken(std::string& buf) -> TokenInfo
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

        char firstChar   = sourceView.Peek();
        TokenKlass klass = TokenKlass::Error;
        if (firstChar == '\0') {
            // eof
            sourceView.Consume();
            klass = TokenKlass::Eof;
        }
        else if (firstChar == '#') {
            // FIXME: we treat PP as comment for now
            // preprocessor
            sourceView.Consume();
            klass = ParsePPComment();
        }
        else if (firstChar == '/' && sourceView.Peek(0) == '/') {
            // line comment
            sourceView.Consume();
            klass = ParseLineComment();
        }
        else if (firstChar == '/' && sourceView.Peek(0) == '*') {
            // block comment
            sourceView.Consume();
            klass = ParseBlockComment();
        }
        else {
            auto remainingSoureceView = std::string_view{sourceView.cursor, sourceView.end};
            auto tokenizeResult       = Tokenize(remainingSoureceView);
            if (tokenizeResult.numAcceptedChar != 0 && tokenizeResult.acceptedKlass != -1) {
                klass = static_cast<TokenKlass>(tokenizeResult.acceptedKlass);
                for (int i = 0; i < tokenizeResult.numAcceptedChar; ++i) {
                    sourceView.Consume();
                }

                if (klass == TokenKlass::Identifier) {
                    static std::unordered_map<std::string_view, TokenKlass> keywordLookup = {
#define DECL_KEYWORD(KEYWORD) {#KEYWORD, TokenKlass::K_##KEYWORD},
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
                    };
                    if (auto it = keywordLookup.find(remainingSoureceView.substr(0, tokenizeResult.numAcceptedChar));
                        it != keywordLookup.end()) {
                        klass = it->second;
                    }
                }
            }
            else {
                // consume a bad char to ensure we are proceeding
                sourceView.Consume();
            }
        }
        // else if (IsAlpha(firstChar) || firstChar == '_') {
        //     // keyword or identifier
        //     sourceView.Consume();
        //     klass = ParseIdentifier(firstChar);
        // }
        // else if (IsDigit(firstChar)) {
        //     // number literal
        //     sourceView.Consume();
        //     klass = ParseNumberLiteral(firstChar);
        // }
        // else if (firstChar == '.' && IsDigit(sourceView.Peek(0))) {
        //     // number literal
        //     sourceView.Consume();
        //     klass = ParseNumberLiteral(firstChar);
        // }
        // else {
        //     // punctuation
        //     sourceView.Consume();
        //     klass = ParsePunctuation(firstChar);
        // }

        auto endCursor = sourceView.CurrentCursor();
        result.klass   = klass;
        result.rawSize = endCursor - beginCursor;
        // FIXME: this is actually one character ahead of the token
        result.lineEnd   = sourceView.GetLineNum();
        result.columnEnd = sourceView.GetColumnNum();

        buf.append(beginCursor, endCursor);
        return result;
    }
} // namespace glsld
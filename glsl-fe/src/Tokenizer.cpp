#include "Tokenizer.h"
#include "Tokenize.Generated.inc"

namespace glsld
{
    // FIXME: unicode?
    static auto IsWhitespaceChar(char ch) -> bool
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

    auto Tokenizer::NextToken(std::string* buf) -> TokenInfo
    {
        while (true) {
            SkipWhitespace();
            if (Eof()) {
                break;
            }

            // We have at least one character in the buffer here
            GLSLD_ASSERT(bufferIndex < lineBuffer.size());

            if (freshLine && lineBuffer[bufferIndex] == '#') {
                // FIXME: support preprocessor
                // PP derivative, skip the line for now
                bufferIndex = lineBuffer.size();
                continue;
            }
            freshLine = false;

            auto tokBegin    = GetCurrentTextPosition();
            TokenKlass klass = TokenKlass::Error;
            if (PeekChar() == '/' && PeekChar(1) == '/') {
                // Parse line comment
                klass = ParseLineComment(buf);
            }
            else if (PeekChar() == '/' && PeekChar(1) == '*') {
                // Parse block comment
                klass = ParseBlockComment(buf);
            }
            else {
                // Parse normal token
                klass = ParseRegularToken(buf);
            }

            return TokenInfo{
                .klass = klass,
                .begin = tokBegin,
                .end   = GetCurrentTextPosition(),
            };
        }

        auto eofPosition = TextPosition{.line = static_cast<int>(lineIndex), .character = 0};
        return TokenInfo{.klass = TokenKlass::Eof, .begin = eofPosition, .end = eofPosition};
    }

    auto Tokenizer::GetCurrentTextPosition() -> TextPosition
    {
        size_t line      = lineIndex;
        size_t character = bufferIndex;
        for (auto lineContinueIndex : lineBreakBuffer) {
            if (bufferIndex >= lineContinueIndex) {
                line += 1;
                character = bufferIndex - lineContinueIndex;
            }
        }

        return TextPosition{.line = static_cast<int>(line), .character = static_cast<int>(character)};
    }

    auto Tokenizer::ParseLineComment(std::string* buf) -> TokenKlass
    {
        GLSLD_ASSERT(PeekChar() == '/' && PeekChar(1) == '/');
        if (buf) {
            *buf += StringView{lineBuffer.begin() + bufferIndex, lineBuffer.end() - 1}.StdStrView();
        }

        bufferIndex = lineBuffer.size();
        return TokenKlass::Comment;
    }

    auto Tokenizer::ParseBlockComment(std::string* buf) -> TokenKlass
    {
        GLSLD_ASSERT(PeekChar() == '/' && PeekChar(1) == '*');
        if (buf) {
            *buf += "/*";
        }
        bufferIndex += 2;

        while (!Eof()) {
            while (bufferIndex + 1 < lineBuffer.size()) {
                if (PeekChar() == '*' && PeekChar(1) == '/') {
                    if (buf) {
                        *buf += "*/";
                    }
                    bufferIndex += 2;
                    return TokenKlass::Comment;
                }
                else {
                    if (buf) {
                        *buf += PeekChar();
                    }
                    bufferIndex += 1;
                }
            }

            if (buf) {
                *buf += "\n";
            }
            LoadLine();
        }

        return TokenKlass::Error;
    }

    auto Tokenizer::ParseRegularToken(std::string* buf) -> TokenKlass
    {
        TokenKlass result = TokenKlass::Error;

        auto remainingSource = StringView{lineBuffer.begin() + bufferIndex, lineBuffer.end()};
        auto tokenizeResult  = Tokenize(remainingSource.StdStrView());
        if (tokenizeResult.numAcceptedChar != 0 && tokenizeResult.acceptedKlass != -1) {
            bufferIndex += tokenizeResult.numAcceptedChar;

            auto tokenView = remainingSource.Take(tokenizeResult.numAcceptedChar);
            if (buf) {
                *buf += tokenView.StdStrView();
            }

            result = static_cast<TokenKlass>(tokenizeResult.acceptedKlass);
            if (result == TokenKlass::Identifier) {
                static std::unordered_map<std::string_view, TokenKlass> keywordLookup = {
#define DECL_KEYWORD(KEYWORD) {#KEYWORD, TokenKlass::K_##KEYWORD},
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
                };
                if (auto it = keywordLookup.find(tokenView.StdStrView()); it != keywordLookup.end()) {
                    result = it->second;
                }
            }
        }
        else {
            // No accepted char, we consume one bad char to ensure proceeding
            bufferIndex += 1;

            if (buf) {
                *buf += remainingSource.Take(1).StdStrView();
            }
        }

        return result;
    }

    auto Tokenizer::PeekChar() -> char
    {
        if (bufferIndex < lineBuffer.size()) {
            return lineBuffer[bufferIndex];
        }
        else {
            return '\0';
        }
    }
    auto Tokenizer::PeekChar(size_t lookahead) -> char
    {
        if (bufferIndex + lookahead < lineBuffer.size()) {
            return lineBuffer[bufferIndex + lookahead];
        }
        else {
            return '\0';
        }
    }

    auto Tokenizer::SkipWhitespace() -> void
    {
        while (!Eof()) {
            while (bufferIndex < lineBuffer.size()) {
                if (IsWhitespaceChar(lineBuffer[bufferIndex])) {
                    ++bufferIndex;
                }
                else {
                    break;
                }
            }

            if (bufferIndex < lineBuffer.size()) {
                break;
            }
            else {
                LoadLine();
            }
        }
    }

    auto Tokenizer::LoadLine() -> void
    {
        freshLine = true;
        lineIndex += lineBreakBuffer.size() + 1;
        bufferIndex = 0;

        lineBuffer.clear();
        lineBreakBuffer.clear();

        while (true) {
            auto remainingSource = StringView{lineCursor, srcEnd};
            if (remainingSource.Empty()) {
                break;
            }

            auto [firstLine, restLines] = remainingSource.Split('\n');

            std::ranges::copy(firstLine, std::back_inserter(lineBuffer));
            lineCursor = restLines.Data();

            if (!lineBuffer.empty() && lineBuffer.back() == '\\') {
                lineBuffer.pop_back();
                lineBreakBuffer.push_back(lineBuffer.size());
            }
            else {
                // Append a endline to make character tracking easier
                lineBuffer.push_back('\n');
                break;
            }
        }
    }
} // namespace glsld
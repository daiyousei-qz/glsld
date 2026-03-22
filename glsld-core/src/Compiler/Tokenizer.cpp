#include "Compiler/Tokenizer.h"
#include "Support/StringView.h"

namespace glsld
{
    static auto IsAscii(char ch) noexcept -> bool
    {
        return (ch & 0x80) == 0;
    }

    static auto IsAlpha(char ch) noexcept -> bool
    {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    static auto IsDigit(char ch) noexcept -> bool
    {
        return ch >= '0' && ch <= '9';
    }

    auto Tokenizer::TryConsumeLineContinuation() -> bool
    {
        bool consumed = false;
        while (srcCursor[0] == '\\') {
            if (srcCursor[1] == '\n') {
                srcCursor += 2;
                ++lineCounter;
                characterCounter = 0;
                consumed         = true;
            }
            else if (srcCursor[1] == '\r' && srcCursor[2] == '\n') {
                srcCursor += 3;
                ++lineCounter;
                characterCounter = 0;
                consumed         = true;
            }
            else {
                break;
            }
        }

        return consumed;
    }

    auto Tokenizer::TryConsumeWhitespace(bool& skippedWhitespace, bool& skippedNewline) -> void
    {
        while (true) {
            auto ch = *srcCursor;

            if (ch == ' ' || ch == '\t' || ch == '\r') {
                ++characterCounter;

                skippedWhitespace = true;
                ++srcCursor;
            }
            else if (ch == '\n') {
                ++lineCounter;
                characterCounter = 0;

                skippedWhitespace = true;
                skippedNewline    = true;
                ++srcCursor;
            }
            else if (!TryConsumeLineContinuation()) {
                break;
            }
        }
    }

    auto Tokenizer::ConsumeChar(TextPosition& endPos) -> StringView
    {
        StringView result;
        if (*srcCursor == '\0') {
            result = StringView{srcCursor, 1};
        }
        else if (*srcCursor == '\n') {
            result = StringView{srcCursor, 1};
            ++srcCursor;
            characterCounter = 0;
            ++lineCounter;
        }
        else {
            result = ConsumeCharUnsafe(endPos);
        }

        endPos = GetTextPosition();
        TryConsumeLineContinuation();
        GLSLD_ASSERT(!result.Empty());
        return result;
    }

    auto Tokenizer::ConsumeCharUnsafe(TextPosition& endPos) -> StringView
    {
        GLSLD_ASSERT(*srcCursor != '\n' && *srcCursor != '\0');

        const char firstCodeUnit = *srcCursor;

        StringView result;
        if (IsAscii(firstCodeUnit)) [[likely]] {
            // Fast path for ascii characters
            result = StringView{srcCursor, 1};

            ++srcCursor;
            characterCounter += 1;
        }
        else [[unlikely]] {
            // Slow path for non-ascii characters. We don't validate the encoding for better performance.
            int numCodeUnits = std::min(std::countl_one(static_cast<unsigned char>(firstCodeUnit)),
                                        static_cast<int>(srcEnd - srcCursor));
            characterCounter += ComputeLspCodeUnitNum(numCodeUnits);
            result = StringView{srcCursor, static_cast<size_t>(numCodeUnits)};
            srcCursor += numCodeUnits;
        }

        endPos = GetTextPosition();
        TryConsumeLineContinuation();
        GLSLD_ASSERT(!result.Empty());
        return result;
    }

    auto Tokenizer::ConsumeAsciiCharUnsafe(TextPosition& endPos) -> char
    {
        GLSLD_ASSERT(IsAscii(*srcCursor) && *srcCursor != '\n' && *srcCursor != '\0');

        const char result = *srcCursor;
        ++srcCursor;
        ++characterCounter;

        endPos = GetTextPosition();
        TryConsumeLineContinuation();
        return result;
    };

    auto Tokenizer::TryConsumeAsciiChar(TextPosition& endPos, char ch) -> bool
    {
        GLSLD_ASSERT(ch != '\n' && ch != '\0');

        if (*srcCursor == ch) {
            ConsumeAsciiCharUnsafe(endPos);
            return true;
        }

        return false;
    }

    auto Tokenizer::LexIdentifier(TextPosition& endPos, char firstChar) -> std::tuple<TokenKlass, AtomString>
    {
        GLSLD_ASSERT(firstChar == '_' || IsAlpha(firstChar));
        tokenTextBuffer = {firstChar};

        while (true) {
            const char nextChar = PeekCodeUnit();
            if (nextChar == '_' || IsAlpha(nextChar) || IsDigit(nextChar)) {
                ConsumeAsciiCharUnsafe(endPos);
                tokenTextBuffer.push_back(nextChar);
            }
            else {
                break;
            }
        }

        return {TokenKlass::Identifier, ExtractTokenText()};
    }
    auto Tokenizer::LexNumberLiteral(TextPosition& endPos, char firstChar) -> std::tuple<TokenKlass, AtomString>
    {
        GLSLD_ASSERT(IsDigit(firstChar) || (firstChar == '.' && IsDigit(PeekCodeUnit())));
        tokenTextBuffer = {firstChar};

        bool seenHexPrefix = false;
        bool seenDot       = (firstChar == '.');
        bool seenExp       = false;

        // Check if we have a hex prefix "0x" or "0X". This affects if we see e/E as exponent or just part of hex
        // literal.
        if (firstChar == '0') {
            if (PeekCodeUnit() == 'x' || PeekCodeUnit() == 'X') {
                tokenTextBuffer.push_back(ConsumeAsciiCharUnsafe(endPos));
                seenHexPrefix = true;
            }
        }

        // This is more permissive than the spec requires. But this allows us to lex invalid literals and report
        // diagnostics in a more user-friendly way, instead of just giving up at the first invalid token.
        while (true) {
            const char nextChar = PeekCodeUnit();
            if (nextChar == '.') {
                if (seenDot || seenExp) {
                    break;
                }
                seenDot = true;
                ConsumeAsciiCharUnsafe(endPos);
                tokenTextBuffer.push_back(nextChar);
            }
            else if (!seenHexPrefix && (nextChar == 'e' || nextChar == 'E')) {
                if (seenExp) {
                    break;
                }
                seenExp = true;
                ConsumeAsciiCharUnsafe(endPos);
                tokenTextBuffer.push_back(nextChar);

                if (PeekCodeUnit() == '+' || PeekCodeUnit() == '-') {
                    tokenTextBuffer.push_back(ConsumeAsciiCharUnsafe(endPos));
                }
            }
            else if (IsDigit(nextChar) || IsAlpha(nextChar)) {
                tokenTextBuffer.push_back(nextChar);
                ConsumeAsciiCharUnsafe(endPos);
            }
            else {
                break;
            }
        }

        return {TokenKlass::NumberLiteral, ExtractTokenText()};
    }
    auto Tokenizer::LexLineComment(TextPosition& endPos) -> std::tuple<TokenKlass, AtomString>
    {
        // Assuming "//" is already consumed
        // FIXME: add a toggle
        tokenTextBuffer = {'/', '/'};

        while (true) {
            if (PeekCodeUnit() == '\0' || PeekCodeUnit() == '\n') {
                tokenTextBuffer.push_back('\n');
                break;
            }
            else {
                const StringView nextChar = ConsumeCharUnsafe(endPos);
                tokenTextBuffer.insert(tokenTextBuffer.end(), nextChar.begin(), nextChar.end());
            }
        }

        return {TokenKlass::Comment, ExtractTokenText()};
    }
    auto Tokenizer::LexBlockComment(TextPosition& endPos) -> std::tuple<TokenKlass, AtomString>
    {
        // Assuming "/*" is already consumed
        // FIXME: add a toggle
        tokenTextBuffer = {'/', '*'};

        bool prevWasStar = false;
        while (true) {
            const StringView nextChar = ConsumeChar(endPos);
            if (nextChar[0] == '\0') {
                // Unexpected EOF in block comment.
                break;
            }
            else if (nextChar[0] == '*') {
                prevWasStar = true;
            }
            else {
                if (prevWasStar && nextChar[0] == '/') {
                    tokenTextBuffer.push_back('/');
                    return {TokenKlass::Comment, ExtractTokenText()};
                }
                prevWasStar = false;
            }

            tokenTextBuffer.insert(tokenTextBuffer.end(), nextChar.begin(), nextChar.end());
        }

        // aka. unterminated block comment
        return {TokenKlass::Unknown, ExtractTokenText()};
    }
    auto Tokenizer::LexHeaderName(TextPosition& endPos, char quoteStart, char quoteEnd, TokenKlass klass)
        -> std::tuple<TokenKlass, AtomString>
    {
        // Assuming `quoteStart` is already consumed
        tokenTextBuffer = {quoteStart};

        while (true) {
            if (PeekCodeUnit() == '\0' || PeekCodeUnit() == '\n') {
                break;
            }
            else if (TryConsumeAsciiChar(endPos, quoteEnd)) {
                tokenTextBuffer.push_back(quoteEnd);
                return {klass, ExtractTokenText()};
            }
            else {
                const StringView nextChar = ConsumeCharUnsafe(endPos);
                tokenTextBuffer.insert(tokenTextBuffer.end(), nextChar.begin(), nextChar.end());
            }
        }

        return {TokenKlass::Unknown, ExtractTokenText()};
    }
    auto Tokenizer::Lex() -> PPToken
    {
        bool skippedWhitespace = false;
        bool skippedNewLine    = srcCursor == srcBegin;
        TryConsumeWhitespace(skippedWhitespace, skippedNewLine);

        if (Exhausted()) {
            // NOTE we always regard an EOF in its own line
            return PPToken{
                .klass                = TokenKlass::Eof,
                .spelledFile          = sourceFile,
                .spelledRange         = TextRange{GetTextPosition()},
                .text                 = {},
                .isFirstTokenOfLine   = true,
                .hasLeadingWhitespace = true,
            };
        }

        TextPosition beginPos = GetTextPosition();
        TextPosition endPos;

        StringView firstChar = ConsumeCharUnsafe(endPos);
        GLSLD_ASSERT(!firstChar.Empty());

        TokenKlass klass = TokenKlass::Unknown;
        AtomString text;
        switch (firstChar[0]) {
        case '!':
        {
            if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::NotEqual;
                text  = pp.GetLanguageAtoms().puncts.p_NotEqual;
            }
            else {
                klass = TokenKlass::Bang;
                text  = pp.GetLanguageAtoms().puncts.p_Bang;
            }
            break;
        }
        case '"':
        {
            if (pp.ShouldLexHeaderName()) {
                std::tie(klass, text) = LexHeaderName(endPos, '"', '"', TokenKlass::UserHeaderName);
            }
            else {
                goto LABEL_LEX_UNKNOWN_TOKEN;
            }
            break;
        }
        case '#':
        {
            if (TryConsumeAsciiChar(endPos, '#')) {
                klass = TokenKlass::HashHash;
                text  = pp.GetLanguageAtoms().miscs.ppHashHash;
            }
            else {
                klass = TokenKlass::Hash;
                text  = pp.GetLanguageAtoms().miscs.ppHash;
            }
            break;
        }
        case '%':
        {
            if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::ModAssign;
                text  = pp.GetLanguageAtoms().puncts.p_ModAssign;
            }
            else {
                klass = TokenKlass::Percent;
                text  = pp.GetLanguageAtoms().puncts.p_Percent;
            }
            break;
        }
        case '&':
        {
            if (TryConsumeAsciiChar(endPos, '&')) {
                klass = TokenKlass::And;
                text  = pp.GetLanguageAtoms().puncts.p_And;
            }
            else if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::AndAssign;
                text  = pp.GetLanguageAtoms().puncts.p_AndAssign;
            }
            else {
                klass = TokenKlass::Ampersand;
                text  = pp.GetLanguageAtoms().puncts.p_Ampersand;
            }
            break;
        }
        case '(':
        {
            klass = TokenKlass::LParen;
            text  = pp.GetLanguageAtoms().puncts.p_LParen;
            break;
        }
        case ')':
        {
            klass = TokenKlass::RParen;
            text  = pp.GetLanguageAtoms().puncts.p_RParen;
            break;
        }
        case '*':
        {
            if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::MulAssign;
                text  = pp.GetLanguageAtoms().puncts.p_MulAssign;
            }
            else {
                klass = TokenKlass::Star;
                text  = pp.GetLanguageAtoms().puncts.p_Star;
            }
            break;
        }
        case '+':
        {
            if (TryConsumeAsciiChar(endPos, '+')) {
                klass = TokenKlass::Increment;
                text  = pp.GetLanguageAtoms().puncts.p_Increment;
            }
            else if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::AddAssign;
                text  = pp.GetLanguageAtoms().puncts.p_AddAssign;
            }
            else {
                klass = TokenKlass::Plus;
                text  = pp.GetLanguageAtoms().puncts.p_Plus;
            }
            break;
        }
        case ',':
        {
            klass = TokenKlass::Comma;
            text  = pp.GetLanguageAtoms().puncts.p_Comma;
            break;
        }
        case '-':
        {
            if (TryConsumeAsciiChar(endPos, '-')) {
                klass = TokenKlass::Decrement;
                text  = pp.GetLanguageAtoms().puncts.p_Decrement;
            }
            else if (TryConsumeAsciiChar(endPos, '=')) {

                klass = TokenKlass::SubAssign;
                text  = pp.GetLanguageAtoms().puncts.p_SubAssign;
            }
            else {
                klass = TokenKlass::Dash;
                text  = pp.GetLanguageAtoms().puncts.p_Dash;
            }
            break;
        }
        case '.':
        {
            if (IsDigit(PeekCodeUnit())) {
                std::tie(klass, text) = LexNumberLiteral(endPos, firstChar[0]);
            }
            else {
                klass = TokenKlass::Dot;
                text  = pp.GetLanguageAtoms().puncts.p_Dot;
            }
            break;
        }
        case '/':
        {
            if (TryConsumeAsciiChar(endPos, '/')) {
                std::tie(klass, text) = LexLineComment(endPos);
            }
            else if (TryConsumeAsciiChar(endPos, '*')) {
                std::tie(klass, text) = LexBlockComment(endPos);
            }
            else if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::DivAssign;
                text  = pp.GetLanguageAtoms().puncts.p_DivAssign;
            }
            else {
                klass = TokenKlass::Slash;
                text  = pp.GetLanguageAtoms().puncts.p_Slash;
            }
            break;
        }
        case ':':
        {
            klass = TokenKlass::Colon;
            text  = pp.GetLanguageAtoms().puncts.p_Colon;
            break;
        }
        case ';':
        {
            klass = TokenKlass::Semicolon;
            text  = pp.GetLanguageAtoms().puncts.p_Semicolon;
            break;
        }
        case '<':
        {
            if (pp.ShouldLexHeaderName()) {
                std::tie(klass, text) = LexHeaderName(endPos, '<', '>', TokenKlass::SystemHeaderName);
            }
            else if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::LessEq;
                text  = pp.GetLanguageAtoms().puncts.p_LessEq;
            }
            else if (TryConsumeAsciiChar(endPos, '<')) {
                if (TryConsumeAsciiChar(endPos, '=')) {
                    klass = TokenKlass::LShiftAssign;
                    text  = pp.GetLanguageAtoms().puncts.p_LShiftAssign;
                }
                else {
                    klass = TokenKlass::LShift;
                    text  = pp.GetLanguageAtoms().puncts.p_LShift;
                }
            }
            else {
                klass = TokenKlass::LAngle;
                text  = pp.GetLanguageAtoms().puncts.p_LAngle;
            }
            break;
        }
        case '=':
        {
            if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::Equal;
                text  = pp.GetLanguageAtoms().puncts.p_Equal;
            }
            else {
                klass = TokenKlass::Assign;
                text  = pp.GetLanguageAtoms().puncts.p_Assign;
            }
            break;
        }
        case '>':
        {
            if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::GreaterEq;
                text  = pp.GetLanguageAtoms().puncts.p_GreaterEq;
            }
            else if (TryConsumeAsciiChar(endPos, '>')) {
                if (TryConsumeAsciiChar(endPos, '=')) {
                    klass = TokenKlass::RShiftAssign;
                    text  = pp.GetLanguageAtoms().puncts.p_RShiftAssign;
                }
                else {
                    klass = TokenKlass::RShift;
                    text  = pp.GetLanguageAtoms().puncts.p_RShift;
                }
            }
            else {
                klass = TokenKlass::RAngle;
                text  = pp.GetLanguageAtoms().puncts.p_RAngle;
            }
            break;
        }
        case '?':
        {
            klass = TokenKlass::Question;
            text  = pp.GetLanguageAtoms().puncts.p_Question;
            break;
        }
        case '[':
        {
            klass = TokenKlass::LBracket;
            text  = pp.GetLanguageAtoms().puncts.p_LBracket;
            break;
        }
        case ']':
        {
            klass = TokenKlass::RBracket;
            text  = pp.GetLanguageAtoms().puncts.p_RBracket;
            break;
        }
        case '^':
        {
            if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::XorAssign;
                text  = pp.GetLanguageAtoms().puncts.p_XorAssign;
            }
            else if (TryConsumeAsciiChar(endPos, '^')) {
                klass = TokenKlass::Xor;
                text  = pp.GetLanguageAtoms().puncts.p_Xor;
            }
            else {
                klass = TokenKlass::Caret;
                text  = pp.GetLanguageAtoms().puncts.p_Caret;
            }
            break;
        }
        case '{':
        {
            klass = TokenKlass::LBrace;
            text  = pp.GetLanguageAtoms().puncts.p_LBrace;
            break;
        }
        case '|':
        {
            if (TryConsumeAsciiChar(endPos, '=')) {
                klass = TokenKlass::OrAssign;
                text  = pp.GetLanguageAtoms().puncts.p_OrAssign;
            }
            else if (TryConsumeAsciiChar(endPos, '|')) {
                klass = TokenKlass::Or;
                text  = pp.GetLanguageAtoms().puncts.p_Or;
            }
            else {
                klass = TokenKlass::VerticalBar;
                text  = pp.GetLanguageAtoms().puncts.p_VerticalBar;
            }
            break;
        }
        case '}':
        {
            klass = TokenKlass::RBrace;
            text  = pp.GetLanguageAtoms().puncts.p_RBrace;
            break;
        }
        case '~':
        {
            klass = TokenKlass::Tilde;
            text  = pp.GetLanguageAtoms().puncts.p_Tilde;
            break;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            std::tie(klass, text) = LexNumberLiteral(endPos, firstChar[0]);
            break;
        }
            // clang-format off
        case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            // clang-format on
            {
                std::tie(klass, text) = LexIdentifier(endPos, firstChar[0]);
                break;
            }
        default:
        LABEL_LEX_UNKNOWN_TOKEN:
            klass = TokenKlass::Unknown;
            text  = pp.GetAtomTable().GetAtom(firstChar);
            break;
        }

        return PPToken{
            .klass                = klass,
            .spelledFile          = sourceFile,
            .spelledRange         = TextRange{beginPos, endPos},
            .text                 = text,
            .isFirstTokenOfLine   = skippedNewLine,
            .hasLeadingWhitespace = skippedWhitespace,
        };
    }

} // namespace glsld
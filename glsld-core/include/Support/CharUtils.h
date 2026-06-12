#pragma once
#include "Basic/Common.h"

namespace glsld
{
    inline constexpr auto IsAscii(char ch) noexcept -> bool
    {
        return (ch & 0x80) == 0;
    }

    inline constexpr auto IsAlpha(char ch) noexcept -> bool
    {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    inline constexpr auto IsDigit(char ch) noexcept -> bool
    {
        return ch >= '0' && ch <= '9';
    }

    inline constexpr auto IsAlnum(char ch) noexcept -> bool
    {
        return IsAlpha(ch) || IsDigit(ch);
    }

    inline constexpr auto IsHexDigit(char ch) noexcept -> bool
    {
        return IsDigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
    }

    inline constexpr auto IsWhitespace(char ch) noexcept -> bool
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    inline constexpr auto ToLower(char ch) noexcept -> char
    {
        if (ch >= 'A' && ch <= 'Z') {
            return ch - 'A' + 'a';
        }
        return ch;
    }

    inline constexpr auto ToUpper(char ch) noexcept -> char
    {
        if (ch >= 'a' && ch <= 'z') {
            return ch - 'a' + 'A';
        }
        return ch;
    }

    inline constexpr auto HexDigitToInt(char ch) noexcept -> int
    {
        GLSLD_ASSERT(IsHexDigit(ch));
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' + 10;
        }

        return ch - 'A' + 10;
    }
} // namespace glsld
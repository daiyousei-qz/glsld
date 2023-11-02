#pragma once
#include "Basic/Common.h"

#include <string>
#include <optional>

namespace glsld
{
    class Uri
    {
    public:
        Uri() = default;

        auto GetScheme() const -> StringView
        {
            return "file";
        }

        auto GetAuthority() const -> StringView
        {
            return "";
        }

        auto GetPath() const -> StringView
        {
            return path;
        }

        auto ToString() const -> std::string
        {
#if defined(GLSLD_OS_WIN)
            return "file:///" + path;
#else
            return "file://" + path;
#endif
        }

        static auto FromString(StringView uri) -> std::optional<Uri>
        {
#if defined(GLSLD_OS_WIN)
            if (!uri.StartWith("file:///")) {
                return std::nullopt;
            }
            uri = uri.Drop(8);
#else
            if (!uri.StartWith("file://")) {
                return std::nullopt;
            }
            uri = uri.Drop(7);
#endif

            Uri result;
            result.path = uri.Str();
            return result;
        }

    private:
        // std::string scheme;
        // std::string authority;
        std::string path;
    };

    inline auto UnescapeHttp(StringView httpStr) -> std::string
    {
        constexpr auto isHexDigit = [](char c) -> bool {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        };
        constexpr auto hexDigitToInt = [](char c) -> int {
            if (c >= '0' && c <= '9') {
                return c - '0';
            }
            else if (c >= 'a' && c <= 'f') {
                return c - 'a' + 10;
            }
            else if (c >= 'A' && c <= 'F') {
                return c - 'A' + 10;
            }
            else {
                GLSLD_UNREACHABLE();
            }
        };

        std::string result;
        for (StringView s = httpStr; !s.Empty();) {
            if (s.Size() >= 3 && s[0] == '%' && isHexDigit(s[1]) && isHexDigit(s[2])) {
                result += static_cast<char>(hexDigitToInt(s[1]) * 16 + hexDigitToInt(s[2]));
                s = s.Drop(3);
            }
            else {
                result += s[0];
                s = s.Drop(1);
            }
        }

        return result;
    }
} // namespace glsld
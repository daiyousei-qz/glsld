#include "Support/Uri.h"
#include "Support/CharUtils.h"

#include <vector>

namespace glsld
{
    struct UriSchemeParseResult
    {
        StringView scheme;
        StringView remainingText;
    };

    static auto IsUriUnreservedChar(char ch) -> bool
    {
        return IsAlpha(ch) || IsDigit(ch) || ch == '-' || ch == '.' || ch == '_' || ch == '~';
    }

    static auto IsUriSubDelimChar(char ch) -> bool
    {
        return ch == '!' || ch == '$' || ch == '&' || ch == '\'' || ch == '(' || ch == ')' || ch == '*' || ch == '+' ||
               ch == ',' || ch == ';' || ch == '=';
    }

    static auto ParseUriScheme(StringView input) -> UriSchemeParseResult
    {
        // A Uri scheme must start with an alpha character
        if (input.empty() || !IsAlpha(input.front())) {
            return {{}, input};
        }

        // A valid scheme must consists of alpha/digit characters or '+'/'-'/'.'
        auto schemeText =
            input.TakeWhile([](char ch) -> bool { return IsAlnum(ch) || ch == '+' || ch == '-' || ch == '.'; });

        // A valid scheme must be followed by a ':'
        auto remainder = input.Drop(schemeText.size());
        if (remainder.StartWith(':')) {
            return {schemeText, remainder.Drop(1)};
        }
        else {
            return {{}, input};
        }
    }

    struct UriComponentParseResult
    {
        std::optional<StringView> authority;
        StringView path;
        StringView remainingText;
    };

    static auto ParseUriComponentsAfterScheme(StringView input, bool skipAuthority) -> UriComponentParseResult
    {
        StringView remainingInput = input;

        // An authority always starts with "//", but we may also optionally skip parsing it.
        std::optional<StringView> authorityComponent;
        if (remainingInput.StartWith("//") && !skipAuthority) {
            remainingInput = remainingInput.Drop(2);

            // FIXME: This is actually accepting some invalid authority syntax. But it's fine for now.
            auto it  = remainingInput.begin();
            auto end = remainingInput.end();
            while (it != end) {
                const char ch = *it;
                if (IsUriUnreservedChar(ch) || IsUriSubDelimChar(ch) || ch == ':' || ch == '@') {
                    ++it;
                }
                else if (end - it >= 3 && ch == '%' && IsHexDigit(it[1]) && IsHexDigit(it[2])) {
                    it += 3;
                }
                else {
                    break;
                }
            }

            authorityComponent = {remainingInput.begin(), it};
            remainingInput     = {it, end};
        }

        // If authority part is present, the path part must start with a '/'.
        StringView pathComponent;
        if (!authorityComponent.has_value() || remainingInput.StartWith('/')) {
            auto it  = remainingInput.begin();
            auto end = remainingInput.end();
            while (it != end) {
                const char ch = *it;
                if (IsUriUnreservedChar(ch) || IsUriSubDelimChar(ch) || ch == ':' || ch == '@' || ch == '/') {
                    ++it;
                }
                else if (end - it >= 3 && ch == '%' && IsHexDigit(it[1]) && IsHexDigit(it[2])) {
                    it += 3;
                }
                else {
                    break;
                }
            }

            pathComponent  = {remainingInput.begin(), it};
            remainingInput = {it, end};
        }

        return {authorityComponent, pathComponent, remainingInput};
    }

    static auto NormalizeScheme(StringView scheme) -> std::string
    {
        std::string result;
        result.reserve(scheme.size());
        for (char ch : scheme) {
            result += ToLower(ch);
        }
        return result;
    }

    // Assuming path is a valid Uri path component.
    static auto NormalizePath(StringView path) -> std::string
    {
        if (path.empty()) {
            return "";
        }

        const bool isAbsolute = path.StartWith('/');

        std::vector<StringView> segments;
        std::optional<StringView> lastSegment;
        for (StringView segment : UriPathSegmentView{path}) {
            if (segment == "..") {
                if (!segments.empty()) {
                    segments.pop_back();
                }
                if (isAbsolute && segments.empty()) {
                    // For absolute path, we should not pop the root segment.
                    segments.push_back("");
                }
            }
            else if (segment != ".") {
                segments.push_back(segment);
            }

            lastSegment = segment;
        }
        if (lastSegment == "." || lastSegment == "..") {
            segments.push_back("");
        }
        GLSLD_ASSERT(!segments.empty());

        std::string result;
        bool firstSegment = true;
        for (StringView segment : segments) {
            if (!firstSegment) {
                result += '/';
            }
            result += segment;
            firstSegment = false;
        }

        return result;
    }

    // Assuming basePath and referencePath are both valid Uri path components.
    static auto MergePathHelper(StringView basePath, StringView referencePath, bool hasAuthority)
        -> std::optional<std::string>
    {
        std::string result;
        if (referencePath.StartWith('/')) {
            // Reference path is absolute. We need discard the base path.
            result = referencePath.Str();
        }
        else if (basePath.empty()) {
            // Base path is empty. Reference path is relative to the root.
            if (hasAuthority) {
                result = fmt::format("/{}", referencePath);
            }
            else {
                // FIXME: we should optionally prefer adding a leading '/'.
                result = referencePath.Str();
            }
        }
        else {
            // Note that if we are merging "mem:/tmp/.." and "test.txt", we'll get "mem:/tmp/test.txt" here as per RFC.
            result = fmt::format("{}{}", basePath.DropBackWhile([](char ch) { return ch != '/'; }), referencePath);
        }

        if (hasAuthority) {
            if (!result.empty() && !result.starts_with('/')) {
                // If there's an authority, the merged path must start with a '/'.
                return std::nullopt;
            }
        }
        else {
            if (result.starts_with("//")) {
                // Path cannot start with "//" if there's no authority.
                return std::nullopt;
            }
        }

        return result;
    }

    template <bool IsPercentEncoded>
    static auto DecodePercentEncodedHelper(const char* data, size_t length) -> PercentDecodeResult
    {
        if (length == 0) {
            return {'\0', 0};
        }

        if constexpr (!IsPercentEncoded) {
            // Input is not percent-encoded, so we simply return the first character.
            return {data[0], 1};
        }
        else {
            // Input is percent-encoded, "%HH" in the front should be replaced with the decoded character.
            // Notably, '%' that don't form a valid percent-encoding results in a failure.
            if (data[0] != '%') {
                return {data[0], 1};
            }
            if (length < 3 || !IsHexDigit(data[1]) || !IsHexDigit(data[2])) {
                return {'\0', 0};
            }

            return {static_cast<char>(HexDigitToInt(data[1]) * 16 + HexDigitToInt(data[2])), 3};
        }
    };

    template <bool LhsIsPercentEncoded, bool RhsIsPercentEncoded>
    static auto EqualPercentDecodedHelper(const char* lhsData, size_t lhsLength, const char* rhsData, size_t rhsLength)
        -> bool
    {
        while (lhsLength > 0 && rhsLength > 0) {
            PercentDecodeResult lhsDecoded = DecodePercentEncodedHelper<LhsIsPercentEncoded>(lhsData, lhsLength);
            PercentDecodeResult rhsDecoded = DecodePercentEncodedHelper<RhsIsPercentEncoded>(rhsData, rhsLength);

            if (lhsDecoded.decodedChar != rhsDecoded.decodedChar) {
                return false;
            }

            lhsData += lhsDecoded.consumedLength;
            lhsLength -= lhsDecoded.consumedLength;
            rhsData += rhsDecoded.consumedLength;
            rhsLength -= rhsDecoded.consumedLength;
        }

        return lhsLength == 0 && rhsLength == 0;
    }

    auto PercentEncodedView::DecodeFront() const -> PercentDecodeResult
    {
        return DecodePercentEncodedHelper<true>(data, length);
    }

    auto PercentEncodedView::DecodeFrontAndDrop() -> PercentDecodeResult
    {
        PercentDecodeResult result = DecodeFront();
        data += result.consumedLength;
        length -= result.consumedLength;
        return result;
    }

    auto PercentEncodedView::DecodeAll() const -> std::optional<std::string>
    {
        std::string result;
        result.reserve(length);

        for (PercentEncodedView view{*this}; !view.Empty();) {
            PercentDecodeResult decodeResult = view.DecodeFrontAndDrop();
            if (decodeResult.decodedChar == '\0') {
                return std::nullopt;
            }
            result += decodeResult.decodedChar;
        }

        return result;
    }

    auto PercentEncodedView::Equals(PercentEncodedView other) const -> bool
    {
        return EqualPercentDecodedHelper<true, true>(data, length, other.data, other.length);
    }

    auto PercentEncodedView::Equals(StringView other) const -> bool
    {
        return EqualPercentDecodedHelper<true, false>(data, length, other.data(), other.size());
    }

    auto PercentEncode(StringView text) -> std::string
    {
        constexpr auto hexDigits = "0123456789ABCDEF";

        auto shouldEncode = [](unsigned char ch) {
            return !(IsUriUnreservedChar(ch) || IsUriSubDelimChar(ch) || ch == ':' || ch == '@' || ch == '/');
        };

        std::string result;
        result.reserve(text.size());
        for (unsigned char ch : text) {
            if (shouldEncode(ch)) {
                result.push_back('%');
                result.push_back(hexDigits[ch >> 4]);
                result.push_back(hexDigits[ch & 0x0f]);
            }
            else {
                result.push_back(static_cast<char>(ch));
            }
        }

        return result;
    }

    auto ParsedUri::Parse(StringView uri) -> std::optional<ParsedUri>
    {
        auto [schemePart, inputAfterScheme] = ParseUriScheme(uri);
        if (schemePart.empty()) {
            // Scheme is required for a valid Uri.
            return std::nullopt;
        }

        auto [authorityPart, pathPart, remainingInput] = ParseUriComponentsAfterScheme(inputAfterScheme, false);
        if (!remainingInput.empty()) {
            // We don't support query and fragment part, so the remaining input must be empty for a valid Uri.
            return std::nullopt;
        }

        return ParsedUri{schemePart, authorityPart.value_or(StringView{}), pathPart, authorityPart.has_value()};
    }

    auto ParsedUri::GetNormalizedScheme() const -> std::string
    {
        return NormalizeScheme(scheme);
    }

    auto ParsedUri::GetNormalizedPath() const -> std::string
    {
        return NormalizePath(path);
    }

    auto ParsedUri::Normalize() const -> Uri
    {
        return Uri{NormalizeScheme(scheme), authority, NormalizePath(path), hasAuthority};
    }

    auto ParsedUri::NormalizeToDirectory() const -> Uri
    {
        auto normalizedPath = GetNormalizedPath();
        while (!normalizedPath.empty() && normalizedPath.back() != '/') {
            normalizedPath.pop_back();
        }
        return Uri{NormalizeScheme(scheme), authority, normalizedPath, hasAuthority};
    }

    auto ParsedUri::ToFileSystemPath() const -> std::optional<std::filesystem::path>
    {
        if (!TestScheme("file") || !authority.empty()) {
            return std::nullopt;
        }

        std::string percentDecodedPathBuffer;
        percentDecodedPathBuffer.reserve(path.size());
        for (PercentEncodedView pathDecodingView{path};;) {
            PercentDecodeResult decodeResult = pathDecodingView.DecodeFrontAndDrop();
            if (decodeResult.consumedLength > 1) {
                switch (decodeResult.decodedChar) {
                case '\0':
                case '\\':
                case '/':
                    // We should reject these characters in file path for security reason.
                    return std::nullopt;
                default:
                    break;
                }
            }

            if (decodeResult.consumedLength == 0) {
                break;
            }
            percentDecodedPathBuffer += decodeResult.decodedChar;
        }

        StringView percentDecodedPath = percentDecodedPathBuffer;
#if GLSLD_OS_WIN
        // On Windows, absolute path in uri looks like "/C:/path/to/file"
        // We need to remove the leading '/'.
        if (percentDecodedPath.size() >= 3 && percentDecodedPath[0] == '/' && IsAlpha(percentDecodedPath[1]) &&
            percentDecodedPath[2] == ':') {
            percentDecodedPath = percentDecodedPath.Drop(1);
        }
#endif

        return std::filesystem::path{percentDecodedPath.StdStrView()};
    }

    auto ParsedUri::MergePath(StringView referencePath) const -> std::optional<Uri>
    {
        auto [_, parsedRefPath, remainingInput] = ParseUriComponentsAfterScheme(referencePath, true);
        if (!remainingInput.empty()) {
            // Extra characters in path should cause parsing failure.
            return std::nullopt;
        }

        const auto mergedPath = MergePathHelper(path, parsedRefPath, hasAuthority);
        if (!mergedPath.has_value()) {
            return std::nullopt;
        }

        return Uri{scheme, authority, *mergedPath, hasAuthority};
    }

    auto ParsedUri::ResolveReference(StringView reference) const -> std::optional<Uri>
    {
        // If the reference has a non-empty scheme part, it's an absolute Uri and we can parse it directly.
        if (auto [parsedRefScheme, remainder] = ParseUriScheme(reference); !parsedRefScheme.empty()) {
            auto [parsedRefAuthority, parsedRefPath, remainingInput] = ParseUriComponentsAfterScheme(remainder, false);
            if (!remainingInput.empty()) {
                // We don't support query and fragment part, so the remaining input must be empty for a valid Uri.
                return std::nullopt;
            }

            return Uri{parsedRefScheme, parsedRefAuthority.value_or(StringView{}), parsedRefPath,
                       parsedRefAuthority.has_value()};
        }

        auto [parsedRefAuthority, parsedRefPath, remainingInput] = ParseUriComponentsAfterScheme(reference, false);

        // We don't support query and fragment, so the remaining input must be empty for a valid Uri.
        if (!remainingInput.empty()) {
            return std::nullopt;
        }

        // If the reference starts with an authority, it's an authority-relative Uri.
        // We inherit the scheme of the base Uri and parse the reference for authority and path.
        if (parsedRefAuthority) {
            return Uri{scheme, *parsedRefAuthority, parsedRefPath, true};
        }

        if (parsedRefPath.empty()) {
            return Uri{scheme, authority, path, hasAuthority};
        }

        // Otherwise, it's a path-relative reference.
        // We have to merge it with the base Uri's path.
        const auto mergedPath = MergePathHelper(path, parsedRefPath, hasAuthority);
        if (!mergedPath.has_value()) {
            return std::nullopt;
        }

        return Uri{scheme, authority, *mergedPath, hasAuthority};
    }

    auto FileSystemPathToUri(const std::filesystem::path& path, bool directory) -> std::optional<Uri>
    {
        std::string uriPath;
        try {
            uriPath = path.generic_string();
        }
        catch (const std::filesystem::filesystem_error&) {
            return std::nullopt;
        }
#if GLSLD_OS_WIN
        if (path.is_absolute() && uriPath.size() >= 2 && IsAlpha(uriPath[0]) && uriPath[1] == ':') {
            uriPath.insert(uriPath.begin(), '/');
        }
        // FIXME: Properly handle Windows UNC path.
#endif
        if (directory && !uriPath.empty() && !StringView{uriPath}.EndWith('/')) {
            uriPath.push_back('/');
        }

        auto uriText   = fmt::format("file:{}", PercentEncode(uriPath));
        auto parsedUri = ParsedUri::Parse(uriText);
        if (!parsedUri) {
            return std::nullopt;
        }
        return parsedUri->Normalize();
    }
} // namespace glsld

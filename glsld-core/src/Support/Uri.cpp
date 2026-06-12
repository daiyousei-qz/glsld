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
    static auto RemoveDotSegments(StringView path) -> std::string
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
        auto [parsedScheme, inputAfterScheme] = ParseUriScheme(uri);
        if (parsedScheme.empty()) {
            // Scheme is required for a valid Uri.
            return std::nullopt;
        }

        auto [parsedAuthority, parsedPath, remainingInput] = ParseUriComponentsAfterScheme(inputAfterScheme, false);
        if (!remainingInput.empty()) {
            // We don't support query and fragment part, so the remaining input must be empty for a valid Uri.
            return std::nullopt;
        }

        return ParsedUri{parsedScheme, parsedAuthority, parsedPath};
    }

    auto ParsedUri::GetNormalizedScheme() const -> std::string
    {
        return NormalizeScheme(scheme);
    }

    auto ParsedUri::GetNormalizedPath() const -> std::string
    {
        return RemoveDotSegments(path);
    }

    auto ParsedUri::ToUri() const -> Uri
    {
        return Uri{scheme, authority, path, HasAuthority()};
    }

    auto ParsedUri::ToNormalizedUri() const -> Uri
    {
        return Uri{NormalizeScheme(scheme), authority, RemoveDotSegments(path), HasAuthority()};
    }

    auto ParsedUri::MergePath(StringView referencePath) const -> std::optional<Uri>
    {
        auto [_, parsedRefPath, remainingInput] = ParseUriComponentsAfterScheme(referencePath, true);
        if (!remainingInput.empty()) {
            // Extra characters in path should cause parsing failure.
            return std::nullopt;
        }

        const auto mergedPath = MergePathHelper(path, parsedRefPath, HasAuthority());
        if (!mergedPath.has_value()) {
            return std::nullopt;
        }

        return Uri{scheme, authority, *mergedPath, HasAuthority()};
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
            return Uri{scheme, authority, path, HasAuthority()};
        }

        // Otherwise, it's a path-relative reference.
        // We have to merge it with the base Uri's path.
        const auto mergedPath = MergePathHelper(path, parsedRefPath, HasAuthority());
        if (!mergedPath.has_value()) {
            return std::nullopt;
        }

        return Uri{scheme, authority, *mergedPath, HasAuthority()};
    }

    auto UriToFileSystemPath(const ParsedUri& uri) -> std::optional<std::string>
    {
        if (!uri.TestScheme("file") || !uri.GetRawAuthority().Empty()) {
            return std::nullopt;
        }

        std::string percentDecodedPath;
        percentDecodedPath.reserve(uri.GetRawPath().GetText().size());
        for (PercentEncodedView pathDecodingView = uri.GetRawPath();;) {
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
            percentDecodedPath += decodeResult.decodedChar;
        }

#if GLSLD_OS_WIN
        // On Windows, absolute path in uri looks like "/C:/path/to/file"
        // We need to remove the leading '/'.
        if (percentDecodedPath.size() >= 3 && percentDecodedPath[0] == '/' && IsAlpha(percentDecodedPath[1]) &&
            percentDecodedPath[2] == ':') {
            percentDecodedPath.erase(percentDecodedPath.begin());
        }
#endif

        return percentDecodedPath;
    }

    static auto IsAbsolutePath(StringView path) -> bool
    {
#if GLSLD_OS_WIN
        if (path.size() < 3) {
            return false;
        }
        if (!IsAlpha(path[0]) || path[1] != ':' || !(path[2] == '/' || path[2] == '\\')) {
            return false;
        }
        return true;
#else
        return path.StartWith('/');
#endif
    }

    auto FileSystemPathToUri(StringView path, bool directory) -> std::optional<Uri>
    {
        if (!IsAbsolutePath(path)) {
            return std::nullopt;
        }

        std::string uriBuffer;
        uriBuffer.reserve(6 + path.size() + (directory ? 1 : 0));

        uriBuffer = "file:";

#if GLSLD_OS_WIN
        // Drive-absolute paths are written as "/C:/path" in file URIs.
        uriBuffer += '/';
#endif

        for (char ch : path) {
            if (ch == '\0') {
                return std::nullopt;
            }

#if GLSLD_OS_WIN
            uriBuffer += ch == '\\' ? '/' : ch;
#else
            uriBuffer += ch;
#endif
        }

        if (directory && !uriBuffer.empty() && uriBuffer.back() != '/') {
            uriBuffer += '/';
        }

        auto encodedPath = PercentEncode(uriBuffer);
        auto parsedUri   = ParsedUri::Parse(encodedPath);
        if (!parsedUri) {
            return std::nullopt;
        }
        return parsedUri->ToUri();
    }
} // namespace glsld

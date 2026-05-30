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
            // If the reference path is absolute, base path is discarded.
            result = NormalizePath(referencePath);
        }
        else if (basePath.empty()) {
            // If base path is empty, we also just use the reference path.
            if (hasAuthority) {
                GLSLD_ASSERT(!referencePath.StartWith('/'));
                result = NormalizePath(fmt::format("/{}", referencePath));
            }
            else {
                result = NormalizePath(referencePath);
            }
        }
        else if (basePath.EndWith('/')) {
            // If the base path is a directory, we simply concatenate the reference path to it.
            result = NormalizePath(fmt::format("{}{}", basePath, referencePath));
        }
        else {
            // Otherwise, base path is a file.
            GLSLD_ASSERT(!basePath.EndWith('/') && !referencePath.StartWith('/'));
            result = NormalizePath(fmt::format("{}/../{}", basePath, referencePath));
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

    auto DecodeUriComponent(StringView component, std::function<bool(char ch)> filter) -> std::optional<std::string>
    {
        std::string result;
        result.reserve(component.size());

        for (size_t i = 0; i < component.size(); ++i) {
            if (component[i] != '%') {
                result += component[i];
                continue;
            }

            if (i + 2 >= component.size() || !IsHexDigit(component[i + 1]) || !IsHexDigit(component[i + 2])) {
                return std::nullopt;
            }

            auto decodedChar =
                static_cast<char>(HexDigitToInt(component[i + 1]) * 16 + HexDigitToInt(component[i + 2]));
            if (!filter(decodedChar)) {
                return std::nullopt;
            }
            result += decodedChar;
            i += 2;
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
        std::string result;
        result.reserve(scheme.size());
        for (char ch : scheme) {
            if (ch >= 'A' && ch <= 'Z') {
                result += static_cast<char>(ch - 'A' + 'a');
            }
            else {
                result += ch;
            }
        }

        return result;
    }

    auto ParsedUri::GetNormalizedPath() const -> std::string
    {
        return NormalizePath(path);
    }

    auto ParsedUri::ToFileSystemPath() const -> std::optional<std::filesystem::path>
    {
        if (GetNormalizedScheme() != "file" || !authority.empty()) {
            return std::nullopt;
        }

        auto normalizedPathOpt =
            DecodeUriComponent(GetNormalizedPath(), [](char ch) { return ch != '\0' && ch != '/' && ch != '\\'; });
        if (!normalizedPathOpt.has_value()) {
            return std::nullopt;
        }

        StringView normalizedPath = *normalizedPathOpt;
#if GLSLD_OS_WIN
        // On Windows, absolute path in uri looks like "/C:/path/to/file"
        // We need to remove the leading '/'.
        if (normalizedPath.size() >= 3 && normalizedPath[0] == '/' && IsAlpha(normalizedPath[1]) &&
            normalizedPath[2] == ':') {
            normalizedPath = normalizedPath.Drop(1);
        }
#endif

        return std::filesystem::path{normalizedPath.StdStrView()};
    }

    auto ParsedUri::MergePath(StringView path) const -> std::optional<Uri>
    {
        auto [_, pathPart, remainingInput] = ParseUriComponentsAfterScheme(path, true);
        if (!remainingInput.empty()) {
            // Extra characters in path should cause parsing failure.
            return std::nullopt;
        }

        auto mergedPath = MergePathHelper(GetRawPath(), pathPart, hasAuthority);
        if (!mergedPath.has_value()) {
            return std::nullopt;
        }

        return Uri{ParsedUri{GetRawScheme(), GetRawAuthority(), *mergedPath, hasAuthority}};
    }

    auto ParsedUri::ResolveReference(StringView reference) const -> std::optional<Uri>
    {
        // If the reference has a non-empty scheme part, it's an absolute Uri and we can parse it directly.
        if (auto [schemePart, remainder] = ParseUriScheme(reference); !schemePart.empty()) {
            auto [authorityPart, pathPart, remainingInput] = ParseUriComponentsAfterScheme(remainder, false);
            if (!remainingInput.empty()) {
                // We don't support query and fragment part, so the remaining input must be empty for a valid Uri.
                return std::nullopt;
            }

            return Uri{ParsedUri{schemePart, authorityPart.value_or(StringView{}), NormalizePath(pathPart),
                                 authorityPart.has_value()}};
        }

        auto [authorityPart, pathPart, remainingInput] = ParseUriComponentsAfterScheme(reference, false);

        // We don't support query and fragment, so the remaining input must be empty for a valid Uri.
        if (!remainingInput.empty()) {
            return std::nullopt;
        }

        // If the reference starts with an authority, it's an authority-relative Uri.
        // We inherit the scheme of the base Uri and parse the reference for authority and path.
        if (authorityPart) {
            return Uri{ParsedUri{GetRawScheme(), *authorityPart, NormalizePath(pathPart), true}};
        }

        if (pathPart.empty()) {
            return Uri{ParsedUri{GetRawScheme(), GetRawAuthority(), NormalizePath(GetRawPath()), hasAuthority}};
        }

        // Otherwise, it's a path-relative reference.
        // We have to merge it with the base Uri's path.
        const auto mergedPath = MergePathHelper(GetRawPath(), pathPart, hasAuthority);
        if (!mergedPath.has_value()) {
            return std::nullopt;
        }

        return Uri{ParsedUri{GetRawScheme(), GetRawAuthority(), *mergedPath, hasAuthority}};
    }
} // namespace glsld

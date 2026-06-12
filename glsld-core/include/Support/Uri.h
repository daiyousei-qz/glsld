#pragma once
#include "Support/StringView.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <iterator>
#include <optional>
#include <string>

namespace glsld
{
    class ParsedUri;
    class Uri;

    struct PercentDecodeResult
    {
        // The decoded character. It is '\0' if the input is empty or invalid for decoding.
        char decodedChar;

        // The number of characters that decoded character consumes. It is:
        // - 0 if the input is empty or invalid for decoding.
        // - 1 for a normal character
        // - 3 for a percent-encoded character (e.g. "%20")
        size_t consumedLength;
    };

    // A view to a percent-encoded string. It provides utilities to decode the string in a streaming way.
    // Note validation of encoding is lazy, so decoding may fail and returns '\0'.
    class PercentEncodedView
    {
    private:
        const char* data;
        size_t length;

    public:
        explicit PercentEncodedView(StringView encodedText) : data(encodedText.data()), length(encodedText.size())
        {
        }

        auto Empty() const -> bool
        {
            return length == 0;
        }

        auto GetText() const -> StringView
        {
            return StringView{data, length};
        }

        // Decodes the first percent-encoded character.
        auto DecodeFront() const -> PercentDecodeResult;

        // Decodes the first percent-encoded character and drops it from the view.
        auto DecodeFrontAndDrop() -> PercentDecodeResult;

        auto DecodeAll() const -> std::optional<std::string>;

        // Compares decoded text of this view and another percent-encoded view for equality.
        // If any of the two views contains invalid percent-encoding, returns false.
        auto Equals(PercentEncodedView other) const -> bool;

        // Compares decoded text of this view and a normal string for equality.
        // If this view contains invalid percent-encoding, returns false.
        auto Equals(StringView other) const -> bool;
    };

    // Percent-encodes a path-like string for use as a URI path or path-relative reference.
    auto PercentEncode(StringView text) -> std::string;

    // A lazy view to split a Uri path into segments. A segment is a substring between '/' characters or start/end of
    // the path. For example, the path "/a//b/c" will have ["", "a", "", "b", "c"] segments.
    //
    // Notably, we don't do validation on the path syntax here but simply split it by '/' character. It is undefined
    // behavior to use this view on an invalid path.
    class UriPathSegmentView
    {
    private:
        StringView path;

        class SegmentIterator
        {
        private:
            StringView path;
            size_t segmentLength;
            size_t nextSegmentOffset;
            bool finished;

            auto AdvanceToNext() -> void
            {
                if (nextSegmentOffset > path.size()) {
                    finished = true;
                    return;
                }

                path = path.Drop(nextSegmentOffset);

                size_t segmentEnd = 0;
                while (segmentEnd < path.size() && path[segmentEnd] != '/') {
                    ++segmentEnd;
                }

                segmentLength     = segmentEnd;
                nextSegmentOffset = segmentEnd == path.size() ? path.size() + 1 : segmentEnd + 1;
                finished          = false;
            }

        public:
            using difference_type = std::ptrdiff_t;
            using value_type      = StringView;

            explicit SegmentIterator(StringView path) : path(path), segmentLength(0), nextSegmentOffset(0)
            {
                if (path.empty()) {
                    finished = true;
                }
                else {
                    AdvanceToNext();
                }
            }

            auto operator*() const -> StringView
            {
                return path.Take(segmentLength);
            }

            auto operator++() -> SegmentIterator&
            {
                AdvanceToNext();
                return *this;
            }

            auto operator++(int) -> void
            {
                ++(*this);
            }

            auto operator==(const std::default_sentinel_t&) const -> bool
            {
                return finished;
            }
        };

    public:
        explicit UriPathSegmentView(StringView path) : path(path)
        {
        }

        auto begin() const -> SegmentIterator
        {
            return SegmentIterator{path};
        }

        auto end() const -> std::default_sentinel_t
        {
            return std::default_sentinel;
        }
    };

    static_assert(std::ranges::input_range<UriPathSegmentView>);

    // ParsedUri represents a view to a parsed/validated Uri string. It provides access to the different components of
    // the Uri and also some utility functions to resolve relative references.
    //
    // Uri = scheme ":" ["//" authority] path ["?" query] ["#" fragment]
    // We only care about scheme, authority and path. If query and fragment part is present, we should fail parsing.
    class ParsedUri
    {
    private:
        StringView scheme;
        // Percent-encoded authority.
        // As an optimization, we assume nullptr means authority is not present.
        StringView authority;
        // Percent-encoded path.
        StringView path;

        ParsedUri(StringView scheme, std::optional<StringView> authority, StringView path)
            : scheme(scheme), authority(authority.value_or(StringView{})), path(path)
        {
            static_assert(StringView{}.data() == nullptr);
        }

        friend class Uri;

    public:
        static auto Parse(StringView uri) -> std::optional<ParsedUri>;

        auto HasAuthority() const -> bool
        {
            return authority.data() != nullptr;
        }

        auto GetRawScheme() const -> StringView
        {
            return scheme;
        }
        auto GetRawAuthority() const -> PercentEncodedView
        {
            return PercentEncodedView{authority};
        }
        auto GetRawPath() const -> PercentEncodedView
        {
            return PercentEncodedView{path};
        }

        auto TestScheme(StringView scheme) const -> bool
        {
            return GetRawScheme().EqualsIgnoreCase(scheme);
        }

        // The scheme is case-insensitive, we normalize it to lower case when returning.
        auto GetNormalizedScheme() const -> std::string;

        // This removes dot segments in the path.
        auto GetNormalizedPath() const -> std::string;

        // Get a lazy view of all path segments.
        // For example, the path "/a/b/c" will have ["", "a", "b", "c"], and the path "/a/" will have ["", "a", ""].
        auto GetPathSegments() const -> UriPathSegmentView
        {
            return UriPathSegmentView{path};
        }

        auto RemoveFileName() const -> ParsedUri
        {
            return ParsedUri{scheme, authority, path.DropBackWhile([](char ch) { return ch != '/'; })};
        }

        // Converts this view to a Uri object that owns the full uri string.
        auto ToUri() const -> Uri;

        // Converts this view to a lexically normal Uri object with that owns the full uri string.
        // The normalization includes:
        // - Scheme is lower-cased.
        // - Dot segments in the path are removed.
        // Note that this transformation is purely lexical and may cause the semantics of the Uri to change in certain
        // scheme. So caller may want to avoid this as much as possible.
        auto ToNormalizedUri() const -> Uri;

        // Merges a path reference without removing dot segments.
        auto MergePath(StringView path) const -> std::optional<Uri>;

        // Resolves a URI reference without removing dot segments.
        auto ResolveReference(StringView reference) const -> std::optional<Uri>;
    };

    // A thin wrapper around ParsedUri that also stores the raw Uri text.
    class Uri
    {
    private:
        std::string rawUri;

        struct StringViewIndexPair
        {
            uint32_t offset;
            uint32_t length;
        };

        StringViewIndexPair schemeRange;
        StringViewIndexPair authorityRange;
        StringViewIndexPair pathRange;
        bool authorityPresent;

        friend class ParsedUri;

        Uri(StringView scheme, StringView authority, StringView path, bool hasAuthority)
        {
            rawUri.reserve(scheme.size() + authority.size() + path.size() + 3);

            schemeRange = {0, static_cast<uint32_t>(scheme.size())};
            rawUri += scheme;
            rawUri += ':';

            authorityPresent = hasAuthority;
            if (authorityPresent) {
                rawUri += "//";
            }

            authorityRange = {static_cast<uint32_t>(rawUri.size()), static_cast<uint32_t>(authority.size())};
            rawUri += authority;

            pathRange = {static_cast<uint32_t>(rawUri.size()), static_cast<uint32_t>(path.size())};
            rawUri += path;
        }

    public:
        auto GetRawText() const -> StringView
        {
            return rawUri;
        }

        auto GetParsedUri() const -> ParsedUri
        {
            auto scheme = StringView{rawUri.data() + schemeRange.offset, schemeRange.length};
            auto authority =
                authorityPresent
                    ? std::optional{StringView{rawUri.data() + authorityRange.offset, authorityRange.length}}
                    : std::nullopt;
            auto path = StringView{rawUri.data() + pathRange.offset, pathRange.length};

            return ParsedUri{scheme, authority, path};
        }
    };

    // Converts the uri to a filesystem path. We only promise we don't change the semantics of a valid path.
    // Even if we return a path, it doesn't necessarily mean the path is valid or exists in the filesystem.
    auto UriToFileSystemPath(const ParsedUri& uri) -> std::optional<std::string>;

    // Converts a native filesystem path to a file URI. If `directory` is true, the URI path is made directory-like by
    // appending a trailing slash unless the path is empty.
    auto FileSystemPathToUri(StringView path, bool directory = false) -> std::optional<Uri>;

} // namespace glsld

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

            SegmentIterator() : path(), segmentLength(0), nextSegmentOffset(0), finished(true)
            {
            }
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
            return path.empty() ? SegmentIterator{} : SegmentIterator{path};
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
        StringView authority;
        StringView path;
        bool hasAuthority;

        ParsedUri(StringView scheme, StringView authority, StringView path, bool hasAuthority)
            : scheme(scheme), authority(authority), path(path), hasAuthority(hasAuthority)
        {
        }

        friend class Uri;

    public:
        static auto Parse(StringView uri) -> std::optional<ParsedUri>;

        auto GetRawScheme() const -> StringView
        {
            return scheme;
        }

        auto GetRawAuthority() const -> StringView
        {
            return authority;
        }
        auto GetRawPath() const -> StringView
        {
            return path;
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

        // Converts the uri to a filesystem path. We only promise we don't change the semantics of a valid path.
        // Even if we return a path, it doesn't necessarily mean the path is valid or exists in the filesystem.
        auto ToFileSystemPath() const -> std::optional<std::filesystem::path>;

        auto MergePath(StringView path) const -> std::optional<Uri>;

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

    public:
        explicit Uri(const ParsedUri& uri)
        {
            rawUri.reserve(uri.GetRawScheme().size() + uri.GetRawAuthority().size() + uri.GetRawPath().size() + 3);

            schemeRange = {0, static_cast<uint32_t>(uri.GetRawScheme().size())};
            rawUri += uri.GetRawScheme();
            rawUri += ':';

            authorityPresent = uri.hasAuthority;
            if (authorityPresent) {
                rawUri += "//";
            }

            authorityRange = {static_cast<uint32_t>(rawUri.size()),
                              static_cast<uint32_t>(uri.GetRawAuthority().size())};
            rawUri += uri.GetRawAuthority();

            pathRange = {static_cast<uint32_t>(rawUri.size()), static_cast<uint32_t>(uri.GetRawPath().size())};
            rawUri += uri.GetRawPath();
        }

        auto GetRawText() const -> StringView
        {
            return rawUri;
        }

        auto GetParsedUri() const -> ParsedUri
        {
            return ParsedUri{StringView{rawUri.data() + schemeRange.offset, schemeRange.length},
                             StringView{rawUri.data() + authorityRange.offset, authorityRange.length},
                             StringView{rawUri.data() + pathRange.offset, pathRange.length}, authorityPresent};
        }
    };

    // Decodes a percent-encoded Uri component. If the component contains invalid percent-encoding, returns nullopt.
    //
    // Note we don't validate the syntax of the component here.
    auto DecodeUriComponent(
        StringView component, std::function_ref<bool(char ch)> filter = [](char) { return true; })
        -> std::optional<std::string>;

} // namespace glsld

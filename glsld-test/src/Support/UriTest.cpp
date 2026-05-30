#include "Catch2Wrapper.h"

#include "Support/Uri.h"

#include <optional>
#include <vector>

using namespace glsld;

namespace
{
    auto CheckParsedUriHelper(const ParsedUri& parsed, StringView expectedScheme, StringView expectedAuthority,
                              StringView expectedPath) -> void
    {
        CHECK(parsed.GetRawScheme() == expectedScheme);
        CHECK(parsed.GetRawAuthority() == expectedAuthority);
        CHECK(parsed.GetRawPath() == expectedPath);
    }

    auto ExpectParsed(StringView uri, StringView expectedScheme, StringView expectedAuthority, StringView expectedPath)
        -> ParsedUri
    {
        INFO(fmt::format("Parsing URI: {}", uri));

        auto parsed = ParsedUri::Parse(uri);
        REQUIRE(parsed.has_value());
        CheckParsedUriHelper(*parsed, expectedScheme, expectedAuthority, expectedPath);

        return *parsed;
    }

    auto ExpectParsed(std::optional<Uri> uri, StringView expectedScheme, StringView expectedAuthority,
                      StringView expectedPath) -> Uri
    {
        REQUIRE(uri.has_value());
        INFO(fmt::format("Seeing URI: {}", uri->GetRawText()));

        CheckParsedUriHelper(uri->GetParsedUri(), expectedScheme, expectedAuthority, expectedPath);

        return std::move(*uri);
    }

    auto ExpectInvalidUri(StringView uri) -> void
    {
        INFO(fmt::format("Parsing URI: {}", uri));

        auto parsed = ParsedUri::Parse(uri);
        if (parsed.has_value()) {
            FAIL(fmt::format("Expected parsing to fail, but got scheme: '{}', authority: '{}', path: '{}'",
                             parsed->GetRawScheme(), parsed->GetRawAuthority(), parsed->GetRawPath()));
        }
    }
} // namespace

TEST_CASE("Support::ParsedUriTest")
{
    SECTION("UriPathSegmentView")
    {
        REQUIRE(std::vector<StringView>{std::from_range, UriPathSegmentView("")} == std::vector<StringView>{});
        REQUIRE(std::vector<StringView>{std::from_range, UriPathSegmentView("/")} == std::vector<StringView>{"", ""});
        REQUIRE(std::vector<StringView>{std::from_range, UriPathSegmentView("//")} ==
                std::vector<StringView>{"", "", ""});
        REQUIRE(std::vector<StringView>{std::from_range, UriPathSegmentView("/a/b/")} ==
                std::vector<StringView>{"", "a", "b", ""});
        REQUIRE(std::vector<StringView>{std::from_range, UriPathSegmentView("/a//b/")} ==
                std::vector<StringView>{"", "a", "", "b", ""});
        REQUIRE(std::vector<StringView>{std::from_range, UriPathSegmentView("a/b/c")} ==
                std::vector<StringView>{"a", "b", "c"});
        REQUIRE(std::vector<StringView>{std::from_range, UriPathSegmentView("/a/b/c")} ==
                std::vector<StringView>{"", "a", "b", "c"});
    }

    SECTION("ParsedUri::Parse")
    {
        SECTION("Parses scheme, authority, and path")
        {
            ExpectParsed("file:///tmp/shader.glsl", "file", "", "/tmp/shader.glsl");
            ExpectParsed("file:/tmp/shader.glsl", "file", "", "/tmp/shader.glsl");
            ExpectParsed("file:relative/shader.glsl", "file", "", "relative/shader.glsl");
            ExpectParsed("untitled:shader.glsl", "untitled", "", "shader.glsl");
            ExpectParsed("http://example.com/shaders/main.glsl", "http", "example.com", "/shaders/main.glsl");
            ExpectParsed("http://example.com", "http", "example.com", "");
            ExpectParsed("custom+v1.2-scheme://user:pass@example.com:8080/a/b", "custom+v1.2-scheme",
                         "user:pass@example.com:8080", "/a/b");
        }

        SECTION("Accepts empty path and empty authority")
        {
            ExpectParsed("a:", "a", "", "");
            ExpectParsed("a://", "a", "", "");
            ExpectParsed("a:///path", "a", "", "/path");
            ExpectParsed("a:////path", "a", "", "//path");
        }

        SECTION("Accepts percent escapes in authority and path")
        {
            ExpectParsed("scheme://user%3Apass@example.com/%E4%B8%AD%20space/%2Fslash", "scheme",
                         "user%3Apass@example.com", "/%E4%B8%AD%20space/%2Fslash");
        }

        SECTION("Rejects invalid scheme syntax")
        {
            // Scheme must start with a letter
            ExpectInvalidUri("1file:/tmp/shader.glsl");
            ExpectInvalidUri("+file:/tmp/shader.glsl");
            ExpectInvalidUri("-file:/tmp/shader.glsl");
            ExpectInvalidUri(".file:/tmp/shader.glsl");

            // Scheme requires a following ':'
            ExpectInvalidUri("file");

            // Invalid characters in scheme
            ExpectInvalidUri("file/tmp/shader.glsl");
            ExpectInvalidUri("file_path:/tmp/shader.glsl");
            ExpectInvalidUri("file path:/tmp/shader.glsl");
            ExpectInvalidUri("file?query:/tmp/shader.glsl");

            // Empty scheme is not allowed
            ExpectInvalidUri(":");
            ExpectInvalidUri("");

            // Scheme cannot contain %<hex><hex> sequences
            ExpectInvalidUri("fi%6Ce:/tmp/shader.glsl");
        }

        SECTION("Rejects invalid authority syntax")
        {
            ExpectInvalidUri("scheme://autho rity/path");
            ExpectInvalidUri("scheme://autho%2/path");
        }

        SECTION("Rejects invalid path syntax")
        {
            ExpectInvalidUri("scheme:/pa th");
            ExpectInvalidUri("scheme:/pa%2Gth");
            ExpectInvalidUri("scheme:/pa%2th");
        }

        SECTION("Rejects query and fragment")
        {
            ExpectInvalidUri("file:/tmp/shader.glsl?version=1");
            ExpectInvalidUri("file:/tmp/shader.glsl#main");
            ExpectInvalidUri("file:/tmp/shader.glsl?version=1#main");
            ExpectInvalidUri("file://example.com?query");
            ExpectInvalidUri("file://example.com#fragment");
            ExpectInvalidUri("file:?query");
            ExpectInvalidUri("file:#fragment");
        }
    }

    SECTION("ParsedUri::GetNormalizedScheme")
    {
        auto parsed = ExpectParsed("FiLe+V1.2-Scheme:path", "FiLe+V1.2-Scheme", "", "path");
        CHECK(parsed.GetNormalizedScheme() == "file+v1.2-scheme");
    }

    SECTION("ParsedUri::GetNormalizedPath")
    {
        // We should remove dot segments, but preserve multiple slashes.
        CHECK(ExpectParsed("scheme:/a/b/../c/./d//e", "scheme", "", "/a/b/../c/./d//e").GetNormalizedPath() ==
              "/a/c/d//e");
        CHECK(ExpectParsed("scheme:relative/path/./to/../resource", "scheme", "", "relative/path/./to/../resource")
                  .GetNormalizedPath() == "relative/path/resource");
        CHECK(ExpectParsed("scheme:.", "scheme", "", ".").GetNormalizedPath() == "");
        CHECK(ExpectParsed("scheme:..", "scheme", "", "..").GetNormalizedPath() == "");
        CHECK(ExpectParsed("scheme:./", "scheme", "", "./").GetNormalizedPath() == "");
        CHECK(ExpectParsed("scheme:../", "scheme", "", "../").GetNormalizedPath() == "");
        CHECK(ExpectParsed("scheme:/./", "scheme", "", "/./").GetNormalizedPath() == "/");
        CHECK(ExpectParsed("scheme:a/.", "scheme", "", "a/.").GetNormalizedPath() == "a/");
        CHECK(ExpectParsed("scheme:a/..", "scheme", "", "a/..").GetNormalizedPath() == "");
    }

    SECTION("ParsedUri::ToFileSystemPath")
    {
        SECTION("Converts file URIs without authority to filesystem paths")
        {
#if GLSLD_OS_WIN
            {
                auto driveAbsoluteFile = ExpectParsed("file:/C:/tmp/shader.glsl", "file", "", "/C:/tmp/shader.glsl");
                auto path              = driveAbsoluteFile.ToFileSystemPath();
                REQUIRE(path.has_value());
                CHECK(path->is_absolute());
                CHECK(path->string() == "C:\\tmp\\shader.glsl");
            }

            {
                auto driveRelativeFile = ExpectParsed("file:C:/tmp/shader.glsl", "file", "", "C:/tmp/shader.glsl");
                auto path              = driveRelativeFile.ToFileSystemPath();
                REQUIRE(path.has_value());
                CHECK(path->is_absolute());
                CHECK(path->string() == "C:\\tmp\\shader.glsl");
            }
#endif

            {
                auto absoluteFile = ExpectParsed("file:/tmp/shader.glsl", "file", "", "/tmp/shader.glsl");
                auto path         = absoluteFile.ToFileSystemPath();
                REQUIRE(path.has_value());
                CHECK(path->has_root_path());
                CHECK(path->string() == "/tmp/shader.glsl");
            }

            {
                auto relativeFile = ExpectParsed("file:relative/shader.glsl", "file", "", "relative/shader.glsl");
                auto path         = relativeFile.ToFileSystemPath();
                REQUIRE(path.has_value());
                CHECK(!path->has_root_path());
                CHECK(path->string() == "relative/shader.glsl");
            }

            {
                // We don't collapse multiple slashes in the path. We expect OS to handle it correctly.
                auto multipleSlashes = ExpectParsed("file:////tmp///shader.glsl", "file", "", "//tmp///shader.glsl");
                auto path            = multipleSlashes.ToFileSystemPath();
                REQUIRE(path.has_value());
                CHECK(path->has_root_path());
                CHECK(path->string() == "//tmp///shader.glsl");
            }

            {
                auto upperCaseFile = ExpectParsed("FILE:/tmp/shader.glsl", "FILE", "", "/tmp/shader.glsl");
                auto path          = upperCaseFile.ToFileSystemPath();
                REQUIRE(path.has_value());
                CHECK(path->has_root_path());
                CHECK(path->string() == "/tmp/shader.glsl");
            }

            {
                // '/' is not allowed in percent-encoding when converting to filesystem path.
                CHECK(!ExpectParsed("file:/tmp/bad%2fpath.glsl", "file", "", "/tmp/bad%2fpath.glsl")
                           .ToFileSystemPath()
                           .has_value());
                // '\\' is not allowed in percent-encoding when converting to filesystem path.
                CHECK(!ExpectParsed("file:/tmp/bad%5cpath.glsl", "file", "", "/tmp/bad%5cpath.glsl")
                           .ToFileSystemPath()
                           .has_value());
                // '\0' is not allowed in percent-encoding when converting to filesystem path.
                CHECK(!ExpectParsed("file:/tmp/bad%00path.glsl", "file", "", "/tmp/bad%00path.glsl")
                           .ToFileSystemPath()
                           .has_value());

                // FIXME: what about percent-decoded '.'?
            }
        }

        SECTION("Rejects filesystem conversion for non-file or authority URIs")
        {
            auto nonFile = ExpectParsed("untitled:shader.glsl", "untitled", "", "shader.glsl");
            auto fileWithAuthority =
                ExpectParsed("file://example.com/tmp/shader.glsl", "file", "example.com", "/tmp/shader.glsl");

            CHECK(!nonFile.ToFileSystemPath().has_value());
            CHECK(!fileWithAuthority.ToFileSystemPath().has_value());
        }
    }

    SECTION("ParsedUri::MergePath")
    {
        SECTION("Without authority")
        {
            auto fileBase =
                ExpectParsed("file:/workspace/shaders/main.glsl", "file", "", "/workspace/shaders/main.glsl");
            ExpectParsed(fileBase.MergePath("common.glsl"), "file", "", "/workspace/shaders/common.glsl");
            ExpectParsed(fileBase.MergePath("includes/lighting.glsl"), "file", "",
                         "/workspace/shaders/includes/lighting.glsl");
            ExpectParsed(fileBase.MergePath("."), "file", "", "/workspace/shaders/");
            ExpectParsed(fileBase.MergePath(".."), "file", "", "/workspace/");
            ExpectParsed(fileBase.MergePath("./"), "file", "", "/workspace/shaders/");
            ExpectParsed(fileBase.MergePath("../"), "file", "", "/workspace/");
            ExpectParsed(fileBase.MergePath("../../test.glsl"), "file", "", "/test.glsl");
            ExpectParsed(fileBase.MergePath("/shared/./include/../common.glsl"), "file", "", "/shared/common.glsl");
            // Empty path should be preserved for URI handling.
            ExpectParsed(fileBase.MergePath(".///test.glsl"), "file", "", "/workspace/shaders///test.glsl");

            auto directoryBase = ExpectParsed("file:/workspace/shaders/", "file", "", "/workspace/shaders/");
            ExpectParsed(directoryBase.MergePath("lighting.glsl"), "file", "", "/workspace/shaders/lighting.glsl");
            ExpectParsed(directoryBase.MergePath("."), "file", "", "/workspace/shaders/");
            ExpectParsed(directoryBase.MergePath(".."), "file", "", "/workspace/");
            ExpectParsed(directoryBase.MergePath(".//"), "file", "", "/workspace/shaders//");
            // Path cannot start with "//" if there's no authority, so merging with ".//" should fail.
            REQUIRE(!directoryBase.MergePath("../..//").has_value());

            auto baseWithDotSegments = ExpectParsed("file:/workspace/shaders/./generated/../main.glsl", "file", "",
                                                    "/workspace/shaders/./generated/../main.glsl");
            // Merging path dot segments removal.
            ExpectParsed(baseWithDotSegments.MergePath("../include/common.glsl"), "file", "",
                         "/workspace/include/common.glsl");

            auto rootlessBase = ExpectParsed("untitled:main.glsl", "untitled", "", "main.glsl");
            ExpectParsed(rootlessBase.MergePath("common.glsl"), "untitled", "", "common.glsl");
            ExpectParsed(rootlessBase.MergePath("."), "untitled", "", "");
            ExpectParsed(rootlessBase.MergePath(".."), "untitled", "", "");

            auto emptyBase = ExpectParsed("untitled:", "untitled", "", "");
            ExpectParsed(emptyBase.MergePath("path/to/resource"), "untitled", "", "path/to/resource");
            ExpectParsed(emptyBase.MergePath("."), "untitled", "", "");
            ExpectParsed(emptyBase.MergePath(".."), "untitled", "", "");
        }

        SECTION("Preserves authority while merging")
        {
            auto base =
                ExpectParsed("cache://address/project/src/main.glsl", "cache", "address", "/project/src/main.glsl");
            ExpectParsed(base.MergePath("include/common.glsl"), "cache", "address", "/project/src/include/common.glsl");
            ExpectParsed(base.MergePath("../include/common.glsl"), "cache", "address", "/project/include/common.glsl");

            auto emptyBase = ExpectParsed("cache://address", "cache", "address", "");
            ExpectParsed(emptyBase.MergePath("main.glsl"), "cache", "address", "/main.glsl");
            // TODO: should we produce "cache://address" or "cache://address/"?
            ExpectParsed(emptyBase.MergePath("."), "cache", "address", "/");
            ExpectParsed(emptyBase.MergePath(".."), "cache", "address", "/");
            ExpectParsed(emptyBase.MergePath("./"), "cache", "address", "/");
        }

        SECTION("Rejects invalid path syntax")
        {
            auto base = ExpectParsed("file:/workspace/shaders/main.glsl", "file", "", "/workspace/shaders/main.glsl");

            CHECK(base.MergePath("bad path.glsl").has_value() == false);
            CHECK(base.MergePath("bad%2Gpath.glsl").has_value() == false);
            CHECK(base.MergePath("bad%2").has_value() == false);
            CHECK(base.MergePath("?query").has_value() == false);
            CHECK(base.MergePath("#fragment").has_value() == false);
            CHECK(base.MergePath("common.glsl?version=1").has_value() == false);
            CHECK(base.MergePath("common.glsl#main").has_value() == false);
        }
    }

    SECTION("ParsedUri::ResolveReference")
    {
        SECTION("Resolves path references against the base URI")
        {
            auto base =
                ExpectParsed("cache://workspace/project/src/main.glsl", "cache", "workspace", "/project/src/main.glsl");

            ExpectParsed(base.ResolveReference("common.glsl"), "cache", "workspace", "/project/src/common.glsl");
            ExpectParsed(base.ResolveReference("./common.glsl"), "cache", "workspace", "/project/src/common.glsl");
            ExpectParsed(base.ResolveReference("../include/common.glsl"), "cache", "workspace",
                         "/project/include/common.glsl");
            ExpectParsed(base.ResolveReference("/shared/./include/../common.glsl"), "cache", "workspace",
                         "/shared/common.glsl");
            ExpectParsed(base.ResolveReference("generated//common.glsl"), "cache", "workspace",
                         "/project/src/generated//common.glsl");
            ExpectParsed(base.ResolveReference(""), "cache", "workspace", "/project/src/main.glsl");
        }

        SECTION("Resolves authority-relative references")
        {
            auto base =
                ExpectParsed("cache://workspace/project/src/main.glsl", "cache", "workspace", "/project/src/main.glsl");

            ExpectParsed(base.ResolveReference("//other/project/main.glsl"), "cache", "other", "/project/main.glsl");
            ExpectParsed(base.ResolveReference("//other"), "cache", "other", "");
            ExpectParsed(base.ResolveReference("///absolute/path.glsl"), "cache", "", "/absolute/path.glsl");
            ExpectParsed(base.ResolveReference("//other/./path/../main.glsl"), "cache", "other", "/main.glsl");
        }

        SECTION("Uses absolute URI references directly")
        {
            auto base =
                ExpectParsed("cache://workspace/project/src/main.glsl", "cache", "workspace", "/project/src/main.glsl");

            ExpectParsed(base.ResolveReference("file:/tmp/../shader.glsl"), "file", "", "/shader.glsl");
            ExpectParsed(base.ResolveReference("file:///tmp/shader.glsl"), "file", "", "/tmp/shader.glsl");
            ExpectParsed(base.ResolveReference("mem://other/root/./shader.glsl"), "mem", "other", "/root/shader.glsl");
            ExpectParsed(base.ResolveReference("untitled:relative/../shader.glsl"), "untitled", "", "shader.glsl");
            ExpectParsed(base.ResolveReference("FILE:/Case/Path.glsl"), "FILE", "", "/Case/Path.glsl");
        }

        SECTION("Rejects invalid reference syntax")
        {
            auto base =
                ExpectParsed("cache://workspace/project/src/main.glsl", "cache", "workspace", "/project/src/main.glsl");
            CHECK(base.ResolveReference("bad path.glsl").has_value() == false);
            CHECK(base.ResolveReference("bad%2Gpath.glsl").has_value() == false);
            CHECK(base.ResolveReference("bad%2").has_value() == false);
            CHECK(base.ResolveReference("common.glsl?version=1").has_value() == false);
            CHECK(base.ResolveReference("common.glsl#main").has_value() == false);
            CHECK(base.ResolveReference("/absolute?query").has_value() == false);
            CHECK(base.ResolveReference("//bad authority/path").has_value() == false);
            CHECK(base.ResolveReference("scheme:/bad path").has_value() == false);
            CHECK(base.ResolveReference("scheme:/bad%2").has_value() == false);
        }
    }

    SECTION("DecodeUriComponent")
    {
        CHECK(DecodeUriComponent("simple") == "simple");
        CHECK(DecodeUriComponent("with%20space") == "with space");
        CHECK(DecodeUriComponent("%E4%B8%AD%20space") == "\xE4\xB8\xAD space");
        CHECK(DecodeUriComponent("%2Fslash") == "/slash");
        CHECK(DecodeUriComponent("mixed%20%2F%3Achars") == "mixed /:chars");

        // Invalid percent-encoding should result in failure.
        CHECK(DecodeUriComponent("%invalid%2Gencoding") == std::nullopt);
        CHECK(DecodeUriComponent("incomplete%2") == std::nullopt);
        CHECK(DecodeUriComponent("incomplete%2G") == std::nullopt);
        CHECK(DecodeUriComponent("%") == std::nullopt);

        // With filter that disallows space character in percent-encoded characters.
        CHECK(DecodeUriComponent("with%20space", [](char ch) { return ch != ' '; }) == std::nullopt);
        CHECK(DecodeUriComponent("%E4%B8%AD%20space", [](char ch) { return ch != ' '; }) == std::nullopt);
        CHECK(DecodeUriComponent("simple", [](char) { return false; }) == "simple");
    }
}

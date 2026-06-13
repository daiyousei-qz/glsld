#include "Catch2Wrapper.h"

#include "Support/Uri.h"

#include <filesystem>
#include <optional>
#include <vector>

using namespace glsld;

namespace
{
    auto CheckParsedUriHelper(const ParsedUri& parsed, StringView expectedScheme, StringView expectedAuthority,
                              StringView expectedPath) -> void
    {
        CHECK(parsed.GetRawScheme() == expectedScheme);
        CHECK(parsed.GetRawAuthority().GetText() == expectedAuthority);
        CHECK(parsed.GetRawPath().GetText() == expectedPath);
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

    auto ExpectNormalized(std::optional<Uri> uri, StringView expectedScheme, StringView expectedAuthority,
                          StringView expectedPath) -> Uri
    {
        REQUIRE(uri.has_value());

        auto normalized = uri->GetParsedUri().ToNormalizedUri();
        INFO(fmt::format("Seeing normalized URI: {}", normalized.GetRawText()));

        CheckParsedUriHelper(normalized.GetParsedUri(), expectedScheme, expectedAuthority, expectedPath);

        return normalized;
    }

    auto ExpectInvalidUri(StringView uri) -> void
    {
        INFO(fmt::format("Parsing URI: {}", uri));

        auto parsed = ParsedUri::Parse(uri);
        if (parsed.has_value()) {
            FAIL(fmt::format("Expected parsing to fail, but got scheme: '{}', authority: '{}', path: '{}'",
                             parsed->GetRawScheme(), parsed->GetRawAuthority().GetText(),
                             parsed->GetRawPath().GetText()));
        }
    }
} // namespace

TEST_CASE("Support::ParsedUriTest")
{
    SECTION("PercentEncodedView")
    {
        auto decodeResult = PercentEncodedView{"simple"}.DecodeFront();
        CHECK(decodeResult.decodedChar == 's');
        CHECK(decodeResult.consumedLength == 1);

        decodeResult = PercentEncodedView{"%20space"}.DecodeFront();
        CHECK(decodeResult.decodedChar == ' ');
        CHECK(decodeResult.consumedLength == 3);

        decodeResult = PercentEncodedView{"%1"}.DecodeFront();
        CHECK(decodeResult.decodedChar == '\0');
        CHECK(decodeResult.consumedLength == 0);

        decodeResult = PercentEncodedView{"%"}.DecodeFront();
        CHECK(decodeResult.decodedChar == '\0');
        CHECK(decodeResult.consumedLength == 0);

        decodeResult = PercentEncodedView{""}.DecodeFront();
        CHECK(decodeResult.decodedChar == '\0');
        CHECK(decodeResult.consumedLength == 0);

        CHECK(PercentEncodedView{"simple"}.DecodeAll() == "simple");
        CHECK(PercentEncodedView{"with%20space"}.DecodeAll() == "with space");
        CHECK(PercentEncodedView{"%E4%B8%AD%20space"}.DecodeAll() == "\xE4\xB8\xAD space");
        CHECK(PercentEncodedView{"%2Fslash"}.DecodeAll() == "/slash");
        CHECK(PercentEncodedView{"mixed%20%2F%3Achars"}.DecodeAll() == "mixed /:chars");

        // Invalid percent-encoding should result in failure.
        CHECK(PercentEncodedView{"%invalid%2Gencoding"}.DecodeAll() == std::nullopt);
        CHECK(PercentEncodedView{"incomplete%2"}.DecodeAll() == std::nullopt);
        CHECK(PercentEncodedView{"incomplete%2G"}.DecodeAll() == std::nullopt);
        CHECK(PercentEncodedView{"%"}.DecodeAll() == std::nullopt);

        CHECK(PercentEncodedView{"with%20space"}.Equals("with space"));
        CHECK(PercentEncodedView{"mixed%20%2F%3Achars"}.Equals(PercentEncodedView{"mixed%20/%3Achars"}));

        // Invalid syntax should result in non-equality even if the text is the same.
        CHECK(!PercentEncodedView{"with%space"}.Equals("with%space"));
        CHECK(!PercentEncodedView{"%GG"}.Equals(PercentEncodedView{"%GG"}));
    }

    SECTION("PercentEncode")
    {
        CHECK(PercentEncode("simple") == "simple");
        CHECK(PercentEncode("with space") == "with%20space");
        CHECK(PercentEncode("percent% hash#") == "percent%25%20hash%23");
        CHECK(PercentEncode("啊") == "%E5%95%8A");
    }

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

    SECTION("ParsedUri::MergePath")
    {
        SECTION("Without authority")
        {
            auto fileBase =
                ExpectParsed("file:/workspace/shaders/main.glsl", "file", "", "/workspace/shaders/main.glsl");
            ExpectParsed(fileBase.MergePath("common.glsl"), "file", "", "/workspace/shaders/common.glsl");
            ExpectParsed(fileBase.MergePath("includes/lighting.glsl"), "file", "",
                         "/workspace/shaders/includes/lighting.glsl");
            ExpectParsed(fileBase.MergePath("."), "file", "", "/workspace/shaders/.");
            ExpectParsed(fileBase.MergePath(".."), "file", "", "/workspace/shaders/..");
            ExpectParsed(fileBase.MergePath("./"), "file", "", "/workspace/shaders/./");
            ExpectParsed(fileBase.MergePath("../"), "file", "", "/workspace/shaders/../");
            ExpectParsed(fileBase.MergePath("../../test.glsl"), "file", "", "/workspace/shaders/../../test.glsl");
            ExpectParsed(fileBase.MergePath("/shared/./include/../common.glsl"), "file", "",
                         "/shared/./include/../common.glsl");
            // Empty path should be preserved for URI handling.
            ExpectParsed(fileBase.MergePath(".///test.glsl"), "file", "", "/workspace/shaders/.///test.glsl");

            auto directoryBase = ExpectParsed("file:/workspace/shaders/", "file", "", "/workspace/shaders/");
            ExpectParsed(directoryBase.MergePath("lighting.glsl"), "file", "", "/workspace/shaders/lighting.glsl");
            ExpectParsed(directoryBase.MergePath("."), "file", "", "/workspace/shaders/.");
            ExpectParsed(directoryBase.MergePath(".."), "file", "", "/workspace/shaders/..");
            ExpectParsed(directoryBase.MergePath(".//"), "file", "", "/workspace/shaders/.//");
            ExpectParsed(directoryBase.MergePath("../..//"), "file", "", "/workspace/shaders/../..//");

            auto baseWithDotSegments = ExpectParsed("file:/workspace/shaders/./generated/../main.glsl", "file", "",
                                                    "/workspace/shaders/./generated/../main.glsl");
            // MergePath appends to all but the last base path segment.
            ExpectParsed(baseWithDotSegments.MergePath("../include/common.glsl"), "file", "",
                         "/workspace/shaders/./generated/../../include/common.glsl");

            auto rootlessBase = ExpectParsed("untitled:main.glsl", "untitled", "", "main.glsl");
            ExpectParsed(rootlessBase.MergePath("common.glsl"), "untitled", "", "common.glsl");
            ExpectParsed(rootlessBase.MergePath("."), "untitled", "", ".");
            ExpectParsed(rootlessBase.MergePath(".."), "untitled", "", "..");

            auto emptyBase = ExpectParsed("untitled:", "untitled", "", "");
            ExpectParsed(emptyBase.MergePath("path/to/resource"), "untitled", "", "path/to/resource");
            ExpectParsed(emptyBase.MergePath("."), "untitled", "", ".");
            ExpectParsed(emptyBase.MergePath(".."), "untitled", "", "..");
        }

        SECTION("Preserves authority while merging")
        {
            auto base =
                ExpectParsed("cache://address/project/src/main.glsl", "cache", "address", "/project/src/main.glsl");
            ExpectParsed(base.MergePath("include/common.glsl"), "cache", "address", "/project/src/include/common.glsl");
            ExpectParsed(base.MergePath("../include/common.glsl"), "cache", "address",
                         "/project/src/../include/common.glsl");

            auto emptyBase = ExpectParsed("cache://address", "cache", "address", "");
            ExpectParsed(emptyBase.MergePath("main.glsl"), "cache", "address", "/main.glsl");
            // TODO: should we produce "cache://address" or "cache://address/"?
            ExpectParsed(emptyBase.MergePath("."), "cache", "address", "/.");
            ExpectParsed(emptyBase.MergePath(".."), "cache", "address", "/..");
            ExpectParsed(emptyBase.MergePath("./"), "cache", "address", "/./");
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
            ExpectParsed(base.ResolveReference("./common.glsl"), "cache", "workspace", "/project/src/./common.glsl");
            ExpectParsed(base.ResolveReference("../include/common.glsl"), "cache", "workspace",
                         "/project/src/../include/common.glsl");
            ExpectParsed(base.ResolveReference("/shared/./include/../common.glsl"), "cache", "workspace",
                         "/shared/./include/../common.glsl");
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
            ExpectParsed(base.ResolveReference("//other/./path/../main.glsl"), "cache", "other",
                         "/./path/../main.glsl");
        }

        SECTION("Uses absolute URI references directly")
        {
            auto base =
                ExpectParsed("cache://workspace/project/src/main.glsl", "cache", "workspace", "/project/src/main.glsl");

            ExpectParsed(base.ResolveReference("file:/tmp/../shader.glsl"), "file", "", "/tmp/../shader.glsl");
            ExpectParsed(base.ResolveReference("file:///tmp/shader.glsl"), "file", "", "/tmp/shader.glsl");
            ExpectParsed(base.ResolveReference("mem://other/root/./shader.glsl"), "mem", "other",
                         "/root/./shader.glsl");
            ExpectParsed(base.ResolveReference("untitled:relative/../shader.glsl"), "untitled", "",
                         "relative/../shader.glsl");
            ExpectParsed(base.ResolveReference("FILE:/Case/Path.glsl"), "FILE", "", "/Case/Path.glsl");
        }

        SECTION("Normalizes supported RFC 3986 reference-resolution examples")
        {
            // Query and fragment are intentionally unsupported, so this mirrors the RFC 3986 examples without "?q".
            auto base = ExpectParsed("http://a/b/c/d;p", "http", "a", "/b/c/d;p");

            ExpectNormalized(base.ResolveReference("g:h"), "g", "", "h");
            ExpectNormalized(base.ResolveReference("g"), "http", "a", "/b/c/g");
            ExpectNormalized(base.ResolveReference("./g"), "http", "a", "/b/c/g");
            ExpectNormalized(base.ResolveReference("g/"), "http", "a", "/b/c/g/");
            ExpectNormalized(base.ResolveReference("/g"), "http", "a", "/g");
            ExpectNormalized(base.ResolveReference("//g"), "http", "g", "");
            ExpectNormalized(base.ResolveReference(";x"), "http", "a", "/b/c/;x");
            ExpectNormalized(base.ResolveReference("g;x"), "http", "a", "/b/c/g;x");
            ExpectNormalized(base.ResolveReference(""), "http", "a", "/b/c/d;p");
            ExpectNormalized(base.ResolveReference("."), "http", "a", "/b/c/");
            ExpectNormalized(base.ResolveReference("./"), "http", "a", "/b/c/");
            ExpectNormalized(base.ResolveReference(".."), "http", "a", "/b/");
            ExpectNormalized(base.ResolveReference("../"), "http", "a", "/b/");
            ExpectNormalized(base.ResolveReference("../g"), "http", "a", "/b/g");
            ExpectNormalized(base.ResolveReference("../.."), "http", "a", "/");
            ExpectNormalized(base.ResolveReference("../../"), "http", "a", "/");
            ExpectNormalized(base.ResolveReference("../../g"), "http", "a", "/g");
            ExpectNormalized(base.ResolveReference("../../../g"), "http", "a", "/g");
            ExpectNormalized(base.ResolveReference("../../../../g"), "http", "a", "/g");
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

    SECTION("UriToFileSystemPath")
    {
        SECTION("Converts file URIs without authority to filesystem paths")
        {
#if GLSLD_OS_WIN
            {
                auto driveAbsoluteFile = ExpectParsed("file:/C:/tmp/shader.glsl", "file", "", "/C:/tmp/shader.glsl");
                auto path              = UriToFileSystemPath(driveAbsoluteFile);
                REQUIRE(path.has_value());
                CHECK(std::filesystem::path{*path}.is_absolute());
                CHECK(std::filesystem::path{*path}.string() == "C:/tmp/shader.glsl");
            }

            {
                auto driveRelativeFile = ExpectParsed("file:C:/tmp/shader.glsl", "file", "", "C:/tmp/shader.glsl");
                auto path              = UriToFileSystemPath(driveRelativeFile);
                REQUIRE(path.has_value());
                CHECK(std::filesystem::path{*path}.is_absolute());
                CHECK(std::filesystem::path{*path}.string() == "C:/tmp/shader.glsl");
            }
#endif

            {
                auto absoluteFile = ExpectParsed("file:/tmp/shader.glsl", "file", "", "/tmp/shader.glsl");
                auto path         = UriToFileSystemPath(absoluteFile);
                REQUIRE(path.has_value());
                CHECK(std::filesystem::path{*path}.has_root_path());
                CHECK(std::filesystem::path{*path}.string() == "/tmp/shader.glsl");
            }

            {
                // We don't collapse multiple slashes in the path. We expect OS to handle it correctly.
                auto multipleSlashes = ExpectParsed("file:////tmp///shader.glsl", "file", "", "//tmp///shader.glsl");
                auto path            = UriToFileSystemPath(multipleSlashes);
                REQUIRE(path.has_value());
                CHECK(std::filesystem::path{*path}.has_root_path());
                CHECK(std::filesystem::path{*path}.string() == "//tmp///shader.glsl");
            }

            {
                auto upperCaseFile = ExpectParsed("FILE:/tmp/shader.glsl", "FILE", "", "/tmp/shader.glsl");
                auto path          = UriToFileSystemPath(upperCaseFile);
                REQUIRE(path.has_value());
                CHECK(std::filesystem::path{*path}.has_root_path());
                CHECK(std::filesystem::path{*path}.string() == "/tmp/shader.glsl");
            }

            {
                // Rootless paths are not allowed to be converted to filesystem paths.
                CHECK(
                    !UriToFileSystemPath(ExpectParsed("file:relative/shader.glsl", "file", "", "relative/shader.glsl"))
                         .has_value());

                // '/' is not allowed in percent-encoding when converting to filesystem path.
                CHECK(
                    !UriToFileSystemPath(ExpectParsed("file:/tmp/bad%2fpath.glsl", "file", "", "/tmp/bad%2fpath.glsl"))
                         .has_value());
                // '\\' is not allowed in percent-encoding when converting to filesystem path.
                CHECK(
                    !UriToFileSystemPath(ExpectParsed("file:/tmp/bad%5cpath.glsl", "file", "", "/tmp/bad%5cpath.glsl"))
                         .has_value());
                // '\0' is not allowed in percent-encoding when converting to filesystem path.
                CHECK(
                    !UriToFileSystemPath(ExpectParsed("file:/tmp/bad%00path.glsl", "file", "", "/tmp/bad%00path.glsl"))
                         .has_value());

                // FIXME: what about percent-decoded '.'?
            }
        }

        SECTION("Rejects filesystem conversion for non-file or authority URIs")
        {
            auto nonFile = ExpectParsed("untitled:shader.glsl", "untitled", "", "shader.glsl");
            auto fileWithAuthority =
                ExpectParsed("file://example.com/tmp/shader.glsl", "file", "example.com", "/tmp/shader.glsl");

            CHECK(!UriToFileSystemPath(nonFile).has_value());
            CHECK(!UriToFileSystemPath(fileWithAuthority).has_value());
        }
    }

    SECTION("FileSystemPathToUri")
    {
        SECTION("Converts filesystem paths to file URIs")
        {
#if GLSLD_OS_WIN
            ExpectParsed(FileSystemPathToUri("C:\\tmp\\shader.glsl"), "file", "", "/C:/tmp/shader.glsl");
            ExpectParsed(FileSystemPathToUri("C:/tmp/shader.glsl"), "file", "", "/C:/tmp/shader.glsl");
            auto escaped          = ExpectParsed(FileSystemPathToUri("C:\\tmp\\with space\\%#?.glsl"), "file", "",
                                                 "/C:/tmp/with%20space/%25%23%3F.glsl");
            auto roundTrippedPath = UriToFileSystemPath(escaped.GetParsedUri());
            REQUIRE(roundTrippedPath.has_value());
            CHECK(std::filesystem::path{*roundTrippedPath}.string() == "C:/tmp/with space/%#?.glsl");
#else
            auto absoluteFile = ExpectParsed(FileSystemPathToUri("/tmp/shader.glsl"), "file", "", "/tmp/shader.glsl");
            CHECK(absoluteFile.GetRawText() == "file:/tmp/shader.glsl");

            auto escaped          = ExpectParsed(FileSystemPathToUri("/tmp/with space/%#?.glsl"), "file", "",
                                                 "/tmp/with%20space/%25%23%3F.glsl");
            auto roundTrippedPath = UriToFileSystemPath(escaped.GetParsedUri());
            REQUIRE(roundTrippedPath.has_value());
            CHECK(*roundTrippedPath == "/tmp/with space/%#?.glsl");
#endif
        }

        SECTION("Appends trailing slash for directories")
        {
#if GLSLD_OS_WIN
            ExpectParsed(FileSystemPathToUri("C:\\tmp\\path", true), "file", "", "/C:/tmp/path/");
            ExpectParsed(FileSystemPathToUri("C:\\tmp\\path\\", true), "file", "", "/C:/tmp/path/");
            ExpectParsed(FileSystemPathToUri("C:\\", true), "file", "", "/C:/");
#else
            ExpectParsed(FileSystemPathToUri("/tmp/path", true), "file", "", "/tmp/path/");
            ExpectParsed(FileSystemPathToUri("/tmp/path/", true), "file", "", "/tmp/path/");
            ExpectParsed(FileSystemPathToUri("/", true), "file", "", "/");
#endif
        }

        SECTION("Preserves dot segments")
        {
#if GLSLD_OS_WIN
            ExpectParsed(FileSystemPathToUri("C:\\workspace\\.\\shaders\\..\\main.glsl"), "file", "",
                         "/C:/workspace/./shaders/../main.glsl");
            ExpectParsed(FileSystemPathToUri("C:\\workspace\\.\\shaders\\..", true), "file", "",
                         "/C:/workspace/./shaders/../");
#else
            ExpectParsed(FileSystemPathToUri("/workspace/./shaders/../main.glsl"), "file", "",
                         "/workspace/./shaders/../main.glsl");
            ExpectParsed(FileSystemPathToUri("/workspace/./shaders/..", true), "file", "", "/workspace/./shaders/../");
#endif
        }

        SECTION("Rejects relative paths")
        {
#if GLSLD_OS_WIN
            CHECK(!FileSystemPathToUri("C:relative\\shader.glsl").has_value());
            CHECK(!FileSystemPathToUri("relative\\shader.glsl").has_value());
#else
            CHECK(!FileSystemPathToUri("relative/shader.glsl").has_value());
#endif
            CHECK(!FileSystemPathToUri("", true).has_value());
        }

        SECTION("Rejects embedded null characters")
        {
            std::string badPath{"bad\0path.glsl", sizeof("bad\0path.glsl") - 1};
            CHECK(!FileSystemPathToUri(StringView{badPath}).has_value());
        }
    }
}

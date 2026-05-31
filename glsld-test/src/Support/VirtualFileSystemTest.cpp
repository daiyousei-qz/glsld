#include "Catch2Wrapper.h"

#include "Support/VirtualFileSystem.h"

using namespace glsld;

namespace
{
    auto ParseUri(StringView uri) -> ParsedUri
    {
        auto parsedUri = ParsedUri::Parse(uri);
        REQUIRE(parsedUri.has_value());
        return *parsedUri;
    }
} // namespace

TEST_CASE("Support::VirtualFileSystemTest")
{
    SECTION("NativeFileSystem")
    {
        NativeFileSystem vfs;

        // FIXME: we should also add positive tests
        CHECK(vfs.Exists("invalid_uri") == std::unexpected(FileSystemError::InvalidUri));
        CHECK(vfs.Exists("file:/_____nonexistent_file.txt") == std::unexpected(FileSystemError::NotFound));
        CHECK(vfs.ReadAllText("invalid_uri") == std::unexpected(FileSystemError::InvalidUri));
        CHECK(vfs.ReadAllText("file:/nonexistent_file.txt") == std::unexpected(FileSystemError::NotFound));
    }

    SECTION("InMemoryFileSystem")
    {
        SECTION("POSIX")
        {
            InMemoryFileSystem vfs;
            REQUIRE(vfs.AddFile("mem:/tmp/test1.txt", "TestData").has_value());
            std::array<char, 5> localBuffer = {'H', 'e', 'l', 'l', 'o'};
            REQUIRE(
                vfs.AddFileNoOwn("mem:/tmp/test2.txt", StringView{localBuffer.data(), localBuffer.size()}).has_value());

            CHECK(vfs.Exists("mem:/tmp/test1.txt"));
            CHECK(vfs.Exists("mem:/tmp/test2.txt"));
            CHECK(!vfs.Exists("mem:/tmp/nonexistent.txt"));
            CHECK(!vfs.Exists("relative/test.txt"));

            auto content1 = vfs.ReadAllText("mem:/tmp/test1.txt");
            REQUIRE(content1.has_value());
            CHECK(content1.value() == "TestData");

            localBuffer   = {'W', 'o', 'r', 'l', 'd'};
            auto content2 = vfs.ReadAllText("mem:/tmp/test2.txt");
            REQUIRE(content2.has_value());
            CHECK(content2.value() == "World");

            auto content3 = vfs.ReadAllText("mem:/tmp/nonexistent.txt");
            REQUIRE(!content3.has_value());
            CHECK(content3.error() == FileSystemError::NotFound);

            auto content4 = vfs.ReadAllText("mem:/tmp");
            REQUIRE(!content4.has_value());
            CHECK(content4.error() == FileSystemError::IsADirectory);
        }

        SECTION("NonFileUri")
        {
            InMemoryFileSystem vfs;
            REQUIRE(vfs.AddFile(ParseUri("untitled:tmp/test1.txt"), "TestData").has_value());
            REQUIRE(vfs.AddFile(ParseUri("glsld-cache://main/include/test.glsl"), "CachedData").has_value());

            CHECK(vfs.Exists("untitled:tmp/test1.txt"));
            CHECK(vfs.Exists("glsld-cache://main/include/test.glsl"));
            CHECK(!vfs.Exists("untitled:tmp/nonexistent.txt"));
            CHECK(!vfs.Exists("glsld-cache://other/include/test.glsl"));

            auto content1 = vfs.ReadAllText("untitled:tmp/test1.txt");
            REQUIRE(content1.has_value());
            CHECK(content1.value() == "TestData");

            auto content2 = vfs.ReadAllText("glsld-cache://main/include/test.glsl");
            REQUIRE(content2.has_value());
            CHECK(content2.value() == "CachedData");

            auto content3 = vfs.ReadAllText("untitled:tmp");
            REQUIRE(!content3.has_value());
            CHECK(content3.error() == FileSystemError::IsADirectory);
        }
    }

    SECTION("OverlayFileSystem")
    {
        OverlayFileSystem overlayVfs;
        overlayVfs.AddLayerWithInit<InMemoryFileSystem>(
            [](InMemoryFileSystem& vfs) { REQUIRE(vfs.AddFile("mem:/base/file.txt", "BaseContent1").has_value()); });
        overlayVfs.AddLayerWithInit<InMemoryFileSystem>([](InMemoryFileSystem& vfs) {
            REQUIRE(vfs.AddFile("mem:/base/file.txt", "BaseContent2").has_value());
            REQUIRE(vfs.AddFile("mem:/tmp/otherFile.txt", "BaseContent2").has_value());
        });

        CHECK(overlayVfs.Exists("mem:/base/file.txt"));
        CHECK(overlayVfs.Exists("mem:/tmp/otherFile.txt"));
        CHECK(!overlayVfs.Exists("mem:/nonexistent.txt"));

        auto content1 = overlayVfs.ReadAllText("mem:/base/file.txt");
        REQUIRE(content1.has_value());
        CHECK(content1.value() == "BaseContent1");

        auto content2 = overlayVfs.ReadAllText("mem:/tmp/otherFile.txt");
        REQUIRE(content2.has_value());
        CHECK(content2.value() == "BaseContent2");

        auto content3 = overlayVfs.ReadAllText("mem:/nonexistent.txt");
        REQUIRE(!content3.has_value());
    }
}

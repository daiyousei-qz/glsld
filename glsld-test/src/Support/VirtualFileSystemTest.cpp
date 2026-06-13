#include "Catch2Wrapper.h"

#include "Support/VirtualFileSystem.h"

using namespace glsld;

TEST_CASE("Support::VirtualFileSystemTest")
{
    SECTION("NativeFileSystem")
    {
        auto vfs = NativeFileSystem::Create();
        REQUIRE(vfs);

        // FIXME: we should also add positive tests
        CHECK(vfs->Exists("invalid_uri") == std::unexpected(FileSystemError::InvalidUri));
        CHECK(vfs->Exists("file:/_____nonexistent_file.txt") == std::unexpected(FileSystemError::NotFound));
        CHECK(vfs->ReadAllText("invalid_uri") == std::unexpected(FileSystemError::InvalidUri));
        CHECK(vfs->ReadAllText("file:/nonexistent_file.txt") == std::unexpected(FileSystemError::NotFound));
    }

    SECTION("InMemoryFileSystem")
    {
        auto vfs = InMemoryFileSystem::Create("mem:");
        REQUIRE(vfs->AddFile("mem:/tmp/test1.txt", "TestData").has_value());
        std::array<char, 5> localBuffer = {'H', 'e', 'l', 'l', 'o'};
        REQUIRE(
            vfs->AddFileNoOwn("mem:/tmp/test2.txt", StringView{localBuffer.data(), localBuffer.size()}).has_value());

        SECTION("Basic")
        {
            CHECK(vfs->Exists("mem:/tmp"));
            CHECK(vfs->Exists("mem:/tmp/test1.txt"));
            CHECK(vfs->Exists("mem:/tmp/test2.txt"));
            CHECK(vfs->Exists("mem:/tmp/nonexistent.txt") == std::unexpected(FileSystemError::NotFound));
            CHECK(vfs->Exists("relative/test.txt") == std::unexpected(FileSystemError::InvalidUri));

            CHECK(vfs->ReadAllText("mem:/tmp/test1.txt") == "TestData");

            localBuffer = {'W', 'o', 'r', 'l', 'd'};
            CHECK(vfs->ReadAllText("mem:/tmp/test2.txt") == "World");

            CHECK(vfs->ReadAllText("mem:/tmp/nonexistent.txt") == std::unexpected(FileSystemError::NotFound));
            CHECK(vfs->ReadAllText("mem:/tmp") == std::unexpected(FileSystemError::IsADirectory));
        }

        SECTION("PathVariations")
        {
            CHECK(vfs->Exists("mem:///tmp///./////"));
            CHECK(vfs->Exists("mem:/tmp/./test1.txt"));
            CHECK(vfs->Exists("mem:/tmp/../tmp/test1.txt"));
            CHECK(vfs->Exists("mem:/tmp/./../tmp/./test1.txt"));
            CHECK(vfs->Exists("mem:/tmp//test1.txt"));

            CHECK(vfs->ReadAllText("mem:/tmp//./test1.txt") == "TestData");

            // Even though this should resolve to the correct file, inexistent intermediate directories should cause it
            // to fail.
            CHECK(vfs->ReadAllText("mem:/tmp/include/../test1.txt") == std::unexpected(FileSystemError::NotFound));
        }

        SECTION("BadMountPoint")
        {

            CHECK(InMemoryFileSystem::Create("invalid_scheme") == nullptr);
            CHECK(InMemoryFileSystem::Create("tmp:/path_must_be_empty") == nullptr);

            CHECK(vfs->AddFile("mem:relative/test1.txt", "TestData") == std::unexpected(FileSystemError::InvalidUri));
            CHECK(vfs->AddFile("bad-scheme:/include/test.glsl", "CachedData") ==
                  std::unexpected(FileSystemError::InvalidUri));
            CHECK(vfs->AddFile("mem://bad_authority/include/test.glsl", "CachedData") ==
                  std::unexpected(FileSystemError::InvalidUri));

            CHECK(vfs->Exists("untitled:/tmp/test1.txt") == std::unexpected(FileSystemError::InvalidUri));
            CHECK(vfs->ReadAllText("untitled:tmp/test1.txt") == std::unexpected(FileSystemError::InvalidUri));
        }
    }

    SECTION("OverlayFileSystem")
    {
        auto inMemoryVfs1 = InMemoryFileSystem::Create("mem:");
        REQUIRE(inMemoryVfs1);
        REQUIRE(inMemoryVfs1->AddFile("mem:/base/file.txt", "BaseContent1").has_value());

        auto inMemoryVfs2 = InMemoryFileSystem::Create("mem:");
        REQUIRE(inMemoryVfs2);
        REQUIRE(inMemoryVfs2->AddFile("mem:/base/file.txt", "BaseContent2").has_value());
        REQUIRE(inMemoryVfs2->AddFile("mem:/tmp/otherFile.txt", "BaseContent2").has_value());

        auto overlayVfs = OverlayFileSystem::Create({inMemoryVfs1, inMemoryVfs2});
        REQUIRE(overlayVfs);

        CHECK(overlayVfs->Exists("mem:/base/file.txt"));
        CHECK(overlayVfs->Exists("mem:/tmp/otherFile.txt"));
        CHECK(!overlayVfs->Exists("mem:/nonexistent.txt"));

        auto content1 = overlayVfs->ReadAllText("mem:/base/file.txt");
        REQUIRE(content1.has_value());
        CHECK(content1.value() == "BaseContent1");

        auto content2 = overlayVfs->ReadAllText("mem:/tmp/otherFile.txt");
        REQUIRE(content2.has_value());
        CHECK(content2.value() == "BaseContent2");

        auto content3 = overlayVfs->ReadAllText("mem:/nonexistent.txt");
        REQUIRE(!content3.has_value());
    }
}

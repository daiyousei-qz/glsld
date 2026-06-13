#pragma once
#include "Support/StringMap.h"
#include "Support/Uri.h"

#include <fmt/format.h>

#include <deque>
#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace glsld
{
    // Just a tag for now, we can add more info to it later if needed.
    enum class FileSystemError
    {
        Unknown,
        // Indicates the URI is malformed or not supported by the file system.
        InvalidUri,
        // Indicates the specified resource doesn't exist.
        NotFound,
        // Indicates that the specified resource could not be accessed due to insufficient permissions.
        PermissionDenied,
        // Indicates that the specified resource is a directory, but a file was expected.
        IsADirectory,
        // Indicates that the specified resource is a file, but a directory was expected.
        NotADirectory,
        // Indicates an input/output error occurred while accessing the resource.
        IOError,
    };

    // Note this isn't a complete VFS implementation, but only provides the necessary operations for our need.
    class VirtualFileSystem
    {
    public:
        VirtualFileSystem()          = default;
        virtual ~VirtualFileSystem() = default;

        // VFS should be pined in the same location to ensure the pointer stability.
        VirtualFileSystem(const VirtualFileSystem&)            = delete;
        VirtualFileSystem& operator=(const VirtualFileSystem&) = delete;

        auto Exists(StringView uri) -> std::expected<std::monostate, FileSystemError>;

        auto Exists(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError>;

        auto ReadAllText(StringView uri) -> std::expected<std::string, FileSystemError>;

        auto ReadAllText(const ParsedUri& uri) -> std::expected<std::string, FileSystemError>;

    protected:
        virtual auto ExistsImpl(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError>   = 0;
        virtual auto ReadAllTextImpl(const ParsedUri& uri) -> std::expected<std::string, FileSystemError> = 0;
    };

    class NativeFileSystem : public VirtualFileSystem
    {
    public:
        NativeFileSystem() = default;

        static auto Create() -> std::shared_ptr<NativeFileSystem>
        {
            return std::make_shared<NativeFileSystem>();
        }

    protected:
        auto ExistsImpl(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError> override;
        auto ReadAllTextImpl(const ParsedUri& uri) -> std::expected<std::string, FileSystemError> override;
    };

    class InMemoryFileSystem : public VirtualFileSystem
    {
    private:
        struct DirectoryOrFile
        {
            DirectoryOrFile* parent = nullptr;

            UnorderedStringMap<DirectoryOrFile> children = {};

            std::optional<StringView> fileContent = {};
        };

        std::deque<std::string> ownedFiles;

        std::string mountScheme;
        std::string mountAuthority;
        DirectoryOrFile root{.parent = &root};

        auto CheckMountPoint(const ParsedUri& uri) -> bool
        {
            return uri.TestScheme(mountScheme) && uri.GetRawAuthority().Equals(mountAuthority) &&
                   uri.GetRawPath().GetText().StartWith('/');
        }

        auto GetPathEntry(UriPathSegmentView pathSegments) -> DirectoryOrFile*;

        auto GetOrCreatePathEntry(UriPathSegmentView pathSegments) -> DirectoryOrFile*;

    public:
        InMemoryFileSystem(const ParsedUri& mountPoint)
            : mountScheme(mountPoint.GetNormalizedScheme()), mountAuthority(mountPoint.GetRawAuthority().GetText())
        {
        }

        static auto Create(StringView mountPoint) -> std::shared_ptr<InMemoryFileSystem>
        {
            auto parsedMountPoint = ParsedUri::Parse(mountPoint);
            if (!parsedMountPoint || !parsedMountPoint->GetRawPath().GetText().empty()) {
                return nullptr;
            }
            return std::make_shared<InMemoryFileSystem>(*parsedMountPoint);
        }

        // Adds a file with the given content to the specified path.
        auto AddFile(StringView uri, std::string content) -> std::expected<void, FileSystemError>;
        auto AddFile(ParsedUri uri, std::string content) -> std::expected<void, FileSystemError>;

        // Adds a file with the given content to the specified path, but does not take ownership of the content. The
        // caller is responsible for ensuring that the content remains valid as long as it's needed.
        auto AddFileNoOwn(StringView uri, StringView content) -> std::expected<void, FileSystemError>;
        auto AddFileNoOwn(ParsedUri uri, StringView content) -> std::expected<void, FileSystemError>;

    protected:
        auto ExistsImpl(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError> override;
        auto ReadAllTextImpl(const ParsedUri& uri) -> std::expected<std::string, FileSystemError> override;
    };

    class OverlayFileSystem : public VirtualFileSystem
    {
    private:
        std::vector<std::shared_ptr<VirtualFileSystem>> layers;

    public:
        OverlayFileSystem() = default;
        OverlayFileSystem(std::vector<std::shared_ptr<VirtualFileSystem>> layerList) : layers(std::move(layerList))
        {
        }

        static auto Create(std::vector<std::shared_ptr<VirtualFileSystem>> layerList)
            -> std::shared_ptr<OverlayFileSystem>
        {
            return std::make_shared<OverlayFileSystem>(std::move(layerList));
        }

    protected:
        auto ExistsImpl(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError> override;
        auto ReadAllTextImpl(const ParsedUri& uri) -> std::expected<std::string, FileSystemError> override;
    };

    template <typename T>
    inline auto SinkFileSystemError(std::expected<T, FileSystemError> result) -> void
    {
        if (!result) {
            throw std::runtime_error(fmt::format("File system error: {}", static_cast<int>(result.error())));
        }
    }
} // namespace glsld

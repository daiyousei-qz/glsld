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
        InvalidUri,
        NotFound,
        PermissionDenied,
        IsADirectory,
        NotADirectory,
        IOError,
    };

    // Note this isn't a complete VFS implementation, but only provides the necessary operations for our need.
    class VirtualFileSystem
    {
    public:
        VirtualFileSystem()          = default;
        virtual ~VirtualFileSystem() = default;

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

    protected:
        auto ExistsImpl(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError> override;
        auto ReadAllTextImpl(const ParsedUri& uri) -> std::expected<std::string, FileSystemError> override;
    };

    class InMemoryFileSystem : public VirtualFileSystem
    {
    private:
        struct DirectoryOrFile
        {
            UnorderedStringMap<DirectoryOrFile> children = {};

            std::optional<StringView> fileContent = {};
        };

        std::deque<std::string> ownedFiles;

        // Maps normalized (scheme, authority) to the root directory of the path
        UnorderedStringMap<DirectoryOrFile> uriRoots;

        auto GetPathEntry(const ParsedUri& uri) -> DirectoryOrFile*;

        auto GetOrCreatePathEntry(const ParsedUri& uri) -> DirectoryOrFile*;

    public:
        InMemoryFileSystem() = default;

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

        template <typename VfsType>
        auto AddLayer() -> void
        {
            layers.push_back(std::make_shared<VfsType>());
        }

        // TODO: use std::function_ref
        template <typename VfsType, typename Fn>
        auto AddLayerWithInit(Fn initFn) -> void
        {
            auto layer = std::make_shared<VfsType>();
            initFn(*layer);
            layers.push_back(std::move(layer));
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

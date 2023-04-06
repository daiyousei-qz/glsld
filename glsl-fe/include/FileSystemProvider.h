#pragma once
#include "StringView.h"
#include "MemoryBuffer.h"

#include <mutex>
#include <string>
#include <memory>
#include <optional>
#include <unordered_map>

namespace glsld
{
    using FileID = int;

    class FileEntry
    {
    public:
        FileEntry(FileID id, const char* data, size_t size) : id(id), data(data), size(size)
        {
        }

        auto GetID() const -> FileID
        {
            return id;
        }

        auto GetData() const -> const char*
        {
            return data;
        }

        auto GetSize() const -> size_t
        {
            return size;
        }

    private:
        FileID id;

        const char* data;

        size_t size;
    };

    // This abstract interface is used to provide file system access to the compiler, allowing downstream users to
    // provide caching, virtual file systems, etc.
    class FileSystemProvider
    {
    public:
        FileSystemProvider()          = default;
        virtual ~FileSystemProvider() = default;

        // Open a file and return a FileEntry object. The returned FileEntry object needs to be closed by calling Close.
        // If the file cannot be opened, the function returns nullptr.
        virtual auto Open(StringView uri) -> const FileEntry* = 0;

        // Close a file entry. The file entry must have been returned by calling Open of this file system provider.
        virtual auto Close(const FileEntry* fileEntry) -> void = 0;
    };

    class DefaultFileSystemProvider final : public FileSystemProvider
    {
    public:
        DefaultFileSystemProvider()           = default;
        ~DefaultFileSystemProvider() override = default;

        static auto GetInstance() -> DefaultFileSystemProvider&;

        auto Open(StringView uri) -> const FileEntry* override;
        auto Close(const FileEntry* fileEntry) -> void override;
    };

} // namespace glsld
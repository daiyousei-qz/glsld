#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"

namespace glsld
{
    //
    class FileHandle
    {
    public:
        FileHandle()          = default;
        virtual ~FileHandle() = default;

        virtual auto GetData() const -> const char* = 0;
        virtual auto GetSize() const -> size_t      = 0;
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
        virtual auto Open(StringView path) -> const FileHandle* = 0;

        // Close a file entry. The file entry must have been returned by calling Open of this file system provider.
        virtual auto Close(const FileHandle* file) -> void = 0;
    };

    class DefaultFileHandle : public FileHandle
    {
    public:
        DefaultFileHandle(const char* data, size_t size) : data(data), size(size)
        {
        }

        virtual auto GetData() const -> const char* override
        {
            return data;
        }

        virtual auto GetSize() const -> size_t override
        {
            return size;
        }

    private:
        const char* data;
        size_t size;
    };

    class DefaultFileSystemProvider final : public FileSystemProvider
    {
    public:
        DefaultFileSystemProvider()           = default;
        ~DefaultFileSystemProvider() override = default;

        static auto GetInstance() -> DefaultFileSystemProvider&;

        auto Open(StringView uri) -> const FileHandle* override;
        auto Close(const FileHandle* file) -> void override;
    };

} // namespace glsld
#pragma once
#include "Support/StringView.h"

namespace glsld
{
    //
    class FileHandle
    {
    public:
        FileHandle()          = default;
        virtual ~FileHandle() = default;

        virtual auto GetContent() const -> StringView = 0;
    };

    // This abstract interface is used to provide file system access to the compiler, allowing downstream users to
    // provide caching, virtual file systems, etc.
    class FileSystemProvider
    {
    public:
        FileSystemProvider()          = default;
        virtual ~FileSystemProvider() = default;

        // Open a file and return a handle object. The returned handle object needs to be closed by calling Close.
        // If the file cannot be opened, the function returns nullptr.
        virtual auto Open(StringView path) -> const FileHandle* = 0;

        // Close an opened file. The file handle must have been returned by calling Open of this file system provider.
        virtual auto Close(const FileHandle* file) -> void = 0;
    };

    class DefaultFileHandle : public FileHandle
    {
    public:
        DefaultFileHandle(const char* data, size_t size) : data(data), size(size)
        {
        }
        ~DefaultFileHandle() override
        {
            if (data) {
                delete[] data;
            }
        }

        virtual auto GetContent() const -> StringView override
        {
            return StringView{data, size};
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
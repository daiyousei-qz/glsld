#pragma once
#include "Basic/FileSystemProvider.h"

namespace glsld
{
    class GlsldFileEntry : public FileHandle
    {
    };

    class GlsldFileSystemProvider : public FileSystemProvider
    {
    public:
        virtual auto Open(StringView path) -> const FileHandle* override
        {
        }

        virtual auto Close(const FileHandle* fileEntry) -> void override
        {
        }

    private:
    };
} // namespace glsld
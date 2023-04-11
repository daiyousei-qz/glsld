#pragma once
#include "FileSystemProvider.h"

namespace glsld
{
    class GlsldFileEntry : public FileRef
    {
    };

    class GlsldFileSystemProvider : public FileSystemProvider
    {
    public:
        virtual auto Open(StringView path) -> const FileRef* override
        {
        }

        virtual auto Close(const FileRef* fileEntry) -> void override
        {
        }

    private:
    };
} // namespace glsld
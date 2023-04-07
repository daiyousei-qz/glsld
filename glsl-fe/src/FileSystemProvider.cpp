#include "FileSystemProvider.h"

#include <fstream>

namespace glsld
{
    auto DefaultFileSystemProvider::GetInstance() -> DefaultFileSystemProvider&
    {
        static DefaultFileSystemProvider instance;
        return instance;
    }

    auto DefaultFileSystemProvider::Open(StringView uri) -> const FileEntry*
    {
        std::ifstream file{uri.Str(), std::ios::binary | std::ios::ate};

        if (!file.is_open()) {
            return nullptr;
        }

        file.seekg(0, std::ios::end);
        auto size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);

        auto data = new char[size + 1];
        file.read(data, size);
        data[size] = '\0';

        file.close();

        return new FileEntry{data, size};
    }
    auto DefaultFileSystemProvider::Close(const FileEntry* fileEntry) -> void
    {
        delete[] fileEntry->GetData();
        delete fileEntry;
    }
} // namespace glsld
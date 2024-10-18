#include "Compiler/SourceContext.h"

namespace glsld
{
    auto SourceContext::OpenFromBuffer(StringView sourceText) -> FileID
    {
        auto result = entries.emplace_back(GetNextFileID(), "", sourceText);
        return result.id;
    }

    auto SourceContext::OpenFromFile(const std::filesystem::path& path) -> FileID
    {
        if (auto it = lookupPathToEntries.find(path); it != lookupPathToEntries.end()) {
            return it->second;
        }

        std::error_code ec;
        auto canonicalPath = std::filesystem::canonical(path, ec);
        if (ec) {
            lookupPathToEntries[path] = {};
            return {};
        }
        if (auto it = canonicalPathToEntries.find(canonicalPath); it != canonicalPathToEntries.end()) {
            lookupPathToEntries[path] = it->second;
            return it->second;
        }

        auto canonicalPathStr = canonicalPath.string();
        auto fileHandle       = fileSystemProvider.Open(canonicalPathStr);
        if (!fileHandle) {
            lookupPathToEntries[path]             = {};
            canonicalPathToEntries[canonicalPath] = {};
            return {};
        }

        auto result = entries.emplace_back(GetNextFileID(), std::move(canonicalPathStr), fileHandle->GetContent());
        lookupPathToEntries[path]             = result.id;
        canonicalPathToEntries[canonicalPath] = result.id;
        return result.id;
    }
} // namespace glsld
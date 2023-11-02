#include "Compiler/SourceContext.h"

namespace glsld
{
    auto SourceContext::OpenFromBuffer(StringView sourceText) -> const SourceFileEntry*
    {
        auto result = entries.emplace_back(std::make_unique<SourceFileEntry>(AllocateFileID(), "")).get();
        result->SetSourceText(sourceText);
        return result;
    }

    auto SourceContext::OpenFromFile(const std::filesystem::path& path) -> const SourceFileEntry*
    {
        if (auto it = lookupPathToEntries.find(path); it != lookupPathToEntries.end()) {
            return it->second;
        }

        std::error_code ec;
        auto canonicalPath = std::filesystem::canonical(path, ec);
        if (ec) {
            lookupPathToEntries[path] = nullptr;
            return nullptr;
        }
        if (auto it = canonicalPathToEntries.find(canonicalPath); it != canonicalPathToEntries.end()) {
            lookupPathToEntries[path] = it->second;
            return it->second;
        }

        auto canonicalPathStr = canonicalPath.string();
        auto FileHandle       = fileSystemProvider.Open(canonicalPathStr);
        if (!FileHandle) {
            lookupPathToEntries[path]             = nullptr;
            canonicalPathToEntries[canonicalPath] = nullptr;
            return nullptr;
        }

        auto result =
            entries.emplace_back(std::make_unique<SourceFileEntry>(AllocateFileID(), std::move(canonicalPathStr)))
                .get();
        result->SetSourceText(StringView{FileHandle->GetData(), FileHandle->GetSize()});
        lookupPathToEntries[path]             = result;
        canonicalPathToEntries[canonicalPath] = result;
        return result;
    }
} // namespace glsld
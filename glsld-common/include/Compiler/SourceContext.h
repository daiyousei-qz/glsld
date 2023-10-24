#pragma once
#include "Basic/Common.h"
#include "Basic/FileSystemProvider.h"

#include <unordered_map>
#include <filesystem>
#include <vector>

namespace glsld
{
    using FileID = int;

    class SourceFileEntry final
    {
    public:
        SourceFileEntry(FileID id, std::string canonicalPath) : id(id), canonicalPath(std::move(canonicalPath))
        {
        }

        auto GetID() const noexcept -> FileID
        {
            return id;
        }

        auto GetCanonicalPath() const noexcept -> const std::string&
        {
            return canonicalPath;
        }

        auto GetSourceText() const noexcept -> std::optional<StringView>
        {
            return sourceText;
        }

        auto SetSourceText(StringView sourceText) -> void
        {
            this->sourceText = sourceText;
        }

        auto ClearSourceText() -> void
        {
            sourceText = std::nullopt;
        }

    private:
        FileID id;

        // The absolute path of file. Could be empty if the file is not on disk.
        std::string canonicalPath;

        std::optional<StringView> sourceText;
    };

    // This class manages views of all source files in a translation unit
    class SourceContext final
    {
    public:
        SourceContext(FileSystemProvider& fileSystemProvider) : fileSystemProvider(fileSystemProvider)
        {
        }

        ~SourceContext()
        {
            Finalize();
        }

        // A SourceContext cannot be copied or moved
        SourceContext(const SourceContext&)                    = delete;
        SourceContext(SourceContext&&)                         = delete;
        auto operator=(const SourceContext&) -> SourceContext& = delete;
        auto operator=(SourceContext&&) -> SourceContext&      = delete;

        auto GetSourceFileEntry(FileID fileId) -> SourceFileEntry*
        {
            GLSLD_ASSERT(fileId >= 0 && fileId < entries.size());
            return entries[fileId].get();
        }

        auto SetMainFile(const SourceFileEntry* file) -> void
        {
            mainFileEntry = file;
        }

        auto GetMainFile() const noexcept -> const SourceFileEntry*
        {
            return mainFileEntry;
        }

        auto OpenFromBuffer(StringView sourceText) -> const SourceFileEntry*
        {
            auto result =
                entries.emplace_back(std::make_unique<SourceFileEntry>(static_cast<FileID>(entries.size()), "")).get();
            result->SetSourceText(sourceText);
            return result;
        }

        auto OpenFromFile(const std::filesystem::path& path) -> const SourceFileEntry*
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
            auto fileRef          = fileSystemProvider.Open(canonicalPathStr);
            if (!fileRef) {
                lookupPathToEntries[path]             = nullptr;
                canonicalPathToEntries[canonicalPath] = nullptr;
                return nullptr;
            }

            auto result = entries
                              .emplace_back(std::make_unique<SourceFileEntry>(static_cast<FileID>(entries.size()),
                                                                              std::move(canonicalPathStr)))
                              .get();
            result->SetSourceText(StringView{fileRef->GetData(), fileRef->GetSize()});
            lookupPathToEntries[path]             = result;
            canonicalPathToEntries[canonicalPath] = result;
            return result;
        }

        // Release all opened files from the file system provider and user buffers.
        // However, the source file entries remain to be valid.
        auto Finalize() -> void
        {
            for (const auto& entry : entries) {
                entry->ClearSourceText();
            }

            for (const auto& fileRef : openedFiles) {
                fileSystemProvider.Close(fileRef);
            }
        }

    private:
        FileSystemProvider& fileSystemProvider;

        const SourceFileEntry* mainFileEntry;

        std::vector<std::unique_ptr<SourceFileEntry>> entries;

        std::unordered_map<std::filesystem::path, SourceFileEntry*> lookupPathToEntries;

        std::unordered_map<std::filesystem::path, SourceFileEntry*> canonicalPathToEntries;

        std::vector<const FileRef*> openedFiles;
    };
} // namespace glsld
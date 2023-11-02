#pragma once
#include "Basic/Common.h"
#include "Basic/FileSystemProvider.h"
#include "Basic/SourceInfo.h"
#include "Compiler/CompilerContextBase.h"

#include <unordered_map>
#include <filesystem>
#include <vector>

namespace glsld
{
    class SourceFileEntry final
    {
    private:
        FileID id;

        // The absolute path of file. Could be empty if the file is not on disk.
        std::string canonicalPath;

        std::optional<StringView> sourceText;

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

        // FIXME: make source text retrival a part of SourceContext instead.
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
    };

    // This class manages everything related the source files/buffers when compiling a translation unit.
    class SourceContext final : CompilerContextBase<SourceContext>
    {
    private:
        FileSystemProvider& fileSystemProvider;

        const SourceFileEntry* mainFileEntry = nullptr;

        FileID nextFileID = 0;

        std::vector<std::unique_ptr<SourceFileEntry>> entries;

        //
        std::unordered_map<std::filesystem::path, SourceFileEntry*> lookupPathToEntries;

        //
        std::unordered_map<std::filesystem::path, SourceFileEntry*> canonicalPathToEntries;

        std::vector<const FileHandle*> openedFiles;

    public:
        SourceContext(const SourceContext* preambleContext, FileSystemProvider& fileSystemProvider)
            : CompilerContextBase(preambleContext), fileSystemProvider(fileSystemProvider)
        {
            if (preambleContext) {
                nextFileID             = preambleContext->nextFileID;
                canonicalPathToEntries = preambleContext->canonicalPathToEntries;
            }
        }

        ~SourceContext()
        {
            Finalize();
        }

        // Release all opened files from the file system provider and user buffers.
        // However, the source file entries remain to be valid.
        auto Finalize() -> void
        {
            for (const auto& entry : entries) {
                entry->ClearSourceText();
            }

            for (const auto& handle : openedFiles) {
                fileSystemProvider.Close(handle);
            }
        }

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

        auto OpenFromBuffer(StringView sourceText) -> const SourceFileEntry*;

        auto OpenFromFile(const std::filesystem::path& path) -> const SourceFileEntry*;

    private:
        auto AllocateFileID() -> FileID
        {
            return nextFileID++;
        }
    };
} // namespace glsld
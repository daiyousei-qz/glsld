#pragma once
#include "Basic/Common.h"
#include "Basic/FileSystemProvider.h"
#include "Basic/SourceInfo.h"
#include "Compiler/CompilerResult.h"

#include <unordered_map>
#include <filesystem>
#include <vector>

namespace glsld
{
    // This class manages everything related the source files/buffers when compiling a translation unit.
    class SourceManager final
    {
    private:
        struct SourceFileEntry
        {
            FileID id;

            // The absolute path of file. Could be empty if the file is not on disk.
            std::string canonicalPath;

            StringView content;
        };

        FileSystemProvider& fileSystemProvider = DefaultFileSystemProvider::GetInstance();

        StringView systemPreamble;

        StringView userPreamble;

        std::vector<SourceFileEntry> entries;

        //
        std::unordered_map<std::filesystem::path, FileID> lookupPathToEntries;

        //
        std::unordered_map<std::filesystem::path, FileID> canonicalPathToEntries;

        std::vector<const FileHandle*> openedFiles;

    public:
        SourceManager(const PrecompiledPreamble* preamble = nullptr)
        {
            if (preamble) {
                systemPreamble = preamble->GetSystemPreamble();
                userPreamble   = preamble->GetUserPreamble();
            }
        }

        ~SourceManager()
        {
            for (const auto& handle : openedFiles) {
                fileSystemProvider.Close(handle);
            }
        }

        auto SetSystemPreamble(StringView content) -> void
        {
            systemPreamble = content;
        }

        auto SetUserPreamble(StringView content) -> void
        {
            userPreamble = content;
        }

        auto GetSystemPreamble() const noexcept -> StringView
        {
            return systemPreamble;
        }

        auto GetUserPreamble() const noexcept -> StringView
        {
            return userPreamble;
        }

        auto GetSourceText(FileID fileId) -> StringView
        {
            if (!fileId.IsValid()) {
                return {};
            }
            else if (fileId.IsSystemPreamble()) {
                return systemPreamble;
            }
            else if (fileId.IsUserPreamble()) {
                return userPreamble;
            }
            else {
                return GetUserFileEntry(fileId).content;
            }
        }

        auto OpenFromBuffer(StringView sourceText) -> FileID;

        auto OpenFromFile(const std::filesystem::path& path) -> FileID;

    private:
        auto GetUserFileEntry(FileID fileId) -> const SourceFileEntry&
        {
            GLSLD_ASSERT(fileId.IsUserFile());
            return entries[fileId.GetValue() - 1];
        }

        auto GetNextFileID() -> FileID
        {
            return FileID::FromIndex(entries.size() + 1);
        }
    };
} // namespace glsld
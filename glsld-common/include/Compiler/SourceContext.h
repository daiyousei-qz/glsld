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
    // This class manages everything related the source files/buffers when compiling a translation unit.
    class SourceContext final : CompilerContextBase<SourceContext>
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

        StringView systemPreambleContent;

        StringView userPreambleContent;

        std::vector<SourceFileEntry> entries;

        //
        std::unordered_map<std::filesystem::path, FileID> lookupPathToEntries;

        //
        std::unordered_map<std::filesystem::path, FileID> canonicalPathToEntries;

        std::vector<const FileHandle*> openedFiles;

    public:
        SourceContext(const SourceContext* preambleContext) : CompilerContextBase(preambleContext)
        {
            if (preambleContext) {
                systemPreambleContent = preambleContext->systemPreambleContent;
                userPreambleContent   = preambleContext->userPreambleContent;
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
            for (const auto& handle : openedFiles) {
                fileSystemProvider.Close(handle);
            }
        }

        auto SetSystemPreamble(StringView content) -> void
        {
            GLSLD_REQUIRE(GetPreambleContext() == nullptr && "Cannot set system preamble if already imported.");
            systemPreambleContent = content;
        }

        auto SetUserPreamble(StringView content) -> void
        {
            GLSLD_REQUIRE(GetPreambleContext() == nullptr && "Cannot set user preamble if already imported.");
            userPreambleContent = content;
        }

        auto GetSourceText(FileID fileId) -> StringView
        {
            if (!fileId.IsValid()) {
                return {};
            }
            else if (fileId.IsSystemPreable()) {
                return systemPreambleContent;
            }
            else if (fileId.IsUserPreamble()) {
                return userPreambleContent;
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
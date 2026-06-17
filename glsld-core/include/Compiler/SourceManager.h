#pragma once
#include "Basic/Common.h"
#include "Basic/SourceInfo.h"
#include "Compiler/CompilerResult.h"
#include "Support/VirtualFileSystem.h"
#include "Support/StringMap.h"

#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace glsld
{
    // This class manages everything related the source files/buffers when compiling a translation unit.
    class SourceManager final
    {
    private:
        struct SourceFileEntry
        {
            // A monolithic unique ID for the file.
            FileID id;

            // URI that is used to open the file.
            std::string canonicalUri;

            // The content of the file read from VFS
            std::string content;
        };

        std::shared_ptr<VirtualFileSystem> vfs = std::make_shared<NativeFileSystem>();

        std::deque<SourceFileEntry> entries;

        StringMap<FileID> lookupUriToEntries;

    public:
        SourceManager(const PrecompiledPreamble* preamble = nullptr)
        {
        }

        auto SetVirtualFileSystem(std::shared_ptr<VirtualFileSystem> newVfs) -> void
        {
            GLSLD_REQUIRE(entries.empty());
            GLSLD_REQUIRE(newVfs != nullptr);
            vfs = std::move(newVfs);
        }

        auto GetUri(FileID fileId) -> StringView;

        auto GetSourceText(FileID fileId) -> SourceTextView;

        // TODO: we should pass an Uri here instead of ParsedUri to require being normalized.
        //       this avoids us repeatedly normalizing the same Uri.
        auto OpenFromUri(ParsedUri uri) -> FileID;

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

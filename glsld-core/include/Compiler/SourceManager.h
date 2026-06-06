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
            FileID id;

            // The canonical URI of the file. Empty if the file came from an unmanaged buffer.
            std::string canonicalUri;

            SourceTextView content;
        };

        std::shared_ptr<VirtualFileSystem> vfs = std::make_shared<NativeFileSystem>();

        SourceTextView systemPreamble;

        SourceTextView userPreamble;

        std::deque<SourceFileEntry> entries;

        std::deque<std::string> ownedFileContents;

        StringMap<FileID> lookupUriToEntries;

    public:
        SourceManager(const PrecompiledPreamble* preamble = nullptr)
        {
            if (preamble) {
                systemPreamble = preamble->GetSystemPreamble();
                userPreamble   = preamble->GetUserPreamble();
            }
        }

        auto SetSystemPreamble(SourceTextView content) -> void
        {
            systemPreamble = content;
        }

        auto SetUserPreamble(SourceTextView content) -> void
        {
            userPreamble = content;
        }

        auto GetSystemPreamble() const noexcept -> SourceTextView
        {
            return systemPreamble;
        }

        auto GetUserPreamble() const noexcept -> SourceTextView
        {
            return userPreamble;
        }

        auto SetVirtualFileSystem(std::shared_ptr<VirtualFileSystem> newVfs) -> void
        {
            GLSLD_REQUIRE(entries.empty());
            GLSLD_REQUIRE(newVfs != nullptr);
            vfs = std::move(newVfs);
        }

        auto GetUri(FileID fileId) -> StringView
        {
            if (!fileId.IsValid()) {
                return "";
            }
            else if (fileId.IsSystemPreamble()) {
                return "glsld-internal:/system_preamble";
            }
            else if (fileId.IsUserPreamble()) {
                return "glsld-internal:/user_preamble";
            }
            else {
                return GetUserFileEntry(fileId).canonicalUri;
            }
        }

        auto GetSourceText(FileID fileId) -> SourceTextView
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

        auto OpenFromBuffer(SourceTextView sourceText) -> FileID;

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

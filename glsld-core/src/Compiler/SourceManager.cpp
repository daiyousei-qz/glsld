#include "Compiler/SourceManager.h"
#include "Support/Uri.h"

namespace glsld
{
    auto SourceManager::OpenFromUri(ParsedUri uri) -> FileID
    {
        // FIXME: this will fail miserably for symlink :(
        const auto uriText = uri.ToString();
        if (auto it = lookupUriToEntries.Find(uriText); it != lookupUriToEntries.end()) {
            return it->second;
        }

        auto fileContent = vfs->ReadAllText(uri);
        if (!fileContent) {
            lookupUriToEntries[uriText] = {};
            return {};
        }

        ownedFileContents.push_back(std::move(*fileContent));
        const auto& result =
            entries.emplace_back(GetNextFileID(), std::move(uriText), SourceTextView{ownedFileContents.back()});

        lookupUriToEntries[uriText] = result.id;
        return result.id;
    }
} // namespace glsld

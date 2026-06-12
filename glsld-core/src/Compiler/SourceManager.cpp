#include "Compiler/SourceManager.h"
#include "Support/Uri.h"

namespace glsld
{
    auto SourceManager::OpenFromBuffer(SourceTextView sourceText) -> FileID
    {
        auto result = entries.emplace_back(GetNextFileID(), "", sourceText);
        return result.id;
    }

    auto SourceManager::OpenFromUri(ParsedUri uri) -> FileID
    {
        // FIXME: this will fail miserably for symlink :(
        const auto persistUri = uri.ToUri();
        const auto uriText    = persistUri.GetRawText();
        if (auto it = lookupUriToEntries.Find(uriText); it != lookupUriToEntries.end()) {
            return it->second;
        }

        auto fileContent = vfs->ReadAllText(persistUri.GetParsedUri());
        if (!fileContent) {
            lookupUriToEntries[uriText] = {};
            return {};
        }

        ownedFileContents.push_back(std::move(*fileContent));
        const auto& result =
            entries.emplace_back(GetNextFileID(), uriText.Str(), SourceTextView{ownedFileContents.back()});

        lookupUriToEntries[uriText] = result.id;
        return result.id;
    }
} // namespace glsld

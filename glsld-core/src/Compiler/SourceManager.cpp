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
        const auto normalizedUri     = uri.Normalize();
        const auto normalizedUriText = normalizedUri.GetRawText();
        if (auto it = lookupUriToEntries.Find(normalizedUriText); it != lookupUriToEntries.end()) {
            return it->second;
        }

        auto fileContent = vfs->ReadAllText(normalizedUri.GetParsedUri());
        if (!fileContent) {
            lookupUriToEntries[normalizedUriText] = {};
            return {};
        }

        ownedFileContents.push_back(std::move(*fileContent));
        const auto& result =
            entries.emplace_back(GetNextFileID(), normalizedUriText.Str(), SourceTextView{ownedFileContents.back()});

        lookupUriToEntries[normalizedUriText] = result.id;
        return result.id;
    }
} // namespace glsld

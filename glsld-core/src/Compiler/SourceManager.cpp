#include "Compiler/SourceManager.h"
#include "Language/Stdlib.Generated.h"
#include "Support/Uri.h"

namespace glsld
{
    auto SourceManager::GetSourceText(FileID fileId) -> SourceTextView
    {
        if (!fileId.IsValid()) {
            return {};
        }
        else if (fileId.IsPreamble()) {
            return GlslStdlibText;
        }
        else {
            return GetUserFileEntry(fileId).content;
        }
    }

    auto SourceManager::GetUri(FileID fileId) -> StringView
    {
        if (!fileId.IsValid()) {
            return "";
        }
        else if (fileId.IsPreamble()) {
            return "glsld-internal:/system_preamble";
        }
        else {
            return GetUserFileEntry(fileId).canonicalUri;
        }
    }

    auto SourceManager::OpenFromUri(ParsedUri uri) -> FileID
    {
        const auto uriText = uri.ToString();
        if (auto it = lookupUriToEntries.Find(uriText); it != lookupUriToEntries.end()) {
            return it->second;
        }

        auto fileContent = vfs->ReadAllText(uri);
        if (!fileContent) {
            lookupUriToEntries[uriText] = {};
            return {};
        }

        const auto& result = entries.emplace_back(GetNextFileID(), std::move(uriText), std::move(*fileContent));

        lookupUriToEntries[uriText] = result.id;
        return result.id;
    }
} // namespace glsld

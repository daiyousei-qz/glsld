#pragma once
#include "Common.h"
#include "StringView.h"
#include "FileSystemProvider.h"

#include <vector>

namespace glsld
{
    // This class manages views of all source files in a translation unit
    class SourceContext final
    {
    public:
        SourceContext(FileSystemProvider& fileSystemProvider) : fileSystemProvider(fileSystemProvider)
        {
        }

        ~SourceContext()
        {
            for (const auto& [uri, fileEntry] : openedFileEntries) {
                fileSystemProvider.Close(fileEntry);
            }
        }

        // A SourceContext cannot be copied or moved
        SourceContext(const SourceContext&)                    = delete;
        SourceContext(SourceContext&&)                         = delete;
        auto operator=(const SourceContext&) -> SourceContext& = delete;
        auto operator=(SourceContext&&) -> SourceContext&      = delete;

        auto GetSourceView(StringView headerName) -> std::optional<StringView>
        {
            auto uriStr = headerName.Str();

            if (auto it = openedFileEntries.find(uriStr); it != openedFileEntries.end()) {
                return StringView{it->second->GetData(), it->second->GetSize()};
            }
            else {
                auto fileEntry = fileSystemProvider.Open(headerName);
                if (fileEntry) {
                    openedFileEntries.emplace(uriStr, fileEntry);
                    return StringView{fileEntry->GetData(), fileEntry->GetSize()};
                }
                else {
                    return std::nullopt;
                }
            }
        }

    private:
        FileSystemProvider& fileSystemProvider;

        std::unordered_map<std::string, const FileEntry*> openedFileEntries;
    };
} // namespace glsld
#include "Support/VirtualFileSystem.h"
#include "Support/ScopeExit.h"

#include <cerrno>
#include <cstdio>

namespace glsld
{

    auto VirtualFileSystem::Exists(StringView uri) -> std::expected<std::monostate, FileSystemError>
    {
        auto parsedUri = ParsedUri::Parse(uri);
        if (!parsedUri) {
            return std::unexpected(FileSystemError::InvalidUri);
        }

        return Exists(*parsedUri);
    }
    auto VirtualFileSystem::Exists(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError>
    {
        return ExistsImpl(uri);
    }
    auto VirtualFileSystem::ReadAllText(StringView uri) -> std::expected<std::string, FileSystemError>
    {
        auto parsedUri = ParsedUri::Parse(uri);
        if (!parsedUri) {
            return std::unexpected(FileSystemError::InvalidUri);
        }

        return ReadAllText(*parsedUri);
    }
    auto VirtualFileSystem::ReadAllText(const ParsedUri& uri) -> std::expected<std::string, FileSystemError>
    {
        return ReadAllTextImpl(uri);
    }

#pragma region NativeFileSystem

    static auto TranslateFileSystemError(int err) -> FileSystemError
    {
        switch (err) {
        case ENOENT:
            return FileSystemError::NotFound;
        case EACCES:
            return FileSystemError::PermissionDenied;
        case EISDIR:
            return FileSystemError::IsADirectory;
        case ENOTDIR:
            return FileSystemError::NotADirectory;
        case EIO:
            return FileSystemError::IOError;
        default:
            return FileSystemError::Unknown;
        }
    }

    auto NativeFileSystem::ExistsImpl(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError>
    {
        auto path = uri.ToFileSystemPath();
        if (!path) {
            return std::unexpected(FileSystemError::InvalidUri);
        }

        try {
            if (!std::filesystem::exists(*path)) {
                return std::unexpected(FileSystemError::NotFound);
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            return std::unexpected(FileSystemError::Unknown);
        }

        return std::monostate{};
    }

    auto NativeFileSystem::ReadAllTextImpl(const ParsedUri& uri) -> std::expected<std::string, FileSystemError>
    {
        auto path = uri.ToFileSystemPath();
        if (!path) {
            return std::unexpected(FileSystemError::InvalidUri);
        }

        auto file = std::fopen(path->string().c_str(), "rb");
        if (!file) {
            return std::unexpected(TranslateFileSystemError(errno));
        }

        ScopeExit fileCloser([file]() { std::fclose(file); });

        if (std::fseek(file, 0, SEEK_END) != 0) {
            return std::unexpected(TranslateFileSystemError(errno));
        }

        auto size = std::ftell(file);
        if (size < 0) {
            return std::unexpected(TranslateFileSystemError(errno));
        }

        if (std::fseek(file, 0, SEEK_SET) != 0) {
            return std::unexpected(TranslateFileSystemError(errno));
        }

        std::string content;
        content.resize(static_cast<std::size_t>(size));

        auto bytesRead = std::fread(content.data(), 1, content.size(), file);
        if (bytesRead != content.size()) {
            return std::unexpected(TranslateFileSystemError(std::ferror(file) ? errno : 0));
        }

        return content;
    }

#pragma endregion

#pragma region InMemoryFileSystem

    auto InMemoryFileSystem::GetPathEntry(const ParsedUri& uri) -> DirectoryOrFile*
    {
        auto rootKey = fmt::format("{}:{}", uri.GetNormalizedScheme(), uri.GetRawAuthority());
        auto rootIt  = uriRoots.Find(StringView{rootKey});
        if (rootIt == uriRoots.end()) {
            return nullptr;
        }

        auto normalizedPath = uri.GetNormalizedPath();
        auto currentEntry   = &rootIt->second;
        for (StringView component : UriPathSegmentView{normalizedPath}) {
            if (component.empty()) {
                // Collapse consecutive '/'
                continue;
            }
            if (currentEntry->fileContent.has_value()) {
                // The entry is a file but we expect a directory.
                return nullptr;
            }

            auto it = currentEntry->children.Find(*DecodeUriComponent(component));
            if (it == currentEntry->children.end()) {
                return nullptr;
            }
            currentEntry = &it->second;
        }

        return currentEntry;
    }

    auto InMemoryFileSystem::GetOrCreatePathEntry(const ParsedUri& uri) -> DirectoryOrFile*
    {
        auto rootKey   = fmt::format("{}:{}", uri.GetNormalizedScheme(), uri.GetRawAuthority());
        auto rootEntry = &uriRoots[StringView{rootKey}];

        auto normalizedPath = uri.GetNormalizedPath();
        auto currentEntry   = rootEntry;
        for (StringView component : UriPathSegmentView{normalizedPath}) {
            if (component.empty()) {
                // Collapse consecutive '/'
                continue;
            }
            if (currentEntry->fileContent.has_value()) {
                // The entry is a file but we expect a directory.
                return nullptr;
            }

            currentEntry = &currentEntry->children[*DecodeUriComponent(component)];
        }

        return currentEntry;
    }

    auto InMemoryFileSystem::AddFile(StringView uri, std::string content) -> std::expected<void, FileSystemError>
    {
        auto parsedUri = ParsedUri::Parse(uri);
        if (!parsedUri) {
            return std::unexpected(FileSystemError::InvalidUri);
        }

        return AddFile(*parsedUri, std::move(content));
    }

    auto InMemoryFileSystem::AddFile(ParsedUri uri, std::string content) -> std::expected<void, FileSystemError>
    {
        auto currentEntry = GetOrCreatePathEntry(uri);
        if (!currentEntry) {
            return std::unexpected(FileSystemError::NotADirectory);
        }
        if (!currentEntry->children.Empty()) {
            return std::unexpected(FileSystemError::IsADirectory);
        }
        if (currentEntry->fileContent.has_value()) {
            return std::unexpected(FileSystemError::IOError);
        }

        currentEntry->fileContent = ownedFiles.emplace_back(std::move(content));
        return {};
    }

    auto InMemoryFileSystem::AddFileNoOwn(StringView uri, StringView content) -> std::expected<void, FileSystemError>
    {
        auto parsedUri = ParsedUri::Parse(uri);
        if (!parsedUri) {
            return std::unexpected(FileSystemError::InvalidUri);
        }

        return AddFileNoOwn(*parsedUri, content);
    }
    auto InMemoryFileSystem::AddFileNoOwn(ParsedUri uri, StringView content) -> std::expected<void, FileSystemError>
    {
        auto currentEntry = GetOrCreatePathEntry(uri);
        if (!currentEntry) {
            return std::unexpected(FileSystemError::NotADirectory);
        }
        if (!currentEntry->children.Empty()) {
            return std::unexpected(FileSystemError::IsADirectory);
        }
        if (currentEntry->fileContent.has_value()) {
            return std::unexpected(FileSystemError::IOError);
        }

        currentEntry->fileContent = content;
        return {};
    }

    auto InMemoryFileSystem::ExistsImpl(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError>
    {
        if (auto entry = GetPathEntry(uri); entry != nullptr) {
            return std::monostate{};
        }
        else {
            return std::unexpected(FileSystemError::NotFound);
        }
    }

    auto InMemoryFileSystem::ReadAllTextImpl(const ParsedUri& uri) -> std::expected<std::string, FileSystemError>
    {
        auto entry = GetPathEntry(uri);
        if (entry == nullptr) {
            return std::unexpected(FileSystemError::NotFound);
        }
        if (!entry->fileContent.has_value()) {
            return std::unexpected(FileSystemError::IsADirectory);
        }

        return entry->fileContent.value().Str();
    }

#pragma endregion

#pragma region OverlayFileSystem

    auto OverlayFileSystem::ExistsImpl(const ParsedUri& uri) -> std::expected<std::monostate, FileSystemError>
    {
        FileSystemError lastError = FileSystemError::NotFound;
        for (const auto& layer : layers) {
            if (auto result = layer->Exists(uri); result.has_value()) {
                return std::monostate{};
            }
            else {
                lastError = result.error();
            }
        }

        return std::unexpected(lastError);
    }

    auto OverlayFileSystem::ReadAllTextImpl(const ParsedUri& uri) -> std::expected<std::string, FileSystemError>
    {
        FileSystemError lastError = FileSystemError::NotFound;
        for (const auto& layer : layers) {
            if (auto result = layer->ReadAllText(uri); result.has_value()) {
                return result;
            }
            else {
                lastError = result.error();
            }
        }

        return std::unexpected(lastError);
    }

#pragma endregion
} // namespace glsld

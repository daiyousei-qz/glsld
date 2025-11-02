#pragma once
#include <Support/StringView.h>

#include <cstdio>
#include <cstddef>
#include <expected>
#include <optional>

namespace glsld
{
    enum class Status
    {
        Ok,
        InvalidArgument,
        IoError,
        EndOfFile,
        NotOpen
    };

    class UniqueFile final
    {
    private:
        FILE* handle = nullptr;

        explicit UniqueFile(FILE* handle) noexcept : handle(handle)
        {
        }

    public:
        using PositionType = long;

        static auto Open(const char* path, const char* mode) -> std::expected<UniqueFile, Status>;
        static auto ReadAllText(const char* path) -> std::optional<std::string>;

        UniqueFile() noexcept = default;
        UniqueFile(UniqueFile&& other) noexcept;
        UniqueFile& operator=(UniqueFile&& other) noexcept;
        UniqueFile(const UniqueFile&)            = delete;
        UniqueFile& operator=(const UniqueFile&) = delete;
        ~UniqueFile()
        {
            Close();
        }

        operator bool() const noexcept
        {
            return IsOpen();
        }

        [[nodiscard]] auto NativeHandle() const noexcept -> FILE*
        {
            return handle;
        }

        [[nodiscard]] auto IsOpen() const noexcept -> bool
        {
            return handle != nullptr;
        }

        auto Close() noexcept -> Status;
        auto Read(void* buffer, std::size_t elementSize, std::size_t elementCount) noexcept -> Status;
        auto Read(char* buffer, std::size_t elementCount) noexcept -> Status;
        auto Write(const void* buffer, std::size_t elementSize, std::size_t elementCount) noexcept -> Status;
        auto Write(const char* buffer, std::size_t elementCount) noexcept -> Status;
        auto Write(StringView text) noexcept -> Status;
        auto Flush() noexcept -> Status;
        auto Seek(PositionType offset, int origin) noexcept -> Status;
        auto Tell() noexcept -> std::expected<PositionType, Status>;
        auto SizeFast() noexcept -> std::expected<PositionType, Status>;
    };
} // namespace glsld

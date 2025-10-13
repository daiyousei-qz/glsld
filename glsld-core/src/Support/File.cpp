#include "Support/File.h"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>

namespace glsld
{
    namespace
    {
        auto TranslateFromErrno(int err) -> Status
        {
            switch (err) {
            case 0:
                return Status::Ok;
            case EINVAL:
                return Status::InvalidArgument;
            default:
                return Status::IoError;
            }
        }
    } // namespace

    auto File::Open(const char* path, const char* mode) -> std::expected<File, Status>
    {
        if (!path || !mode) {
            return std::unexpected(Status::InvalidArgument);
        }
        FILE* handle = std::fopen(path, mode);
        if (!handle) {
            return std::unexpected(TranslateFromErrno(errno));
        }

        return File(handle);
    }

    auto File::ReadAllText(const char* path) -> std::optional<std::string>
    {
        auto fileOrErr = Open(path, "rb");
        if (!fileOrErr) {
            return std::nullopt;
        }
        auto& file = *fileOrErr;

        auto sizeOrErr = file.SizeFast();
        if (!sizeOrErr) {
            return std::nullopt;
        }
        auto size = *sizeOrErr;

        if (auto status = file.Seek(0, SEEK_SET); status != Status::Ok) {
            return std::nullopt;
        }

        std::string result;
        result.resize(size);

        if (auto status = file.Read(result.data(), size); status != Status::Ok) {
            return std::nullopt;
        }

        return result;
    }

    File::File(File&& other) noexcept : handle(other.handle)
    {
        other.handle = nullptr;
    }

    File& File::operator=(File&& other) noexcept
    {
        if (this != &other) {
            Close();
            handle       = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    auto File::Close() noexcept -> Status
    {
        if (!handle) {
            return Status::NotOpen;
        }

        const int result = std::fclose(handle);
        handle           = nullptr;
        return result == 0 ? Status::Ok : TranslateFromErrno(errno);
    }

    auto File::Read(void* buffer, std::size_t elementSize, std::size_t elementCount) noexcept -> Status
    {
        if (!handle) {
            return Status::NotOpen;
        }

        const auto result = std::fread(buffer, elementSize, elementCount, handle);
        return result == elementCount ? Status::Ok : TranslateFromErrno(errno);
    }

    auto File::Read(char* buffer, std::size_t elementCount) noexcept -> Status
    {
        return Read(buffer, 1, elementCount);
    }

    auto File::Write(const void* buffer, std::size_t elementSize, std::size_t elementCount) noexcept -> Status
    {
        if (!handle) {
            return Status::NotOpen;
        }

        const auto result = std::fwrite(buffer, elementSize, elementCount, handle);
        return result == elementCount ? Status::Ok : TranslateFromErrno(errno);
    }

    auto File::Write(const char* buffer, std::size_t elementCount) noexcept -> Status
    {
        return Write(buffer, 1, elementCount);
    }

    auto File::Write(StringView text) noexcept -> Status
    {
        return Write(text.data(), 1, text.Size());
    }

    auto File::Flush() noexcept -> Status
    {
        if (!handle) {
            return Status::NotOpen;
        }

        return std::fflush(handle) == 0 ? Status::Ok : TranslateFromErrno(errno);
    }

    auto File::Seek(long offset, int origin) noexcept -> Status
    {
        if (!handle) {
            return Status::NotOpen;
        }
        return std::fseek(handle, offset, origin) == 0 ? Status::Ok : TranslateFromErrno(errno);
    }

    auto File::Tell() noexcept -> std::expected<PositionType, Status>
    {
        if (!handle) {
            return std::unexpected(Status::NotOpen);
        }
        const auto pos = std::ftell(handle);
        if (pos < 0) {
            return std::unexpected(TranslateFromErrno(errno));
        }
        return pos;
    }

    auto File::SizeFast() noexcept -> std::expected<PositionType, Status>
    {
        if (!handle) {
            return std::unexpected(Status::NotOpen);
        }

        if (auto status = Seek(0, SEEK_END); status != Status::Ok) {
            return std::unexpected(status);
        }

        return Tell();
    }
} // namespace glsld

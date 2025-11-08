#include "Server/TextTransport.h"

#include <cstdio>
#include <vector>

#if defined(GLSLD_OS_WIN)
#include <io.h>
#include <fcntl.h>
#endif

namespace glsld
{
    class StdioTransport : public TextTransport
    {
    private:
        static constexpr size_t MinBufferSize = 10 * 1024;        // 10 KB
        static constexpr size_t MaxBufferSize = 10 * 1024 * 1024; // 10 MB

        std::vector<char> buffer = std::vector<char>(MinBufferSize);

    public:
        StdioTransport()
        {
#if defined(GLSLD_OS_WIN)
            // Use binary mode for stdin/stdout. We handle "/r/n" conversion inhouse.
            _setmode(_fileno(stdout), O_BINARY);
            _setmode(_fileno(stdin), O_BINARY);
#else
            // FIXME: not sure if this is needed
            // freopen(nullptr, "rb", stdout);
            // freopen(nullptr, "wb", stdin);
#endif

            setvbuf(stdin, nullptr, _IOFBF, 64 * 1024);
        }

        auto ReadLine() -> std::expected<StringView, TextTransportError> override
        {
            if (fgets(buffer.data(), static_cast<int>(buffer.size()), stdin) != nullptr) {
                return StringView{buffer.data(), strlen(buffer.data())};
            }
            else {
                return std::unexpected(TextTransportError::Unknown);
            }
        }

        auto Read(size_t size) -> std::expected<StringView, TextTransportError> override
        {
            if (size > buffer.size()) {
                if (size > MaxBufferSize) {
                    return std::unexpected(TextTransportError::Unknown);
                }
                buffer.resize(size);
            }

            size_t totalRead = 0;
            while (totalRead < size) {
                size_t bytesRead = fread(buffer.data() + totalRead, 1, size - totalRead, stdin);
                if (bytesRead == 0) {
                    return std::unexpected(TextTransportError::Unknown);
                }
                totalRead += bytesRead;
            }

            return StringView{buffer.data(), size};
        }

        auto Write(StringView data) -> bool override
        {
            size_t totalWritten = 0;
            while (totalWritten < data.Size()) {
                size_t bytesWritten = fwrite(data.data() + totalWritten, 1, data.Size() - totalWritten, stdout);
                if (bytesWritten == 0) {
                    return false;
                }
                totalWritten += bytesWritten;
            }
            return true;
        }

        auto Flush() -> bool override
        {
            return fflush(stdout) == 0;
        }
    };

    auto CreateStdioTextTransport() -> std::unique_ptr<TextTransport>
    {
        return std::make_unique<StdioTransport>();
    }

} // namespace glsld
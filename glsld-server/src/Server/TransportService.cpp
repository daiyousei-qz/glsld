#include "Server/TransportService.h"

namespace glsld
{
    static auto ReadLine(std::vector<char>& buf, FILE* inputFile) -> bool
    {
        while (true) {
            int ch = fgetc(inputFile);
            if (ch == EOF) {
                return false;
            }
            else {
                buf.push_back(ch);
                if (ch == '\n') {
                    return true;
                }
            }

            if (buf.size() > 10 * 1024 * 1024) {
                return false;
            }
        }
    }

    auto TransportService::PullMessage() -> bool
    {
        // Read LSP headers from input file
        // Although LSP requires "\r\n" as end-line, we only use "\n" as the delimitor for simplicity.
        size_t payloadLength = 0;
        while (true) {
            messageBuffer.clear();

            if (!ReadLine(messageBuffer, inFile)) {
                server.LogError("Failed to read LSP message header.");
                return false;
            }

            StringView headerView = StringView{messageBuffer}.Trim();

            if (headerView.Empty()) {
                // Empty line indicates end of headers, aka. start of payload.
                break;
            }

            if (headerView.StartWith("Content-Length: ")) {
                auto lengthView = headerView.Drop(16);
                if (std::from_chars(lengthView.data(), lengthView.data() + lengthView.Size(), payloadLength).ec !=
                    std::errc()) {
                    server.LogError("Failed to parse Content-Length header.");
                    return false;
                }
            }
            else if (headerView.StartWith("Content-Type: ")) {
                // do nothing...
            }
            else {
                server.LogWarn("Unknown LSP message header: {}", headerView);
                // We ignore any unknown header fields
            }
        }

        // If payload size provided is too large, close the transport as if EOF is reached
        if (payloadLength > 10 * 1024 * 1024) {
            server.LogError("LSP message payload size {} is too large.", payloadLength);
            return false;
        }

        messageBuffer.clear();
        messageBuffer.resize(payloadLength);

        size_t readSize = 0;
        while (readSize < payloadLength) {
            auto len = fread(messageBuffer.data() + readSize, sizeof(char), payloadLength - readSize, inFile);

            if (len == 0) {
                // See EOF
                server.LogError("Unexpected EOF while reading payload.");
                return false;
            }

            readSize += len;
        }

        server.HandleClientMessage(StringView{messageBuffer});
        return true;
    }

    auto TransportService::PushMessage(StringView payload) -> bool
    {
        // TODO: optimize this
        std::string header = fmt::format("Content-Length: {}\r\n\r\n", payload.Size());

        if (auto writeSize = fwrite(header.data(), sizeof(char), header.size(), outFile); writeSize != header.size()) {
            server.LogError("Failed to write LSP message header.");
            return false;
        }
        if (auto writeSize = fwrite(payload.data(), sizeof(char), payload.Size(), outFile);
            writeSize != payload.Size()) {
            server.LogError("Failed to write LSP message payload.");
            return false;
        }

        if (fflush(outFile) != 0) {
            server.LogError("Failed to flush LSP message.");
            return false;
        }

        return true;
    }

} // namespace glsld
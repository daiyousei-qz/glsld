#include "TransportService.h"

namespace glsld
{
    static auto ReadLine(std::string& buf, FILE* inputFile) -> bool
    {
        // FIXME: do not use std::cin
        buf.clear();

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
        lineBuffer.clear();
        payloadBuffer.clear();

        // Read LSP headers from input file
        // Although LSP requires "\r\n" as end-line, we only use "\n" as the delimitor for simplicity.
        size_t payloadLength = 0;
        while (true) {
            if (!ReadLine(lineBuffer, inFile)) {
                return false;
            }

            StringView headerView = StringView{lineBuffer}.Trim();

            if (headerView.Empty()) {
                break;
            }

            if (headerView.StartWith("Content-Length: ")) {
                headerView = headerView.Drop(16);

                // TODO: handle error
                std::from_chars(headerView.data(), headerView.data() + headerView.Size(), payloadLength);
            }
            else if (headerView.StartWith("Content-Type: ")) {
                // headerView.remove_prefix(14);
                // do nothing...
            }

            // We ignore any unknown header fields
        }

        // If payload size provided is too large, close the transport as if EOF is reached
        if (payloadLength > 10 * 1024 * 1024) {
            return false;
        }

        payloadBuffer.resize(payloadLength);

        size_t readSize = 0;
        while (readSize < payloadLength) {
            auto len = fread(payloadBuffer.data() + readSize, sizeof(char), payloadLength - readSize, inFile);

            if (len == 0) {
                // See EOF
                return false;
            }

            readSize += len;
        }

        server->HandleClientMessage(std::string_view{payloadBuffer.data(), payloadBuffer.size()});
        return true;
    }

    auto TransportService::PushMessage(StringView payload) -> void
    {
        // TODO: optimize this
        std::string header = fmt::format("Content-Length: {}\r\n\r\n", payload.Size());

        fwrite(header.data(), sizeof(char), header.size(), outFile);
        fwrite(payload.data(), sizeof(char), payload.Size(), outFile);
        fflush(outFile);
    }

} // namespace glsld
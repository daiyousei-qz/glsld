#include "TransportService.h"

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
                return false;
            }

            StringView headerView = StringView{messageBuffer}.Trim();

            if (headerView.Empty()) {
                break;
            }

            if (headerView.StartWith("Content-Length: ")) {
                headerView = headerView.Drop(16);

                // FIXME: handle error
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

        messageBuffer.clear();
        messageBuffer.resize(payloadLength);

        size_t readSize = 0;
        while (readSize < payloadLength) {
            auto len = fread(messageBuffer.data() + readSize, sizeof(char), payloadLength - readSize, inFile);

            if (len == 0) {
                // See EOF
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
            return false;
        }
        if (auto writeSize = fwrite(payload.data(), sizeof(char), payload.Size(), outFile);
            writeSize != payload.Size()) {
            return false;
        }

        if (fflush(outFile) != 0) {
            return false;
        }

        return true;
    }

} // namespace glsld
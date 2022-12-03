#pragma once

#include "Common.h"
#include "Protocol.h"
#include "LanguageServerInterface.h"

#include <fmt/format.h>

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <iterator>
#include <string>
#include <string_view>
#include <optional>
#include <utility>
#include <vector>
#include <functional>
#include <iostream>
#include <ranges>

namespace glsld
{
    // FIXME: figure out why vscode is requiring \n instead of \r\n
    class TransportService
    {
    public:
        TransportService(FILE* inFile, FILE* outFile, LanguageServerCallback* server)
            : inFile(inFile), outFile(outFile), server(server)
        {
            setvbuf(inFile, nullptr, _IOFBF, 64 * 1024);
            payloadBuffer.reserve(4096);
        }

        auto PullMessage() -> bool
        {
            lineBuffer.clear();
            payloadBuffer.clear();

            // Read LSP headers from input file
            // Although LSP requires "\r\n" as end-line, we allow "\n" as well.
            size_t payloadLength = 0;
            while (true) {
                if (!ReadLine(lineBuffer)) {
                    return false;
                }

                std::string_view headerView = Trim(lineBuffer);

                if (headerView.empty()) {
                    break;
                }

                if (headerView.starts_with("Content-Length: ")) {
                    headerView.remove_prefix(16);

                    // TODO: handle error
                    std::from_chars(headerView.data(), headerView.data() + headerView.size(), payloadLength);
                }
                else if (headerView.starts_with("Content-Type: ")) {
                    headerView.remove_prefix(14);
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
                    return false; // eof
                }

                readSize += len;
            }

            server->HandleClientMessage(std::string_view{payloadBuffer.data(), payloadBuffer.size()});
            return true;
        }

        auto PushMessage(const std::string_view& payload)
        {
            // TODO: optimize this
            std::string header = fmt::format("Content-Length: {}\n\n", payload.size());

            fwrite(header.data(), sizeof(char), header.size(), outFile);
            fwrite(payload.data(), sizeof(char), payload.size(), outFile);
            fflush(outFile);
        }

    private:
        auto Trim(std::string_view s) -> std::string_view
        {
            for (char ch : s) {
                if (isspace(ch)) {
                    s.remove_prefix(1);
                }
                else {
                    break;
                }
            }
            for (char ch : std::views::reverse(s)) {
                if (isspace(ch)) {
                    s.remove_suffix(1);
                }
                else {
                    break;
                }
            }

            return s;
        }

        auto ReadLine(std::string& buf) -> bool
        {
            buf.clear();

            while (true) {
                int ch = std::cin.get();
                if (std::cin.eof()) {
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

        FILE* inFile;
        FILE* outFile;
        LanguageServerCallback* server;

        std::string lineBuffer;
        std::vector<char> payloadBuffer = {};
    };

} // namespace glsld
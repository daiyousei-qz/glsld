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

namespace glsld
{
    struct LSPHeader
    {
        // The length of the content part in bytes. This header is required.
        size_t contentLength = 0;

        // The mime type of the content part. Defaults to application/vscode-jsonrpc; charset=utf-8
        std::string contentType = {};
    };

    class TransportService
    {
    public:
        TransportService(FILE* inFile, FILE* outFile, LanguageServerCallback* server)
            : inFile(inFile), outFile(outFile), server(server)
        {
        }

        auto PullMessage() -> bool
        {
            if (transportOffset >= sizeof(transportBuffer)) {
                // We are running out of the buffer for communication
                // This could be caused by rediculously long header
                std::abort();
            }

            auto len = fread(transportBuffer + transportOffset, sizeof(char), sizeof(transportBuffer), inFile);
            if (len == 0) {
                return false; // eof
            }

            transportOffset += len;
            std::string_view bufferView{transportBuffer, transportOffset};
            if (extraPayloadSize == 0) {
                ProcessFromHeader(bufferView);
            }
            else {
                ProcessFromPayload(bufferView);
            }

            return true;
        }

        auto PushMessage(const std::string_view& payload)
        {
            // TODO: optimize this
            std::string header = fmt::format("Content-Length: {}\r\n\r\n", payload.size());

            fwrite(header.data(), sizeof(char), header.size(), outFile);
            fwrite(payload.data(), sizeof(char), payload.size(), outFile);
            fflush(outFile);
        }

    private:
        auto ReadHeaderLine(std::string_view data) -> std::tuple<std::string_view, std::string_view>
        {
            bool CRFlag = false;
            for (size_t i = 0; i < data.size(); ++i) {
                if (CRFlag && data[i] == '\n') {
                    auto readLine      = data.substr(0, i + 1);
                    auto remainingData = data.substr(i + 1);
                    return {readLine, remainingData};
                }

                CRFlag = data[i] == '\r';
            }

            return {std::string_view{}, data};
        }

        auto ProcessFromHeader(std::string_view bufferView) -> void
        {
            // initialize message buffer
            headerBuffer = {};
            payloadBuffer.clear();

            // parse header
            while (true) {
                auto [headerView, remainingView] = ReadHeaderLine(bufferView);

                bufferView = remainingView;
                if (!headerView.empty()) {
                    GLSLD_ASSERT(headerView.ends_with("\r\n"));
                    headerView.remove_suffix(2);

                    if (headerView.empty()) {
                        break;
                    }

                    if (headerView.starts_with("Content-Length: ")) {
                        headerView.remove_prefix(16);

                        // TODO: handle error
                        std::from_chars(headerView.data(), headerView.data() + headerView.size(),
                                        headerBuffer.contentLength);
                    }
                    else if (headerView.starts_with("Content-Type: ")) {
                        headerView.remove_prefix(14);

                        headerBuffer.contentType = std::string{headerView};
                    }
                    else {
                        // We ignore unknown header fields
                    }
                }
                else {
                    // The buffer view given cannot form a complete header
                    // so we give up and wait for more data to come
                    return;
                }
            }

            extraPayloadSize = headerBuffer.contentLength;
            ProcessFromPayload(bufferView);
        }
        auto ProcessFromPayload(std::string_view bufferView) -> void
        {
            auto payloadView = bufferView.substr(0, extraPayloadSize);
            auto remainingView =
                extraPayloadSize < bufferView.size() ? bufferView.substr(extraPayloadSize) : std::string_view{};

            // Copy payload to payload buffer
            extraPayloadSize -= payloadView.size();
            std::ranges::copy(payloadView, std::back_inserter(payloadBuffer));

            if (remainingView.size() > 0) {
                // Copy remaining data back to the begining of the transport buffer
                std::ranges::copy(remainingView, transportBuffer);
                transportOffset = remainingView.size();
            }
            else {
                // Clear transport buffer since every byte is consumed
                transportOffset = 0;
            }

            if (extraPayloadSize == 0) {
                // We have everything for one message ready
                server->HandleClientRequest(std::string_view{payloadBuffer.data(), payloadBuffer.size()});
            }
        }

        FILE* inFile;
        FILE* outFile;
        LanguageServerCallback* server;

        size_t transportOffset     = 0;
        char transportBuffer[4096] = {};

        size_t extraPayloadSize         = 0;
        LSPHeader headerBuffer          = {};
        std::vector<char> payloadBuffer = {};
    };

} // namespace glsld
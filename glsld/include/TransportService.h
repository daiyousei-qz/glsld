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
    // This class handles json-rpc transport via standard file interface
    class TransportService
    {
    public:
        TransportService(FILE* inFile, FILE* outFile, LanguageServerCallback* server)
            : inFile(inFile), outFile(outFile), server(server)
        {
            payloadBuffer.reserve(4096);
        }

        auto PullMessage() -> bool;

        auto PushMessage(StringView payload) -> void;

    private:
        FILE* inFile;
        FILE* outFile;
        LanguageServerCallback* server;

        std::string lineBuffer;
        std::vector<char> payloadBuffer = {};
    };

} // namespace glsld
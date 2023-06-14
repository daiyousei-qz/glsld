#pragma once

#include "Common.h"
#include "Protocol.h"
#include "LanguageServerInterface.h"

#include <cstdio>
#include <string>
#include <vector>

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

        // Pull a message from the input stream
        // Returns true if a message is successfully pulled
        auto PullMessage() -> bool;

        // Push a message to the output stream
        auto PushMessage(StringView payload) -> void;

    private:
        FILE* inFile;
        FILE* outFile;
        LanguageServerCallback* server;

        std::string lineBuffer;
        std::vector<char> payloadBuffer = {};
    };

} // namespace glsld
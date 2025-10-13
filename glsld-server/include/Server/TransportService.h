#pragma once
#include "Basic/StringView.h"
#include "Server/LanguageServer.h"

#include <cstdio>
#include <vector>

namespace glsld
{
    // This class handles json-rpc transport via standard file interface
    class TransportService
    {
    private:
        LanguageServer& server;
        FILE* inFile;
        FILE* outFile;

        std::vector<char> messageBuffer = {};

    public:
        TransportService(LanguageServer& server, FILE* inFile, FILE* outFile)
            : inFile(inFile), outFile(outFile), server(server)
        {
            messageBuffer.reserve(4096);
        }

        // Synchrounously pull a message from the input stream and forward it to the server.
        // This message WILL be invalidated after the next call to this function.
        auto PullMessage() -> std::optional<StringView>;

        // Synchrounously push a message to the output stream.
        // Returns true if a message is successfully pushed.
        auto PushMessage(StringView payload) -> bool;
    };

} // namespace glsld
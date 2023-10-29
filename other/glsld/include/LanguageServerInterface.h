#pragma once
#include "Protocol.h"

namespace glsld
{
    // This is bridge interface between services and the language server
    // and it translate data between native data structure and json.
    class LanguageServerCallback
    {
    public:
        // Submit a message from the client to language server and dispatch it to the right handler.
        auto HandleClientMessage(std::string_view messageText) -> void
        {
            auto message = lsp::ParseJson(messageText);
            if (message) {
                DoHandleClientMessage(std::move(*message));
            }
            else {
                DoHandleBadClientMessage(messageText);
            }
        }

        // Submit a response from the language service to language server and forward it to the client.
        template <typename T>
        auto HandleServerResponse(int requestId, const T& params, bool isError) -> void
        {
            DoHandleServerResponse(requestId, lsp::ToJson(params), isError);
        }

        // Submit a notification from the language service to language server and forward it to the client.
        template <typename T>
        auto HandleServerNotification(const char* method, const T& params) -> void
        {
            DoHandleNotification(method, lsp::ToJson(params));
        }

    protected:
        virtual auto DoHandleBadClientMessage(std::string_view messageText) -> void = 0;

        virtual auto DoHandleClientMessage(lsp::JsonObject rpcBlob) -> void = 0;

        virtual auto DoHandleServerResponse(int requestId, lsp::JsonObject resultOrError, bool isError) -> void = 0;

        virtual auto DoHandleNotification(const char* method, lsp::JsonObject param) -> void = 0;
    };

} // namespace glsld
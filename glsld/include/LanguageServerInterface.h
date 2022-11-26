#pragma once
#include "Protocol.h"

namespace glsld
{
    // this is bridge between services and the language server
    // and it translate data between raw data and json
    class LanguageServerCallback
    {
    public:
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

        template <typename T>
        auto HandleServerResponse(int requestId, const T& params, bool isError) -> void
        {
            DoHandleServerResponse(requestId, lsp::ToJson(params), isError);
        }

        template <typename T>
        auto HandleServerNotification(const char* method, const T& params) -> void
        {
            DoHandleNotification(method, lsp::ToJson(params));
        }

    protected:
        virtual auto DoHandleBadClientMessage(std::string_view messageText) -> void = 0;

        virtual auto DoHandleClientMessage(lsp::JsonObject rpcBlob) -> void = 0;

        virtual auto DoHandleServerRequest(int requestId, lsp::JsonObject params) -> void = 0;

        virtual auto DoHandleServerResponse(int requestId, lsp::JsonObject resultOrError, bool isError) -> void = 0;

        virtual auto DoHandleNotification(const char* method, lsp::JsonObject param) -> void = 0;
    };

} // namespace glsld
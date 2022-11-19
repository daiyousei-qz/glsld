#pragma once
#include "Protocol.h"

namespace glsld
{
    // this is bridge between services and the language server
    // and it translate data between raw data and json
    class LanguageServerCallback
    {
    public:
        auto HandleClientRequest(std::string_view rpcBlob) -> void
        {
            DoHandleClientRequest(lsp::JsonObject::parse(rpcBlob));
        }

        template <typename T>
        auto HandleServerResponse(int requestId, const T& params, bool isError) -> void
        {
            DoHandleServerResponse(requestId, lsp::ToJson(params), isError);
        }

        template <typename T>
        auto HandleServerNotification(const char* method, const T& params)
        {
            DoHandleNotification(method, lsp::ToJson(params));
        }

    protected:
        virtual auto DoHandleClientRequest(lsp::JsonObject rpcBlob) -> void = 0;

        // virtual auto DoHandleServerRequest(lsp::JsonObject result) -> void = 0;

        virtual auto DoHandleServerResponse(int requestId, lsp::JsonObject result, bool isError) -> void = 0;

        virtual auto DoHandleNotification(const char* method, lsp::JsonObject param) -> void = 0;
    };

} // namespace glsld
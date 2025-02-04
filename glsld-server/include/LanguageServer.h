#pragma once

#include "Basic/Common.h"

#include "LanguageServerCallback.h"
#include "TransportService.h"
#include "LanguageService.h"

#include <memory>

namespace glsld
{
    // handles jsonrpc
    class LanguageServer : private LanguageServerCallback
    {
    private:
        using MessageDispatcherType = std::function<void(LanguageServer& server, const JsonObject& rpcBlob)>;
        std::unordered_map<std::string, MessageDispatcherType> dispatcherMap;

        std::unique_ptr<LanguageService> language;
        std::unique_ptr<TransportService> transport;

    public:
        LanguageServer(const LanguageServerConfig& config) : LanguageServerCallback(config)
        {
            Initialize();
        }

        auto Run() -> void;

    private:
        auto Initialize() -> void;

        auto DoHandleClientMessage(JsonObject rpcBlob) -> void override;
        auto DoHandleBadClientMessage(StringView messageText) -> void override;
        auto DoHandleServerResponse(int requestId, JsonObject result, bool isError) -> void override;
        auto DoHandleNotification(const char* method, JsonObject params) -> void override;

        template <typename ParamType>
        using RequestHandlerType = void (LanguageService::*)(int requestId, ParamType params);

        template <typename ParamType>
        auto AddRequestHandler(StringView methodName, RequestHandlerType<ParamType> handler) -> void
        {
            auto [it, inserted] = dispatcherMap.insert_or_assign(
                methodName.Str(), [handler](LanguageServer& server, const JsonObject& rpcBlob) {
                    const auto& jreqid = rpcBlob["id"];
                    if (!jreqid.is_number_integer()) {
                        return;
                    }
                    int requestId = jreqid;

                    ParamType params;
                    if (FromJson(rpcBlob["params"], params)) {
                        std::invoke(handler, server.language.get(), requestId, std::move(params));
                    }
                });
            GLSLD_ASSERT(inserted);
        }

        template <typename ParamType>
        using NotificationHandlerType = void (LanguageService::*)(ParamType params);

        template <typename ParamType>
        auto AddNotificationHandler(StringView methodName, NotificationHandlerType<ParamType> handler) -> void
        {
            auto [it, inserted] = dispatcherMap.insert_or_assign(
                methodName.Str(), [handler](LanguageServer& server, const JsonObject& rpcBlob) {
                    ParamType params;
                    if (FromJson(rpcBlob["params"], params)) {
                        std::invoke(handler, server.language.get(), std::move(params));
                    }
                });
            GLSLD_ASSERT(inserted);
        }
    };
} // namespace glsld
#pragma once

#include "Common.h"
#include "Protocol.h"
#include "LanguageServerInterface.h"
#include "TransportService.h"
#include "LanguageService.h"
#include "ThreadingService.h"

#include <memory>
#include <thread>
#include <chrono>

namespace glsld
{
    // handles jsonrpc
    class LanguageServer : public LanguageServerCallback
    {
    public:
        auto Run() -> void
        {
            using namespace std::literals;

            Initialize();
            // std::this_thread::sleep_for(10s);

            // std::jthread thd{[](LanguageServer* server) {
            //                      while (true) {
            //                          std::this_thread::sleep_for(5s);
            //                          lsp::ShowMessageParams params{
            //                              .type    = lsp::MessageType::Info,
            //                              .message = "hello lsp!",
            //                          };
            //                          server->language->ShowMessage(params);
            //                      }
            //                  },
            //                  this};

            while (true) {
                std::this_thread::sleep_for(100ms);
                transport->PullMessage();
            }
        }

    private:
        auto Initialize() -> void
        {
            language  = std::make_unique<LanguageService>(this);
            transport = std::make_unique<TransportService>(stdin, stdout, this);
            threading = std::make_unique<ThreadingService>();

            InitializeClientMessageHandler();
        }

    protected:
        auto DoHandleClientMessage(lsp::JsonObject rpcBlob) -> void override
        {
            const auto& jmethod = rpcBlob["method"];
            if (!jmethod.is_string()) {
                // FIXME: handle bad rpc blob
                return;
            }
            std::string method = jmethod;

            if (auto it = dispatcherMap.find(method); it != dispatcherMap.end()) {
                it->second(*this, rpcBlob);
            }
            else {
                // FIXME: handle unknown method
            }
        }

        auto DoHandleBadClientMessage(std::string_view messageText) -> void override
        {
            // FIXME: log this?
        }

        auto DoHandleServerRequest(int requestId, lsp::JsonObject params) -> void override
        {
        }

        auto DoHandleServerResponse(int requestId, lsp::JsonObject result, bool isError) -> void override
        {
            lsp::JsonObject rpcBlob;
            rpcBlob["jsonrpc"] = "2.0";
            rpcBlob["id"]      = requestId;
            if (isError) {
                rpcBlob["error"] = std::move(result);
            }
            else {
                rpcBlob["result"] = std::move(result);
            }

            auto payload = rpcBlob.dump();
            transport->PushMessage(payload);
        }

        auto DoHandleNotification(const char* method, lsp::JsonObject params) -> void override
        {
            lsp::JsonObject rpcBlob;
            rpcBlob["jsonrpc"] = "2.0";
            rpcBlob["method"]  = method;
            rpcBlob["params"]  = std::move(params);

            auto payload = rpcBlob.dump();
            transport->PushMessage(payload);
        }

    private:
        auto InitializeClientMessageHandler() -> void
        {
            AddRequestHandler(lsp::LSPMethod_Initialize, &LanguageService::Initialize);
            AddRequestHandler(lsp::LSPMethod_DocumentSymbol, &LanguageService::DocumentSymbol);

            AddNotificationHandler(lsp::LSPMethod_DidOpenTextDocument, &LanguageService::DidOpenTextDocument);
            AddNotificationHandler(lsp::LSPMethod_DidChangeTextDocument, &LanguageService::DidChangeTextDocument);
            AddNotificationHandler(lsp::LSPMethod_DidCloseTextDocument, &LanguageService::DidCloseTextDocument);
        };

        template <typename ParamType>
        using RequestHandlerType = void (LanguageService::*)(int requestId, ParamType params);

        template <typename ParamType>
        auto AddRequestHandler(std::string_view methodName, RequestHandlerType<ParamType> handler) -> void
        {
            auto [it, inserted] = dispatcherMap.insert_or_assign(
                std::string{methodName}, [=](LanguageServer& server, const lsp::JsonObject& rpcBlob) {
                    const auto& jreqid = rpcBlob["id"];
                    if (!jreqid.is_number_integer()) {
                        return;
                    }
                    int requestId = jreqid;

                    ParamType params;
                    if (lsp::FromJson(rpcBlob["params"], params)) {
                        std::invoke(handler, server.language.get(), requestId, std::move(params));
                    }
                });
            GLSLD_ASSERT(inserted);
        }

        template <typename ParamType>
        using NotificationHandlerType = void (LanguageService::*)(ParamType params);

        template <typename ParamType>
        auto AddNotificationHandler(std::string_view methodName, NotificationHandlerType<ParamType> handler) -> void
        {
            auto [it, inserted] = dispatcherMap.insert_or_assign(
                std::string{methodName}, [=](LanguageServer& server, const lsp::JsonObject& rpcBlob) {
                    ParamType params;
                    if (lsp::FromJson(rpcBlob["params"], params)) {
                        std::invoke(handler, server.language.get(), std::move(params));
                    }
                });
            GLSLD_ASSERT(inserted);
        }

        using MessageDispatcherType = std::function<void(LanguageServer& server, const lsp::JsonObject& rpcBlob)>;
        std::unordered_map<std::string, MessageDispatcherType> dispatcherMap;

        std::unique_ptr<LanguageService> language;
        std::unique_ptr<TransportService> transport;
        std::unique_ptr<ThreadingService> threading;
    };
} // namespace glsld
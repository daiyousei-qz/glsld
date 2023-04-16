#pragma once

#include "Common.h"
#include "Protocol.h"
#include "LanguageServerInterface.h"
#include "TransportService.h"
#include "LanguageService.h"
#include "ThreadingService.h"

#include <memory>

namespace glsld
{
    auto InitializeStdIO() -> void;

    // handles jsonrpc
    class LanguageServer : public LanguageServerCallback
    {
    public:
        auto Run() -> void
        {
            Initialize();

            while (true) {
                transport->PullMessage();
            }
        }

    private:
        auto Initialize() -> void
        {
            InitializeStdIO();

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
            transport->PushMessage(StringView{payload});
        }

        auto DoHandleNotification(const char* method, lsp::JsonObject params) -> void override
        {
            lsp::JsonObject rpcBlob;
            rpcBlob["jsonrpc"] = "2.0";
            rpcBlob["method"]  = method;
            rpcBlob["params"]  = std::move(params);

            auto payload = rpcBlob.dump();
            transport->PushMessage(StringView{payload});
        }

    private:
        auto InitializeClientMessageHandler() -> void
        {
            AddRequestHandler(lsp::LSPMethod_Initialize, &LanguageService::Initialize);
            AddRequestHandler(lsp::LSPMethod_DocumentSymbol, &LanguageService::DocumentSymbol);
            AddRequestHandler(lsp::LSPMethod_SemanticTokensFull, &LanguageService::SemanticTokensFull);
            AddRequestHandler(lsp::LSPMethod_Completion, &LanguageService::Completion);
            AddRequestHandler(lsp::LSPMethod_SignatureHelp, &LanguageService::SignatureHelp);
            AddRequestHandler(lsp::LSPMethod_Hover, &LanguageService::Hover);
            AddRequestHandler(lsp::LSPMethod_Declaration, &LanguageService::Declaration);
            AddRequestHandler(lsp::LSPMethod_Definition, &LanguageService::Definition);
            AddRequestHandler(lsp::LSPMethod_InlayHint, &LanguageService::InlayHint);
            AddRequestHandler(lsp::LSPMethod_DocumentColor, &LanguageService::DocumentColor);

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
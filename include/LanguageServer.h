#pragma once

#include "Common.h"
#include "Protocol.h"
#include "LanguageServerInterface.h"
#include "TransportService.h"
#include "LanguageService.h"

#include <memory>
#include <thread>
#include <chrono>

namespace glsld
{
    struct LSPMethodDesc
    {
        static constexpr const char* id = "initialize";

        using ClientMessageType = lsp::InitializeParams;
        using ServerMessageType = lsp::InitializedResult;
        using ServerErrorType   = lsp::InitializeError;
    };

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
            // auto inFile = fopen("test_lsp.txt", "rb");
            // transport   = std::make_unique<TransportService>(inFile, stdout, this);
        }

        template <typename ParamType>
        auto DispatchClientRequest(int requestId, ParamType params)
        {
        }

        auto DispatchClientMessage(std::string_view method, const lsp::JsonObject& rpcBlob) -> void
        {
            static const std::map<std::string, std::function<void(const lsp::JsonObject&)>> requestHandlers{

            };
        }

    protected:
        auto DoHandleClientMessage(lsp::JsonObject rpcBlob) -> void override
        {
            const auto& jmethod = rpcBlob["method"];
            if (!jmethod.is_string()) {
                return;
            }

            std::string s = jmethod;
            if (s == lsp::LSPMethod_Initialize) {
                const auto& jreqid = rpcBlob["id"];
                if (!jreqid.is_number_integer()) {
                    return;
                }
                int requestId = jreqid;

                lsp::InitializeParams params;
                if (lsp::FromJson(rpcBlob["params"], params)) {
                    language->Initialize(requestId, params);
                }
            }
            if (s == lsp::LSPMethod_DocumentSymbol) {
                const auto& jreqid = rpcBlob["id"];
                if (!jreqid.is_number_integer()) {
                    return;
                }
                int requestId = jreqid;

                lsp::DocumentSymbolParams params;
                if (lsp::FromJson(rpcBlob["params"], params)) {
                    language->DocumentSymbol(requestId, params);
                }
            }
            else if (s == lsp::LSPMethod_DidOpenTextDocument) {
                lsp::DidOpenTextDocumentParams params;
                if (lsp::FromJson(rpcBlob["params"], params)) {
                    language->DidOpenTextDocument(params);
                }
            }
            else if (s == lsp::LSPMethod_DidChangeTextDocument) {
                lsp::DidChangeTextDocumentParams params;
                if (lsp::FromJson(rpcBlob["params"], params)) {
                    language->DidChangeTextDocument(params);
                }
            }
            else if (s == lsp::LSPMethod_DidCloseTextDocument) {
                lsp::DidCloseTextDocumentParams params;
                if (lsp::FromJson(rpcBlob["params"], params)) {
                    language->DidCloseTextDocument(params);
                }
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

        std::unique_ptr<LanguageService> language;
        std::unique_ptr<TransportService> transport;
    };
} // namespace glsld
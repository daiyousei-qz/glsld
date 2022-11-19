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
    // handles jsonrpc
    class LanguageServer : public LanguageServerCallback
    {
    public:
        auto Run() -> void
        {
            using namespace std::literals;

            Initialize();

            while (true) {
                transport->PullMessage();
                std::this_thread::sleep_for(1000ms);

                lsp::ShowMessageParams params{
                    .type    = lsp::MessageType::Info,
                    .message = "hello lsp!",
                };
                language->ShowMessage(params);
            }
        }

    protected:
        auto DoHandleClientRequest(lsp::JsonObject rpcBlob) -> void override
        {
            const auto& jreqid = rpcBlob["id"];
            if (!jreqid.is_number_integer()) {
                return;
            }
            const auto& jmethod = rpcBlob["method"];
            if (!jmethod.is_string()) {
                return;
            }

            int requestId = jreqid;
            std::string s = jmethod;
            if (s == lsp::LSPMethod_Initialize) {
                lsp::InitializeParams params;
                if (lsp::FromJson(rpcBlob["params"], params)) {
                    language->Initialize(requestId, params);
                }
            }
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
        auto Initialize() -> void
        {
            language = std::make_unique<LanguageService>(this);
            // transport = std::make_unique<TransportService>(stdin, stdout, this);
            auto inFile = fopen("test_lsp.txt", "rb");
            transport   = std::make_unique<TransportService>(inFile, stdout, this);
        }

        std::unique_ptr<LanguageService> language;
        std::unique_ptr<TransportService> transport;
    };
} // namespace glsld
#include "Server/LanguageServer.h"
#include "Server/LanguageService.h"
#include "Server/TransportService.h"

#include <cstdio>

#if defined(GLSLD_OS_WIN)
#include <io.h>
#include <fcntl.h>
#endif

namespace glsld
{
    LanguageServer::LanguageServer(const LanguageServerConfig& config) : config(config)
    {
        Initialize();
    }
    LanguageServer::~LanguageServer()
    {
    }

    auto CreateDefaultLanguageService(LanguageServer& server) -> std::unique_ptr<LanguageService>
    {
        return std::make_unique<LanguageService>(server);
    }

    auto CreateStdIOTransportService(LanguageServer& server) -> std::unique_ptr<TransportService>
    {
#if defined(GLSLD_OS_WIN)
        // Use binary mode for stdin/stdout. We handle "/r/n" conversion inhouse.
        _setmode(_fileno(stdout), O_BINARY);
        _setmode(_fileno(stdin), O_BINARY);
#else
        // FIXME: not sure if this is needed
        // freopen(nullptr, "rb", stdout);
        // freopen(nullptr, "wb", stdin);
#endif

        setvbuf(stdin, nullptr, _IOFBF, 64 * 1024);
        return std::make_unique<TransportService>(server, stdin, stdout);
    }

    auto LanguageServer::Run() -> void
    {
        while (true) {
            transport->PullMessage();
        }
    }

    auto LanguageServer::Initialize() -> void
    {
        logger = spdlog::stderr_color_mt("glsld_logger");
        logger->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
        switch (config.loggingLevel) {
        case LoggingLevel::Trace:
            logger->set_level(spdlog::level::trace);
            break;
        case LoggingLevel::Debug:
            logger->set_level(spdlog::level::debug);
            break;
        case LoggingLevel::Info:
            logger->set_level(spdlog::level::info);
            break;
        case LoggingLevel::Warn:
            logger->set_level(spdlog::level::warn);
            break;
        case LoggingLevel::Error:
            logger->set_level(spdlog::level::err);
            break;
        case LoggingLevel::Critical:
            logger->set_level(spdlog::level::critical);
            break;
        }

        language  = CreateDefaultLanguageService(*this);
        transport = CreateStdIOTransportService(*this);

        AddRequestHandler(lsp::LSPMethod_Initialize, &LanguageService::Initialize);
        AddRequestHandler(lsp::LSPMethod_DocumentSymbol, &LanguageService::DocumentSymbol);
        AddRequestHandler(lsp::LSPMethod_SemanticTokensFull, &LanguageService::SemanticTokensFull);
        AddRequestHandler(lsp::LSPMethod_Completion, &LanguageService::Completion);
        AddRequestHandler(lsp::LSPMethod_SignatureHelp, &LanguageService::SignatureHelp);
        AddRequestHandler(lsp::LSPMethod_Hover, &LanguageService::Hover);
        AddRequestHandler(lsp::LSPMethod_References, &LanguageService::References);
        AddRequestHandler(lsp::LSPMethod_Declaration, &LanguageService::Declaration);
        AddRequestHandler(lsp::LSPMethod_InlayHint, &LanguageService::InlayHint);

        AddNotificationHandler(lsp::LSPMethod_DidOpenTextDocument, &LanguageService::DidOpenTextDocument);
        AddNotificationHandler(lsp::LSPMethod_DidChangeTextDocument, &LanguageService::DidChangeTextDocument);
        AddNotificationHandler(lsp::LSPMethod_DidCloseTextDocument, &LanguageService::DidCloseTextDocument);
    }

    auto LanguageServer::HandleClientMessage(StringView messagePayload) -> void
    {
        auto j = nlohmann::json::parse(messagePayload, nullptr, false);
        if (j.is_discarded()) {
            LogError("Client message is not a valid JSON:\n```\n{}```\n", messagePayload);
            return;
        }

        const auto& jmethod = j["method"];
        if (!jmethod.is_string()) {
            LogError("JSON-RPC method is not valid.");
            return;
        }
        std::string method = jmethod;

        if (auto it = dispatcherMap.find(method); it != dispatcherMap.end()) {
            it->second(*this, j);
        }
        else {
            LogError("Unknown JSON-RPC method: {}", method);
        }
    }

    auto LanguageServer::DoHandleServerResponse(int requestId, nlohmann::json result, bool isError) -> void
    {
        nlohmann::json rpcBlob;
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

    auto LanguageServer::DoHandleNotification(const char* method, nlohmann::json params) -> void
    {
        nlohmann::json rpcBlob;
        rpcBlob["jsonrpc"] = "2.0";
        rpcBlob["method"]  = method;
        rpcBlob["params"]  = std::move(params);

        auto payload = rpcBlob.dump();
        transport->PushMessage(StringView{payload});
    }
} // namespace glsld
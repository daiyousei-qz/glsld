#include "LanguageServer.h"

#include <cstdio>

#if defined(GLSLD_OS_WIN)
#include <io.h>
#include <fcntl.h>
#endif

namespace glsld
{
    auto CreateDefaultLanguageService(LanguageServerCallback& server) -> std::unique_ptr<LanguageService>
    {
        return std::make_unique<LanguageService>(server);
    }

    auto CreateStdIOTransportService(LanguageServerCallback& server) -> std::unique_ptr<TransportService>
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
        Initialize();

        while (true) {
            transport->PullMessage();
        }
    }

    auto LanguageServer::Initialize() -> void
    {
        language  = CreateDefaultLanguageService(static_cast<LanguageServerCallback&>(*this));
        transport = CreateStdIOTransportService(static_cast<LanguageServerCallback&>(*this));

        AddRequestHandler(lsp::LSPMethod_Initialize, &LanguageService::Initialize);
        AddRequestHandler(lsp::LSPMethod_DocumentSymbol, &LanguageService::DocumentSymbol);
        AddRequestHandler(lsp::LSPMethod_SemanticTokensFull, &LanguageService::SemanticTokensFull);
        AddRequestHandler(lsp::LSPMethod_Completion, &LanguageService::Completion);
        AddRequestHandler(lsp::LSPMethod_SignatureHelp, &LanguageService::SignatureHelp);
        AddRequestHandler(lsp::LSPMethod_Hover, &LanguageService::Hover);
        AddRequestHandler(lsp::LSPMethod_References, &LanguageService::References);
        AddRequestHandler(lsp::LSPMethod_Declaration, &LanguageService::Declaration);
        AddRequestHandler(lsp::LSPMethod_Definition, &LanguageService::Definition);
        AddRequestHandler(lsp::LSPMethod_InlayHint, &LanguageService::InlayHint);

        AddNotificationHandler(lsp::LSPMethod_DidOpenTextDocument, &LanguageService::DidOpenTextDocument);
        AddNotificationHandler(lsp::LSPMethod_DidChangeTextDocument, &LanguageService::DidChangeTextDocument);
        AddNotificationHandler(lsp::LSPMethod_DidCloseTextDocument, &LanguageService::DidCloseTextDocument);
    }

    auto LanguageServer::DoHandleClientMessage(lsp::JsonObject rpcBlob) -> void
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

    auto LanguageServer::DoHandleBadClientMessage(StringView messageText) -> void
    {
        // FIXME: log this?
    }

    auto LanguageServer::DoHandleServerResponse(int requestId, lsp::JsonObject result, bool isError) -> void
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

    auto LanguageServer::DoHandleNotification(const char* method, lsp::JsonObject params) -> void
    {
        lsp::JsonObject rpcBlob;
        rpcBlob["jsonrpc"] = "2.0";
        rpcBlob["method"]  = method;
        rpcBlob["params"]  = std::move(params);

        auto payload = rpcBlob.dump();
        transport->PushMessage(StringView{payload});
    }

} // namespace glsld
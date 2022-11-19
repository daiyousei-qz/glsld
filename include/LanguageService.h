#pragma once
#include "Protocol.h"
#include "LanguageServerInterface.h"

namespace glsld
{
    class LanguageService
    {
    public:
        LanguageService(LanguageServerCallback* server) : server(server)
        {
        }

        auto Initialize(int requestId, lsp::InitializeParams params) -> void
        {
            auto result = lsp::InitializedResult{
                .capabilities = {},
                .serverInfo =
                    {
                        .name    = "glsld",
                        .version = "0.0.1",
                    },
            };
            server->HandleServerResponse(requestId, result, false);
        }

#pragma region Document Synchronization

        auto DidOpenTextDocument(int requestId, lsp::DidOpenTextDocumentParams params) -> void
        {
        }
        auto DidChangeTextDocument(int requestId, lsp::DidChangeTextDocumentParams params) -> void
        {
        }
        auto DidCloseDocument(int requestId, lsp::DidCloseTextDocumentParams params) -> void
        {
        }

#pragma endregion

#pragma region Language Features

        auto SemanticTokensFull(lsp::SemanticTokensParam params) -> void
        {
        }

#pragma endregion

#pragma region Window Features

        auto ShowMessage(lsp::ShowMessageParams params) -> void
        {
            server->HandleServerNotification(lsp::LSPMethod_ShowMessage, params);
        }

#pragma endregion

    private:
        LanguageServerCallback* server;
    };
} // namespace glsld
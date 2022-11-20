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
                .capabilities =
                    {
                        .textDocumentSync =
                            {
                                .openClose = true,
                                .change    = lsp::TextDocumentSyncKind::Full,
                            },
                        .documentSymbolProvider = true,
                    },
                .serverInfo =
                    {
                        .name    = "glsld",
                        .version = "0.0.1",
                    },
            };
            server->HandleServerResponse(requestId, result, false);
        }

#pragma region Document Synchronization

        auto DidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void
        {
            sourceMap[params.textDocument.uri] = params.textDocument.text;
        }
        auto DidChangeTextDocument(lsp::DidChangeTextDocumentParams params) -> void
        {
            sourceMap[params.textDocument.uri] = params.contentChanges[0].text;
        }
        auto DidCloseTextDocument(lsp::DidCloseTextDocumentParams params) -> void
        {
            sourceMap.erase(params.textDocument);
        }

#pragma endregion

#pragma region Language Features

        auto DocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void
        {
            std::vector<lsp::DocumentSymbol> result;
            lsp::Range testRange{
                .start =
                    {
                        .line      = 0,
                        .character = 0,
                    },
                .end =
                    {
                        .line      = 0,
                        .character = 1,
                    },
            };
            result.push_back(lsp::DocumentSymbol{
                .name           = "firstSymbol",
                .kind           = lsp::SymbolKind::Variable,
                .range          = testRange,
                .selectionRange = testRange,
            });
            result.push_back(lsp::DocumentSymbol{
                .name           = "secondSymbol",
                .kind           = lsp::SymbolKind::Function,
                .range          = testRange,
                .selectionRange = testRange,
            });

            server->HandleServerResponse(requestId, result, false);
        }

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
        // uri -> source
        std::map<std::string, std::string> sourceMap;

        LanguageServerCallback* server;
    };
} // namespace glsld
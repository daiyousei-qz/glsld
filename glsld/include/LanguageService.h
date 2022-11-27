#pragma once
#include "Protocol.h"
#include "LanguageServerInterface.h"
#include "Compiler.h"

namespace glsld
{
    auto ComputeDocumentSymbol(GlsldCompiler& compiler) -> std::vector<lsp::DocumentSymbol>;
    auto ComputeSemanticTokens() -> lsp::SemanticTokens;

    // this class manages online source codes
    class SourceManager
    {
    };

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
            sourceMap.erase(params.textDocument.uri);
        }

#pragma endregion

#pragma region Language Features

        auto DocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void
        {
            GlsldCompiler compiler;
            compiler.Compile(sourceMap[params.textDocument.uri]);

            std::vector<lsp::DocumentSymbol> result = ComputeDocumentSymbol(compiler);
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
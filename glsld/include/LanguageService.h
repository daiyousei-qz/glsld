#pragma once
#include "Protocol.h"
#include "LanguageServerInterface.h"
#include "Compiler.h"

namespace glsld
{
    auto ComputeDocumentSymbol(GlsldCompiler& compiler) -> std::vector<lsp::DocumentSymbol>;

    auto GetTokenLegend() -> lsp::SemanticTokensLegend;
    auto ComputeSemanticTokens(GlsldCompiler& compiler) -> lsp::SemanticTokens;
    auto ComputeSemanticTokensDelta(GlsldCompiler& compiler) -> lsp::SemanticTokensDelta;

    auto ComputeHover(GlsldCompiler& compiler, lsp::Position position) -> std::optional<lsp::Hover>;

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
                        .hoverProvider          = true,
                        .declarationProvider    = true,
                        .definitionProvider     = true,
                        .documentSymbolProvider = true,
                        .semanticTokensProvider =
                            {
                                .legend = GetTokenLegend(),
                                .range  = false,
                                .full   = true,
                                .delta  = false,
                            },
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

        auto SemanticTokensFull(int requestId, lsp::SemanticTokensParam params) -> void
        {
            GlsldCompiler compiler;
            compiler.Compile(sourceMap[params.textDocument.uri]);

            lsp::SemanticTokens result = ComputeSemanticTokens(compiler);
            server->HandleServerResponse(requestId, result, false);
        }

        auto Completion(int requestId, lsp::CompletionParams params) -> void
        {
        }

        auto Hover(int requestId, lsp::HoverParams params) -> void
        {
            GlsldCompiler compiler;
            compiler.Compile(sourceMap[params.baseParams.textDocument.uri]);

            std::optional<lsp::Hover> result = ComputeHover(compiler, params.baseParams.position);
            server->HandleServerResponse(requestId, result, false);
        }

        auto Declaration(int requestId, lsp::DeclarationParams params) -> void
        {
            // GlsldCompiler compiler;
            // compiler.Compile(sourceMap[params.baseParams.textDocument.uri]);

            std::vector<lsp::Location> result;
            server->HandleServerResponse(requestId, result, false);
        }

        auto Definition(int requestId, lsp::DefinitionParams params) -> void
        {
            // GlsldCompiler compiler;
            // compiler.Compile(sourceMap[params.baseParams.textDocument.uri]);

            std::vector<lsp::Location> result;
            server->HandleServerResponse(requestId, result, false);
        }

        // auto InlayHint(int requestId, lsp::InlayHintParams params) -> void {
        // }

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
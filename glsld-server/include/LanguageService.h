#pragma once
#include "Config.h"
#include "Protocol.h"
#include "LanguageServerCallback.h"
#include "LanguageQueryProvider.h"

namespace glsld
{
    class LanguageService
    {
    private:
        LanguageServerCallback& server;

        // uri -> provider
        std::map<std::string, std::shared_ptr<PendingBackgroundCompilation>> providerLookup;

    public:
        LanguageService(LanguageServerCallback& server) : server(server)
        {
        }

        auto Initialize(int requestId, lsp::InitializeParams params) -> void;

#pragma region Document Synchronization

        auto DidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void;

        auto DidChangeTextDocument(lsp::DidChangeTextDocumentParams params) -> void;

        auto DidCloseTextDocument(lsp::DidCloseTextDocumentParams params) -> void;

#pragma endregion

#pragma region Language Features

        auto DocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void;

        auto SemanticTokensFull(int requestId, lsp::SemanticTokensParam params) -> void;

        auto Completion(int requestId, lsp::CompletionParams params) -> void;

        auto SignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void;

        auto Hover(int requestId, lsp::HoverParams params) -> void;

        auto Declaration(int requestId, lsp::DeclarationParams params) -> void;

        auto Definition(int requestId, lsp::DefinitionParams params) -> void;

        auto References(int requestId, lsp::ReferenceParams params) -> void;

        auto InlayHint(int requestId, lsp::InlayHintParams params) -> void;

#pragma endregion

#pragma region Window Features

        auto ShowMessage(lsp::ShowMessageParams params) -> void;

#pragma endregion

    private:
        // Schedule a language query for the given uri in a background thread, which waits for the compilation and then
        // runs the callback.
        auto ScheduleLanguageQuery(const std::string& uri,
                                   std::function<auto(const LanguageQueryProvider&)->void> callback) -> void
        {
            // NOTE we need to make a copy of the provider here so it doesn't get released while in analysis
            auto provider = providerLookup[uri];
            if (provider != nullptr) {
                server.ScheduleTask([this, provider = std::move(provider), callback = std::move(callback)] {
                    if (provider->WaitAvailable()) {
                        callback(provider->GetProvider());
                    }
                });
            }
        }
    };
} // namespace glsld
#pragma once
#include "Protocol.h"
#include "LanguageServerInterface.h"
#include "LanguageQueryProvider.h"

#include <BS_thread_pool.hpp>

namespace glsld
{
    class LanguageService
    {
    private:
        // uri -> provider
        std::map<std::string, std::shared_ptr<PendingBackgroundCompilation>> providerLookup;

        LanguageServerCallback* server;

        BS::thread_pool threadPool;

    public:
        LanguageService(LanguageServerCallback* server) : server(server)
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
        auto ScheduleLanguageQuery(const std::string& uri,
                                   std::function<auto(const LanguageQueryProvider&)->void> callback) -> void
        {
            // NOTE we need to make a copy of the provider here so it doesn't get released while in analysis
            auto provider = providerLookup[uri];
            if (provider != nullptr) {
                ScheduleTask([this, provider = std::move(provider), callback = std::move(callback)] {
                    if (provider->WaitAvailable()) {
                        callback(provider->GetProvider());
                    }
                });
            }
        }

        template <typename F, typename... Args>
        auto ScheduleTask(F&& f, Args&&... args) -> void
        {
            threadPool.detach_task(std::forward<F>(f), std::forward<Args>(args)...);
        }
    };
} // namespace glsld
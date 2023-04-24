#pragma once
#include "PPCallback.h"
#include "Protocol.h"
#include "LanguageServerInterface.h"
#include "SourceText.h"
#include "LanguageQueryProvider.h"

#include <BS_thread_pool.hpp>

namespace glsld
{
    auto ComputeDocumentSymbol(const LanguageQueryProvider& provider) -> std::vector<lsp::DocumentSymbol>;

    auto GetTokenLegend() -> lsp::SemanticTokensLegend;
    auto ComputeSemanticTokens(const LanguageQueryProvider& provider) -> lsp::SemanticTokens;
    // auto ComputeSemanticTokensDelta(const IntellisenseProvider& provider) -> lsp::SemanticTokensDelta;

    auto ComputeCompletion(const LanguageQueryProvider& provider, lsp::Position position)
        -> std::vector<lsp::CompletionItem>;

    auto ComputeSignatureHelp(const LanguageQueryProvider& provider, lsp::Position position)
        -> std::optional<lsp::SignatureHelp>;

    auto ComputeHover(const LanguageQueryProvider& provider, lsp::Position position) -> std::optional<lsp::Hover>;

    // we assume single source file for now
    auto ComputeDeclaration(const LanguageQueryProvider& provider, const lsp::DocumentUri& uri, lsp::Position position)
        -> std::vector<lsp::Location>;

    auto ComputeReferences(const LanguageQueryProvider& provider, const lsp::DocumentUri& uri, lsp::Position position,
                           bool includeDeclaration) -> std::vector<lsp::Location>;

    auto ComputeInlayHint(const LanguageQueryProvider& provider, lsp::Range range) -> std::vector<lsp::InlayHint>;

    auto ComputeDocumentColor(const LanguageQueryProvider& provider) -> std::vector<lsp::ColorInformation>;

    class LanguageService
    {
    public:
        LanguageService(LanguageServerCallback* server) : server(server)
        {
        }

        auto Initialize(int requestId, lsp::InitializeParams params) -> void
        {
            auto completionTriggerCharacters    = std::vector<std::string>{"."};
            auto signatureHelpTriggerCharacters = std::vector<std::string>{"("};

            auto result = lsp::InitializedResult{
                .capabilities =
                    {
                        .textDocumentSync =
                            {
                                .openClose = true,
                                .change    = lsp::TextDocumentSyncKind::Incremental,
                            },
                        .completionProvider =
                            lsp::CompletionOptions{
                                // FIXME: this is a MSVC ICE workaround
                                .triggerCharacters   = std::move(completionTriggerCharacters),
                                .allCommitCharacters = {},
                                .resolveProvider     = false,
                            },
                        .hoverProvider = true,
                        .signatureHelpProvider =
                            lsp::SignatureHelpOptions{
                                // FIXME: this is a MSVC ICE workaround
                                .triggerCharacters = std::move(signatureHelpTriggerCharacters),
                            },
                        .declarationProvider    = true,
                        .definitionProvider     = true,
                        .referenceProvider      = true,
                        .documentSymbolProvider = true,
                        .semanticTokensProvider =
                            lsp::SemanticTokenOptions{
                                .legend = GetTokenLegend(),
                                .range  = false,
                                .full   = true,
                                .delta  = false,
                            },
                        .inlayHintProvider =
                            lsp::InlayHintOptions{
                                .resolveProvider = false,
                            },
                        .colorProvider  = false,
                        .renameProvider = std::nullopt,
                    },
                .serverInfo =
                    {
                        .name    = "glsld",
                        .version = "0.0.1",
                    },
            };
            server->HandleServerResponse(requestId, result, false);
        } // namespace glsld

#pragma region Document Synchronization

        auto DidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void
        {
            auto& providerEntry = providerLookup[params.textDocument.uri];
            if (providerEntry) {
                // Bad request
                return;
            }

            providerEntry = std::make_shared<PendingIntellisenseProvider>(params.textDocument.version,
                                                                          UnescapeHttp(params.textDocument.uri),
                                                                          std::move(params.textDocument.text));

            ScheduleTask([provider = providerEntry]() { provider->Setup(); });
        }
        auto DidChangeTextDocument(lsp::DidChangeTextDocumentParams params) -> void
        {
            auto& providerEntry = providerLookup[params.textDocument.uri];
            if (!providerEntry) {
                // Bad request
                return;
            }

            // TODO: Could have a buffer manager so we don't keep allocating new buffers if user types faster than
            // compilation
            auto sourceBuffer = providerEntry->StealBuffer();
            for (const auto& change : params.contentChanges) {
                if (change.range) {
                    ApplySourceChange(sourceBuffer, FromLspRange(*change.range), StringView{change.text});
                }
                else {
                    sourceBuffer = std::move(change.text);
                }
            }
            providerEntry = std::make_shared<PendingIntellisenseProvider>(
                params.textDocument.version, UnescapeHttp(params.textDocument.uri), std::move(sourceBuffer));

            ScheduleTask([provider = providerEntry]() { provider->Setup(); });
        }
        auto DidCloseTextDocument(lsp::DidCloseTextDocumentParams params) -> void
        {
            providerLookup.erase(params.textDocument.uri);
        }

#pragma endregion

#pragma region Language Features

        auto DocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void
        {
            auto uri = params.textDocument.uri;
            ScheduleLanguageQuery(uri, [this, requestId](const LanguageQueryProvider& provider) {
                auto result = ComputeDocumentSymbol(provider);
                server->HandleServerResponse(requestId, result, false);
            });
        }

        auto SemanticTokensFull(int requestId, lsp::SemanticTokensParam params) -> void
        {
            auto uri = params.textDocument.uri;
            ScheduleLanguageQuery(uri, [this, requestId](const LanguageQueryProvider& provider) {
                lsp::SemanticTokens result = ComputeSemanticTokens(provider);
                server->HandleServerResponse(requestId, result, false);
            });
        }

        auto Completion(int requestId, lsp::CompletionParams params) -> void
        {
            auto uri = params.baseParams.textDocument.uri;
            ScheduleLanguageQuery(
                uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                    std::vector<lsp::CompletionItem> result = ComputeCompletion(provider, params.baseParams.position);
                    server->HandleServerResponse(requestId, result, false);
                });
        }

        auto SignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void
        {
            auto uri = params.baseParams.textDocument.uri;
            ScheduleLanguageQuery(uri, [this, requestId,
                                        params = std::move(params)](const LanguageQueryProvider& provider) {
                std::optional<lsp::SignatureHelp> result = ComputeSignatureHelp(provider, params.baseParams.position);
                server->HandleServerResponse(requestId, result, false);
            });
        }

        auto Hover(int requestId, lsp::HoverParams params) -> void
        {
            auto uri = params.baseParams.textDocument.uri;
            ScheduleLanguageQuery(
                uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                    std::optional<lsp::Hover> result = ComputeHover(provider, params.baseParams.position);
                    server->HandleServerResponse(requestId, result, false);
                });
        }

        auto Declaration(int requestId, lsp::DeclarationParams params) -> void
        {
            auto uri = params.baseParams.textDocument.uri;
            ScheduleLanguageQuery(
                uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                    std::vector<lsp::Location> result =
                        ComputeDeclaration(provider, params.baseParams.textDocument.uri, params.baseParams.position);
                    server->HandleServerResponse(requestId, result, false);
                });
        }

        auto Definition(int requestId, lsp::DefinitionParams params) -> void
        {
            auto uri = params.baseParams.textDocument.uri;
            ScheduleLanguageQuery(
                uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                    std::vector<lsp::Location> result =
                        ComputeDeclaration(provider, params.baseParams.textDocument.uri, params.baseParams.position);
                    server->HandleServerResponse(requestId, result, false);
                });
        }

        auto References(int requestId, lsp::ReferenceParams params) -> void
        {
            auto uri = params.baseParams.textDocument.uri;
            ScheduleLanguageQuery(
                uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                    std::vector<lsp::Location> result =
                        ComputeReferences(provider, params.baseParams.textDocument.uri, params.baseParams.position,
                                          params.context.includeDeclaration);
                    server->HandleServerResponse(requestId, result, false);
                });
        }

        auto InlayHint(int requestId, lsp::InlayHintParams params) -> void
        {
            auto uri = params.textDocument.uri;
            ScheduleLanguageQuery(uri,
                                  [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                                      std::vector<lsp::InlayHint> result = ComputeInlayHint(provider, params.range);
                                      server->HandleServerResponse(requestId, result, false);
                                  });
        }

        auto DocumentColor(int requestId, lsp::DocumentColorParams params) -> void
        {
            auto uri = params.textDocument.uri;
            ScheduleLanguageQuery(uri,
                                  [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                                      std::vector<lsp::ColorInformation> result = ComputeDocumentColor(provider);
                                      server->HandleServerResponse(requestId, result, false);
                                  });
        }

#pragma endregion

#pragma region Window Features

        auto ShowMessage(lsp::ShowMessageParams params) -> void
        {
            server->HandleServerNotification(lsp::LSPMethod_ShowMessage, params);
        }

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
            threadPool.push_task(std::forward<F>(f), std::forward<Args>(args)...);
        }

        // uri -> provider
        std::map<std::string, std::shared_ptr<PendingIntellisenseProvider>> providerLookup;

        LanguageServerCallback* server;

        BS::thread_pool threadPool;
    };
} // namespace glsld
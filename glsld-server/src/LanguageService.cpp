#include "LanguageService.h"
#include "DocumentSymbol.h"
#include "LanguageQueryProvider.h"
#include "SemanticTokens.h"
#include "CodeCompletion.h"
#include "Hover.h"
#include "Declaration.h"
#include "InlayHints.h"
#include "SourceText.h"

namespace glsld
{
    auto ComputeSignatureHelp(const LanguageQueryProvider& provider, lsp::Position position)
        -> std::optional<lsp::SignatureHelp>;

    auto ComputeReferences(const LanguageQueryProvider& provider, const lsp::DocumentUri& uri, lsp::Position position,
                           bool includeDeclaration) -> std::vector<lsp::Location>;

    auto LanguageService::Initialize(int requestId, lsp::InitializeParams params) -> void
    {
        auto completionTriggerCharacters    = std::vector<std::string>{"."};
        auto signatureHelpTriggerCharacters = std::vector<std::string>{"("};

        auto result = lsp::InitializedResult{
            .capabilities =
                {
                    .positionEncoding = "utf-16",
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
                    // .colorProvider  = false,
                    // .renameProvider = std::nullopt,
                },
            .serverInfo =
                {
                    .name    = "glsld",
                    .version = "0.0.2",
                },
        };
        server.HandleServerResponse(requestId, result, false);
    }

#pragma region Document Synchronization

    auto LanguageService::DidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void
    {
        auto& providerEntry = providerLookup[params.textDocument.uri];
        if (providerEntry) {
            // Bad request
            return;
        }

        providerEntry = std::make_shared<PendingBackgroundCompilation>(
            params.textDocument.version, UnescapeHttp(params.textDocument.uri), std::move(params.textDocument.text));

        ScheduleTask([provider = providerEntry]() { provider->Setup(); });
    }
    auto LanguageService::DidChangeTextDocument(lsp::DidChangeTextDocumentParams params) -> void
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
        providerEntry = std::make_shared<PendingBackgroundCompilation>(
            params.textDocument.version, UnescapeHttp(params.textDocument.uri), std::move(sourceBuffer));

        ScheduleTask([provider = providerEntry]() { provider->Setup(); });
    }
    auto LanguageService::DidCloseTextDocument(lsp::DidCloseTextDocumentParams params) -> void
    {
        providerLookup.erase(params.textDocument.uri);
    }

#pragma endregion

#pragma region Language Features

    auto LanguageService::DocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void
    {
        auto uri = params.textDocument.uri;
        ScheduleLanguageQuery(uri, [this, requestId](const LanguageQueryProvider& provider) {
            auto result = ComputeDocumentSymbol(provider);
            server.HandleServerResponse(requestId, result, false);
        });
    }

    auto LanguageService::SemanticTokensFull(int requestId, lsp::SemanticTokensParam params) -> void
    {
        auto uri = params.textDocument.uri;
        ScheduleLanguageQuery(uri, [this, requestId](const LanguageQueryProvider& provider) {
            lsp::SemanticTokens result = ComputeSemanticTokens(provider);
            server.HandleServerResponse(requestId, result, false);
        });
    }

    auto LanguageService::Completion(int requestId, lsp::CompletionParams params) -> void
    {
        auto uri = params.baseParams.textDocument.uri;
        ScheduleLanguageQuery(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                std::vector<lsp::CompletionItem> result = ComputeCompletion(provider, params.baseParams.position);
                server.HandleServerResponse(requestId, result, false);
            });
    }

    auto LanguageService::SignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void
    {
        auto uri = params.baseParams.textDocument.uri;
        ScheduleLanguageQuery(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                std::optional<lsp::SignatureHelp> result = ComputeSignatureHelp(provider, params.baseParams.position);
                server.HandleServerResponse(requestId, result, false);
            });
    }

    auto LanguageService::Hover(int requestId, lsp::HoverParams params) -> void
    {
        auto uri = params.baseParams.textDocument.uri;
        ScheduleLanguageQuery(uri,
                              [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                                  std::optional<lsp::Hover> result = ComputeHover(provider, params.baseParams.position);
                                  server.HandleServerResponse(requestId, result, false);
                              });
    }

    auto LanguageService::Declaration(int requestId, lsp::DeclarationParams params) -> void
    {
        auto uri = params.baseParams.textDocument.uri;
        ScheduleLanguageQuery(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                std::vector<lsp::Location> result =
                    ComputeDeclaration(provider, params.baseParams.textDocument.uri, params.baseParams.position);
                server.HandleServerResponse(requestId, result, false);
            });
    }

    auto LanguageService::Definition(int requestId, lsp::DefinitionParams params) -> void
    {
        // FIXME: compute definition properly
        auto uri = params.baseParams.textDocument.uri;
        ScheduleLanguageQuery(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                std::vector<lsp::Location> result =
                    ComputeDeclaration(provider, params.baseParams.textDocument.uri, params.baseParams.position);
                server.HandleServerResponse(requestId, result, false);
            });
    }

    auto LanguageService::References(int requestId, lsp::ReferenceParams params) -> void
    {
        auto uri = params.baseParams.textDocument.uri;
        ScheduleLanguageQuery(uri,
                              [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                                  std::vector<lsp::Location> result =
                                      ComputeReferences(provider, params.baseParams.textDocument.uri,
                                                        params.baseParams.position, params.context.includeDeclaration);
                                  server.HandleServerResponse(requestId, result, false);
                              });
    }

    auto LanguageService::InlayHint(int requestId, lsp::InlayHintParams params) -> void
    {
        auto uri = params.textDocument.uri;
        ScheduleLanguageQuery(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryProvider& provider) {
                std::vector<lsp::InlayHint> result = ComputeInlayHint(provider, config.inlayHint, params.range);
                server.HandleServerResponse(requestId, result, false);
            });
    }

#pragma endregion

#pragma region Window Features

    auto LanguageService::ShowMessage(lsp::ShowMessageParams params) -> void
    {
        server.HandleServerNotification(lsp::LSPMethod_ShowMessage, params);
    }

#pragma endregion

} // namespace glsld

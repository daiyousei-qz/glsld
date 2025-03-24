#include "Server/LanguageService.h"
#include "Feature/Completion.h"
#include "Feature/Declaration.h"
#include "Feature/DocumentSymbol.h"
#include "Feature/FoldingRange.h"
#include "Feature/Hover.h"
#include "Feature/InlayHint.h"
#include "Feature/Reference.h"
#include "Feature/SemanticTokens.h"
#include "Feature/SignatureHelp.h"
#include "Support/SourceText.h"

namespace glsld
{
    auto LanguageService::ScheduleLanguageQuery(const std::string& uri,
                                                std::function<auto(const LanguageQueryInfo&)->void> callback) -> void
    {
        // NOTE we need to make a copy of the provider here so it doesn't get released while in analysis
        auto provider = providerLookup[uri];
        if (provider != nullptr) {
            server.ScheduleBackgroundTask([this, provider = std::move(provider), callback = std::move(callback)] {
                if (provider->WaitAvailable()) {
                    callback(provider->GetProvider());
                }
            });
        }
    }

    auto LanguageService::Initialize(int requestId, lsp::InitializeParams params) -> void
    {
        auto completionTriggerCharacters    = std::vector<std::string>{"."};
        auto signatureHelpTriggerCharacters = std::vector<std::string>{"("};

        auto result = lsp::InitializeResult{
            .capabilities =
                {
                    .textDocumentSync =
                        {
                            .openClose = true,
                            .change    = lsp::TextDocumentSyncKind::Incremental,
                        },
                    .completionProvider    = GetCompletionOptions(server.GetConfig().languageService.completion),
                    .hoverProvider         = GetHoverOptions(server.GetConfig().languageService.hover),
                    .signatureHelpProvider = GetSignatureHelpOptions(server.GetConfig().languageService.signatureHelp),
                    .declarationProvider   = GetDeclarationOptions(server.GetConfig().languageService.declaration),
                    // .definitionProvider     = GetDefinitionOptions(),
                    .referencesProvider = GetReferenceOptions(server.GetConfig().languageService.reference),
                    .documentSymbolProvider =
                        GetDocumentSymbolOptions(server.GetConfig().languageService.documentSymbol),
                    .semanticTokensProvider =
                        GetSemanticTokensOptions(server.GetConfig().languageService.semanticTokens),
                    .inlayHintProvider = GetInlayHintsOptions(server.GetConfig().languageService.inlayHint),
                    // .renameProvider = std::nullopt,
                },
            .serverInfo =
                {
                    .name = "glsld",
                },
        };
        server.HandleServerResponse(requestId, result, false);
        server.LogInfo("GLSLD initialized");
    }

#pragma region Document Synchronization

    auto LanguageService::DidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void
    {
        auto& providerEntry = providerLookup[params.textDocument.uri];
        if (providerEntry) {
            // Bad request
            return;
        }

        providerEntry = std::make_shared<BackgroundCompilation>(
            params.textDocument.version, UnescapeHttp(params.textDocument.uri), std::move(params.textDocument.text));

        server.ScheduleBackgroundTask([provider = providerEntry]() { provider->Setup(); });
        server.LogInfo("Opened document: {}", params.textDocument.uri);
        server.LogDebug("Document updated: {}\n{}", params.textDocument.uri, providerEntry->GetBuffer());
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
        providerEntry = std::make_shared<BackgroundCompilation>(
            params.textDocument.version, UnescapeHttp(params.textDocument.uri), std::move(sourceBuffer));

        server.ScheduleBackgroundTask([provider = providerEntry]() { provider->Setup(); });
        server.LogInfo("Edited document: {}", params.textDocument.uri);
        server.LogDebug("Document updated: {}\n{}", params.textDocument.uri, providerEntry->GetBuffer());
    }
    auto LanguageService::DidCloseTextDocument(lsp::DidCloseTextDocumentParams params) -> void
    {
        providerLookup.erase(params.textDocument.uri);
        server.LogInfo("Closed document: {}", params.textDocument.uri);
    }

#pragma endregion

#pragma region Language Features

    auto LanguageService::DocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "documentSymbol", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo& info) {
            auto result = HandleDocumentSymbol(server.GetConfig().languageService.documentSymbol, info, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}", requestId, "documentSymbol");
        });
    }

    auto LanguageService::SemanticTokensFull(int requestId, lsp::SemanticTokensParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "semanticTokensFull", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo& info) {
            lsp::SemanticTokens result =
                HandleSemanticTokens(server.GetConfig().languageService.semanticTokens, info, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}", requestId, "semanticTokensFull");
        });
    }

    auto LanguageService::Completion(int requestId, lsp::CompletionParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "completion", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo& info) {
            std::vector<lsp::CompletionItem> result =
                HandleCompletion(server.GetConfig().languageService.completion, info, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}", requestId, "completion");
        });
    }

    auto LanguageService::SignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "signatureHelp", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo& info) {
            std::optional<lsp::SignatureHelp> result =
                HandleSignatureHelp(server.GetConfig().languageService.signatureHelp, info, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}", requestId, "signatureHelp");
        });
    }

    auto LanguageService::Hover(int requestId, lsp::HoverParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "hover", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo& info) {
            std::optional<lsp::Hover> result = HandleHover(server.GetConfig().languageService.hover, info, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}", requestId, "hover");
        });
    }

    auto LanguageService::Declaration(int requestId, lsp::DeclarationParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "declaration", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo& info) {
            std::vector<lsp::Location> result =
                HandleDeclaration(server.GetConfig().languageService.declaration, info, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}", requestId, "declaration");
        });
    }

    auto LanguageService::References(int requestId, lsp::ReferenceParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "references", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo& info) {
            std::vector<lsp::Location> result =
                HandleReferences(server.GetConfig().languageService.reference, info, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}", requestId, "references");
        });
    }

    auto LanguageService::InlayHint(int requestId, lsp::InlayHintParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "inlayHint", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo& info) {
            std::vector<lsp::InlayHint> result =
                HandleInlayHints(server.GetConfig().languageService.inlayHint, info, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}", requestId, "inlayHint");
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

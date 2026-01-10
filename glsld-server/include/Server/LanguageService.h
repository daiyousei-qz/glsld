#pragma once
#include "Language/ShaderTarget.h"
#include "Server/BackgroundCompilation.h"
#include "Server/Protocol.h"
#include "Server/LanguageServer.h"
#include "Server/LanguageQueryInfo.h"
#include "Support/SimpleTimer.h"
#include "Support/StringView.h"
#include <memory>
#include <thread>

#include <cstddef>

namespace glsld
{
    class LanguageService
    {
    private:
        LanguageServer& server;

        // uri -> provider
        std::map<std::string, std::shared_ptr<BackgroundCompilation>> providerLookup;

        // FIXME: we should allow a preamble to expire if no one is using it
        std::unordered_map<LanguageConfig, std::shared_ptr<LanguagePreambleInfo>> preambleInfoCache;

        struct PendingDiagnostic
        {
            SimpleTimer triggerTimer;
            std::shared_ptr<BackgroundCompilation> backgroundCompilation;
        };

        // This queue tracks diagnostics that are to be computed in the background.
        // An edit of a document will enqueue a diagnostic request. It will be processed
        // after a delay if no further edits happen to the document, or discarded otherwise.
        std::deque<PendingDiagnostic> pendingDiagnostics;
        std::mutex pendingDiagnosticsMutex;
        std::jthread diagnosticConsumerThread;

        // Schedule a background compilation for the given BackgroundCompilation instance.
        auto ScheduleBackgroundCompilation(std::shared_ptr<BackgroundCompilation> backgroundCompilation) -> void;

        // Schedule a background diagnostic for the given BackgroundCompilation instance.
        auto ScheduleBackgroundDiagnostic(std::shared_ptr<BackgroundCompilation> backgroundCompilation) -> void;

        // Starts a thread to consume pending diagnostics. It should keep running until the LanguageServer is shut down.
        auto SetupDiagnosticConsumerThread() -> void;

        // Schedule a language query for the given uri in a background thread, which waits for the compilation and then
        // runs the callback.
        // TODO: could we use std::move_only_function here?
        auto ScheduleLanguageQuery(
            const std::string& uri,
            std::function<auto(const LanguagePreambleInfo&, const LanguageQueryInfo&)->void> callback) -> void;

        auto InferShaderStageFromUri(StringView uri) -> GlslShaderStage;

        auto GetLanguageQueryPreambleInfo(const LanguageConfig& config) -> std::shared_ptr<LanguagePreambleInfo>;

    public:
        LanguageService(LanguageServer& server) : server(server)
        {
        }

        auto Initialize() -> void;

        auto Finalize() -> void;

#pragma region Lifecycle

        auto OnInitialize(int requestId, lsp::InitializeParams params) -> void;

        auto OnInitialized(lsp::InitializedParams) -> void;

        auto OnSetTrace(lsp::SetTraceParams params) -> void;

        auto OnShutdown(int requestId, std::nullptr_t) -> void;

        auto OnExit(std::nullptr_t) -> void;

#pragma endregion

#pragma region Document Synchronization

        auto OnDidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void;

        auto OnDidChangeTextDocument(lsp::DidChangeTextDocumentParams params) -> void;

        auto OnDidCloseTextDocument(lsp::DidCloseTextDocumentParams params) -> void;

#pragma endregion

#pragma region Language Features

        auto OnDocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void;

        auto OnSemanticTokensFull(int requestId, lsp::SemanticTokensParams params) -> void;

        auto OnCompletion(int requestId, lsp::CompletionParams params) -> void;

        auto OnSignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void;

        auto OnHover(int requestId, lsp::HoverParams params) -> void;

        auto OnDefinition(int requestId, lsp::DefinitionParams params) -> void;

        auto OnReferences(int requestId, lsp::ReferenceParams params) -> void;

        auto OnInlayHint(int requestId, lsp::InlayHintParams params) -> void;

        auto OnFoldingRange(int requestId, lsp::FoldingRangeParams params) -> void;

#pragma endregion

#pragma region Window Features

        auto OnShowMessage(lsp::ShowMessageParams params) -> void;

#pragma endregion
    };
} // namespace glsld
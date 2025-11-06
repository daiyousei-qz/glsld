#pragma once
#include "Language/ShaderTarget.h"
#include "Server/BackgroundCompilation.h"
#include "Server/Protocol.h"
#include "Server/LanguageServer.h"
#include "Server/LanguageQueryInfo.h"
#include "Support/StringView.h"

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

        // Schedule a language query for the given uri in a background thread, which waits for the compilation and then
        // runs the callback.
        auto ScheduleLanguageQuery(
            const std::string& uri,
            std::function<auto(const LanguagePreambleInfo&, const LanguageQueryInfo&)->void> callback) -> void;

        auto InferShaderStageFromUri(StringView uri) -> GlslShaderStage;

        auto GetLanguageQueryPreambleInfo(const LanguageConfig& config) -> std::shared_ptr<LanguagePreambleInfo>;

    public:
        LanguageService(LanguageServer& server) : server(server)
        {
        }

#pragma region Lifecycle

        auto Initialize(int requestId, lsp::InitializeParams params) -> void;

        auto Initialized(lsp::InitializedParams) -> void;

        auto SetTrace(lsp::SetTraceParams params) -> void;

        auto Shutdown(int requestId, std::nullptr_t) -> void;

        auto Exit(std::nullptr_t) -> void;

#pragma endregion

#pragma region Document Synchronization

        auto DidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void;

        auto DidChangeTextDocument(lsp::DidChangeTextDocumentParams params) -> void;

        auto DidCloseTextDocument(lsp::DidCloseTextDocumentParams params) -> void;

#pragma endregion

#pragma region Language Features

        auto DocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void;

        auto SemanticTokensFull(int requestId, lsp::SemanticTokensParams params) -> void;

        auto Completion(int requestId, lsp::CompletionParams params) -> void;

        auto SignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void;

        auto Hover(int requestId, lsp::HoverParams params) -> void;

        auto Definition(int requestId, lsp::DefinitionParams params) -> void;

        auto References(int requestId, lsp::ReferenceParams params) -> void;

        auto InlayHint(int requestId, lsp::InlayHintParams params) -> void;

#pragma endregion

#pragma region Window Features

        auto ShowMessage(lsp::ShowMessageParams params) -> void;

#pragma endregion
    };
} // namespace glsld
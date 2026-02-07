#pragma once
#include "Language/ShaderTarget.h"
#include "Server/BackgroundCompilation.h"
#include "Server/Protocol.h"
#include "Server/LanguageServer.h"
#include "Server/LanguageQueryInfo.h"
#include "Support/AsyncMutex.h"
#include "Support/StringView.h"

#include <exec/async_scope.hpp>
#include <exec/static_thread_pool.hpp>
#include <exec/timed_thread_scheduler.hpp>

#include <memory>
#include <cstddef>

namespace glsld
{
    class LanguageService
    {
    private:
        LanguageServer& server;

        bool enableGlsldExtensions = false;

        exec::timed_thread_context timedSchedulerCtx{};
        exec::static_thread_pool backgroundWorkerCtx{};

        struct FeatureStateObject
        {
            // Mutex to protect the state object. If a language feature requires mutable state, it should lock this
            // mutex before accessing the state. That is, only one async task can access the state at a time.
            AsyncMutex mutex;

            // Type-erased state value.
            std::any state;
        };

        class TextDocumentContext
        {
        private:
            // All async tasks related to this document should be spawned in this scope.
            // We have to keep this context alive until all tasks in the scope are finished.
            exec::async_scope scope;

            // The latest background compilation for this document.
            std::shared_ptr<BackgroundCompilation> backgroundCompilation = nullptr;

            // This map holds the state objects for different language features.
            std::map<void*, FeatureStateObject> stateLookup = {};

            template <typename T>
            static auto GetTypeId() noexcept -> void*
            {
                static std::byte dummy;
                return static_cast<void*>(&dummy);
            }

            auto InferShaderStageFromUri(StringView uri) -> GlslShaderStage;

            auto InitializeTextDocument(const lsp::DidOpenTextDocumentParams& params) -> void;

        public:
            TextDocumentContext(const lsp::DidOpenTextDocumentParams& params)
            {
                InitializeTextDocument(params);
            }

            auto GetAsyncScope() -> exec::async_scope&
            {
                return scope;
            }

            auto GetBackgroundCompilation() -> const std::shared_ptr<BackgroundCompilation>&
            {
                return backgroundCompilation;
            }

            auto UpdateTextDocument(const lsp::DidChangeTextDocumentParams& params) -> void;

            template <typename StateType>
            auto GetLanguageFeatureState() -> FeatureStateObject&
            {
                FeatureStateObject& stateObject = stateLookup[GetTypeId<std::remove_cvref_t<StateType>>()];
                if (!stateObject.state.has_value()) {
                    // FIXME: How to handle custom initialization?
                    stateObject.state = StateType{};
                }

                return stateObject;
            }
        };

        // uri -> document context
        StringMap<std::unique_ptr<TextDocumentContext>> documentContexts;

        // Schedule a background compilation for the given TextDocumentContext instance.
        auto ScheduleBackgroundCompilation(TextDocumentContext& ctx) -> void;

        // Schedule a background diagnostic for the given TextDocumentContext instance.
        auto ScheduleBackgroundDiagnostic(TextDocumentContext& ctx) -> void;

        // Schedule a background destruction for the given TextDocumentContext instance.
        auto ScheduleBackgroundClosingDocument(std::unique_ptr<TextDocumentContext> ctx) -> void;

        // Schedule a language query for the given uri in a background thread, which waits for the compilation and then
        // runs the callback.
        template <typename StateType>
        auto ScheduleLanguageQuery(StringView uri,
                                   std::move_only_function<auto(const LanguageQueryInfo&, StateType&)->void> callback)
            -> void
        {
            auto& ctx = documentContexts[uri];
            if (!ctx) {
                // Bad request. Document is not open.
                server.LogInfo("Received language query for a document that is not open: {}", uri);
                return;
            }

            // NOTE we need to make a copy of the provider here so it doesn't get released while in analysis
            GLSLD_ASSERT(ctx->GetBackgroundCompilation() &&
                         "Background compilation must be present in the document context");

            FeatureStateObject& stateObject = ctx->GetLanguageFeatureState<StateType>();
            ctx->GetAsyncScope().spawn(
                // Wait for the background compilation to finish
                ctx->GetBackgroundCompilation()->AsyncWaitAvailable()
                // Acquire the state lock if necessary
                | stdexec::let_value([&stateObject] {
                      if constexpr (std::is_const_v<StateType> || std::is_same_v<StateType, std::monostate>) {
                          // If the state is const or monostate, we don't need to lock it.
                          return stdexec::just(std::unique_lock<AsyncMutex>{stateObject.mutex, std::defer_lock});
                      }
                      else {
                          return stateObject.mutex.AsyncLetLock();
                      }
                  })
                // Continue on the background worker thread
                | stdexec::continues_on(backgroundWorkerCtx.get_scheduler())
                // Finally handle the query
                | stdexec::then([backgroundCompilation = ctx->GetBackgroundCompilation(), &stateObject,
                                 callback = std::move(callback)](std::unique_lock<AsyncMutex> /*lock*/) mutable {
                      callback(backgroundCompilation->GetLanguageQueryInfo(),
                               std::any_cast<StateType&>(stateObject.state));
                  }));
        }

    public:
        LanguageService(LanguageServer& server) : server(server)
        {
        }

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

        auto OnSemanticTokensDelta(int requestId, lsp::SemanticTokensDeltaParams params) -> void;

        auto OnCompletion(int requestId, lsp::CompletionParams params) -> void;

        auto OnSignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void;

        auto OnHover(int requestId, lsp::HoverParams params) -> void;

        auto OnDefinition(int requestId, lsp::DefinitionParams params) -> void;

        auto OnReferences(int requestId, lsp::ReferenceParams params) -> void;

        auto OnInlayHint(int requestId, lsp::InlayHintParams params) -> void;

        auto OnFoldingRange(int requestId, lsp::FoldingRangeParams params) -> void;

#pragma endregion
    };
} // namespace glsld
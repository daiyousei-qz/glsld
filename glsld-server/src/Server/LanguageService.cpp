#include "Compiler/CompilerConfig.h"
#include "Compiler/CompilerInvocation.h"
#include "Feature/Completion.h"
#include "Feature/Definition.h"
#include "Feature/Diagnostic.h"
#include "Feature/DocumentSymbol.h"
#include "Feature/FoldingRange.h"
#include "Feature/Hover.h"
#include "Feature/InlayHint.h"
#include "Feature/Reference.h"
#include "Feature/SemanticTokens.h"
#include "Feature/SignatureHelp.h"
#include "Server/LanguageService.h"
#include "Server/LanguageQueryInfo.h"
#include "Support/SimpleTimer.h"
#include "Support/SourceText.h"
#include "Support/Uri.h"

namespace glsld
{
    LanguagePreambleInfo::LanguagePreambleInfo()
    {
    }

    LanguagePreambleInfo::LanguagePreambleInfo(std::shared_ptr<PrecompiledPreamble> preamble)
        : preamble(std::move(preamble))
    {
        completionData    = ComputeCompletionPreambleInfo(*GetPreamble());
        signatureHelpData = ComputeSignatureHelpPreambleInfo(*GetPreamble());
    }

    LanguagePreambleInfo::~LanguagePreambleInfo()
    {
    }

    auto LanguageService::ScheduleBackgroundCompilation(std::shared_ptr<BackgroundCompilation> backgroundCompilation)
        -> void
    {
        server.ScheduleBackgroundTask(
            [&server = this->server, backgroundCompilation = std::move(backgroundCompilation)] {
                SimpleTimer timer;
                backgroundCompilation->Setup();
                server.LogInfo("Background compilation of ({} version {}) took {} ms", backgroundCompilation->GetUri(),
                               backgroundCompilation->GetVersion(), timer.GetElapsedMilliseconds());
            });
    }

    auto LanguageService::ScheduleBackgroundDiagnostic(std::shared_ptr<BackgroundCompilation> backgroundCompilation)
        -> void
    {
        std::lock_guard _{pendingDiagnosticsMutex};
        pendingDiagnostics.push_back(PendingDiagnostic{
            .triggerTimer          = SimpleTimer{},
            .backgroundCompilation = std::move(backgroundCompilation),
        });
    }

    auto LanguageService::SetupDiagnosticConsumerThread() -> void
    {
        diagnosticConsumerThread = std::jthread{[this]() {
            while (server.IsRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                std::lock_guard _{pendingDiagnosticsMutex};
                while (!pendingDiagnostics.empty()) {
                    const auto& pendingDiagnostic = pendingDiagnostics.front();
                    if (pendingDiagnostic.triggerTimer.GetElapsedMilliseconds() < 1000) {
                        break;
                    }

                    if (pendingDiagnostic.backgroundCompilation->TestExpired()) {
                        server.LogInfo("Diagnostic compilation of ({} version {}) is skipped because of expiration",
                                       pendingDiagnostic.backgroundCompilation->GetUri(),
                                       pendingDiagnostic.backgroundCompilation->GetVersion());
                    }
                    else {
                        server.ScheduleBackgroundTask(
                            [&server               = this->server,
                             backgroundCompilation = std::move(pendingDiagnostic.backgroundCompilation)] {
                                SimpleTimer timer;
                                auto diagnostics = HandleDiagnostic(
                                    server.GetConfig().languageService.diagnostic, backgroundCompilation->GetUri(),
                                    backgroundCompilation->GetVersion(), backgroundCompilation->GetNextLanguageConfig(),
                                    backgroundCompilation->GetBuffer());

                                if (!backgroundCompilation->TestExpired()) {
                                    server.HandleServerNotification(lsp::LSPMethod_PublishDiagnostic, diagnostics);
                                }
                                server.LogInfo("Diagnostic compilation of ({} version {}) took {} ms",
                                               backgroundCompilation->GetUri(), backgroundCompilation->GetVersion(),
                                               timer.GetElapsedMilliseconds());
                            });
                    }

                    pendingDiagnostics.pop_front();
                }
            }
        }};
    }

    auto LanguageService::ScheduleLanguageQuery(
        const std::string& uri,
        std::function<auto(const LanguagePreambleInfo&, const LanguageQueryInfo&)->void> callback) -> void
    {
        // NOTE we need to make a copy of the provider here so it doesn't get released while in analysis
        auto backgroundCompilation = providerLookup[uri];
        if (backgroundCompilation != nullptr) {
            server.ScheduleBackgroundTask([backgroundCompilation = std::move(backgroundCompilation),
                                           callback              = std::move(callback)] {
                if (backgroundCompilation->WaitAvailable()) {
                    callback(backgroundCompilation->GetPreambleInfo(), backgroundCompilation->GetLanguageQueryInfo());
                }
            });
        }
    }

    auto LanguageService::InferShaderStageFromUri(StringView uri) -> GlslShaderStage
    {
        constexpr auto cases = std::to_array<std::pair<StringView, GlslShaderStage>>({
            {".vert", GlslShaderStage::Vertex},
            {".vs", GlslShaderStage::Vertex},
            {".frag", GlslShaderStage::Fragment},
            {".fs", GlslShaderStage::Fragment},
            {".geom", GlslShaderStage::Geometry},
            {".gs", GlslShaderStage::Geometry},
            {".tesc", GlslShaderStage::TessControl},
            {".tese", GlslShaderStage::TessEvaluation},
            {".comp", GlslShaderStage::Compute},
            {".cs", GlslShaderStage::Compute},
            {".rgen", GlslShaderStage::RayGeneration},
            {".rint", GlslShaderStage::RayIntersection},
            {".rahit", GlslShaderStage::RayAnyHit},
            {".rchit", GlslShaderStage::RayClosestHit},
            {".rmiss", GlslShaderStage::RayMiss},
            {".rcall", GlslShaderStage::RayCallable},
            {".task", GlslShaderStage::Task},
            {".mesh", GlslShaderStage::Mesh},
        });

        for (const auto& [ext, stage] : cases) {
            if (uri.EndWith(ext)) {
                return stage;
            }
        }

        return GlslShaderStage::Unknown;
    }

    auto LanguageService::GetLanguageQueryPreambleInfo(const LanguageConfig& config)
        -> std::shared_ptr<LanguagePreambleInfo>
    {
        if (auto it = preambleInfoCache.find(config); it != preambleInfoCache.end()) {
            return it->second;
        }

        // FIXME: this compilation could happen in a background thread
        CompilerInvocation invocation;
        invocation.ApplyLanguageConfig(config);
        auto preambleInfo         = std::make_shared<LanguagePreambleInfo>(invocation.CompilePreamble(nullptr));
        preambleInfoCache[config] = preambleInfo;
        return preambleInfo;
    }

    auto LanguageService::Initialize() -> void
    {
        SetupDiagnosticConsumerThread();
    }

    auto LanguageService::Finalize() -> void
    {
    }

    auto LanguageService::OnInitialize(int requestId, lsp::InitializeParams params) -> void
    {
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
                    // .declarationProvider   = GetDeclarationOptions(server.GetConfig().languageService.declaration),
                    .definitionProvider = GetDefinitionOptions(server.GetConfig().languageService.definition),
                    .referencesProvider = GetReferenceOptions(server.GetConfig().languageService.reference),
                    .documentSymbolProvider =
                        GetDocumentSymbolOptions(server.GetConfig().languageService.documentSymbol),
                    .foldingRangeProvider = GetFoldingRangeOptions(server.GetConfig().languageService.foldingRange),
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

    auto LanguageService::OnInitialized(lsp::InitializedParams) -> void
    {
        // No-op for now
    }

    auto LanguageService::OnSetTrace(lsp::SetTraceParams params) -> void
    {
        // No-op for now
    }

    auto LanguageService::OnShutdown(int requestId, std::nullptr_t) -> void
    {
        // We do nothing on shutdown request but an acknowledgement
        server.HandleServerResponse(requestId, nullptr, false);
        server.LogInfo("GLSLD shutting down");
    }

    auto LanguageService::OnExit(std::nullptr_t) -> void
    {
        server.Shutdown();
    }

#pragma region Document Synchronization

    auto LanguageService::OnDidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void
    {
        auto& backgroundCompilation = providerLookup[params.textDocument.uri];
        if (backgroundCompilation) {
            // Bad request. Document is already open.
            return;
        }

        backgroundCompilation = std::make_shared<BackgroundCompilation>(
            params.textDocument.version, UnescapeHttp(params.textDocument.uri), std::move(params.textDocument.text),
            GetLanguageQueryPreambleInfo({.stage = InferShaderStageFromUri(params.textDocument.uri)}));

        ScheduleBackgroundCompilation(backgroundCompilation);
        ScheduleBackgroundDiagnostic(backgroundCompilation);

        server.LogInfo("Opened document: {}. New version is {}", params.textDocument.uri, params.textDocument.version);
        server.LogDebug("Document updated: {}\n{}", params.textDocument.uri, backgroundCompilation->GetBuffer());
    }
    auto LanguageService::OnDidChangeTextDocument(lsp::DidChangeTextDocumentParams params) -> void
    {
        auto& backgroundCompilation = providerLookup[params.textDocument.uri];
        if (!backgroundCompilation) {
            // Bad request. Document is not open.
            return;
        }

        // Since the text document is changed, we mark the previous compilation as expired.
        backgroundCompilation->MarkExpired();

        // TODO: Could have a buffer manager so we don't keep allocating new buffers if user types faster than
        // compilation
        // TODO: Research if add a line-offset hints vector speedup the editing
        auto sourceBuffer = backgroundCompilation->GetBuffer().Str();
        for (const auto& change : params.contentChanges) {
            if (change.range) {
                ApplySourceChange(sourceBuffer, FromLspRange(*change.range), StringView{change.text});
            }
            else {
                sourceBuffer = std::move(change.text);
            }
        }
        backgroundCompilation = std::make_shared<BackgroundCompilation>(
            params.textDocument.version, UnescapeHttp(params.textDocument.uri), std::move(sourceBuffer),
            GetLanguageQueryPreambleInfo(backgroundCompilation->GetNextLanguageConfig()));

        ScheduleBackgroundCompilation(backgroundCompilation);
        ScheduleBackgroundDiagnostic(backgroundCompilation);

        server.LogInfo("Edited document: {}. New version is {}", params.textDocument.uri, params.textDocument.version);
        server.LogDebug("Document updated: {}\n{}", params.textDocument.uri, backgroundCompilation->GetBuffer());
    }
    auto LanguageService::OnDidCloseTextDocument(lsp::DidCloseTextDocumentParams params) -> void
    {
        providerLookup.erase(params.textDocument.uri);
        server.LogInfo("Closed document: {}", params.textDocument.uri);
    }

#pragma endregion

#pragma region Language Features

    auto LanguageService::OnDocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "documentSymbol", uri);
        ScheduleLanguageQuery(uri, [&server = this->server, requestId, params = std::move(params)](
                                       const LanguagePreambleInfo& preambleInfo, const LanguageQueryInfo& queryInfo) {
            SimpleTimer timer;
            auto result = HandleDocumentSymbol(server.GetConfig().languageService.documentSymbol, queryInfo, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "documentSymbol",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnSemanticTokensFull(int requestId, lsp::SemanticTokensParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "semanticTokensFull", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](
                                       const LanguagePreambleInfo& preambleInfo, const LanguageQueryInfo& queryInfo) {
            SimpleTimer timer;
            lsp::SemanticTokens result =
                HandleSemanticTokens(server.GetConfig().languageService.semanticTokens, queryInfo, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "semanticTokensFull",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnCompletion(int requestId, lsp::CompletionParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "completion", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](
                                       const LanguagePreambleInfo& preambleInfo, const LanguageQueryInfo& queryInfo) {
            SimpleTimer timer;
            std::vector<lsp::CompletionItem> result = HandleCompletion(
                server.GetConfig().languageService.completion, preambleInfo.GetCompletionInfo(), queryInfo, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "completion",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnSignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "signatureHelp", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](
                                       const LanguagePreambleInfo& preambleInfo, const LanguageQueryInfo& queryInfo) {
            SimpleTimer timer;
            std::optional<lsp::SignatureHelp> result =
                HandleSignatureHelp(server.GetConfig().languageService.signatureHelp,
                                    preambleInfo.GetSignatureHelpInfo(), queryInfo, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "signatureHelp",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnHover(int requestId, lsp::HoverParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "hover", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](
                                       const LanguagePreambleInfo& preambleInfo, const LanguageQueryInfo& queryInfo) {
            SimpleTimer timer;
            std::optional<lsp::Hover> result = HandleHover(server.GetConfig().languageService.hover, queryInfo, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "hover",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnDefinition(int requestId, lsp::DefinitionParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "definition", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](
                                       const LanguagePreambleInfo& preambleInfo, const LanguageQueryInfo& queryInfo) {
            SimpleTimer timer;
            std::vector<lsp::Location> result =
                HandleDefinition(server.GetConfig().languageService.definition, queryInfo, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "definition",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnReferences(int requestId, lsp::ReferenceParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "references", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](
                                       const LanguagePreambleInfo& preambleInfo, const LanguageQueryInfo& queryInfo) {
            SimpleTimer timer;
            std::vector<lsp::Location> result =
                HandleReferences(server.GetConfig().languageService.reference, queryInfo, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "references",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnInlayHint(int requestId, lsp::InlayHintParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "inlayHint", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](
                                       const LanguagePreambleInfo& preambleInfo, const LanguageQueryInfo& queryInfo) {
            SimpleTimer timer;
            std::vector<lsp::InlayHint> result =
                HandleInlayHints(server.GetConfig().languageService.inlayHint, queryInfo, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "inlayHint",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnFoldingRange(int requestId, lsp::FoldingRangeParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "foldingRange", uri);
        ScheduleLanguageQuery(uri, [this, requestId, params = std::move(params)](
                                       const LanguagePreambleInfo& preambleInfo, const LanguageQueryInfo& queryInfo) {
            SimpleTimer timer;
            std::vector<lsp::FoldingRange> result =
                HandleFoldingRange(server.GetConfig().languageService.foldingRange, queryInfo, params);
            server.HandleServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "foldingRange",
                           timer.GetElapsedMilliseconds());
        });
    }

#pragma endregion

#pragma region Window Features

    auto LanguageService::OnShowMessage(lsp::ShowMessageParams params) -> void
    {
        server.HandleServerNotification(lsp::LSPMethod_ShowMessage, params);
    }

#pragma endregion
} // namespace glsld

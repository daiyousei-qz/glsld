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

namespace glsld
{
    auto LanguageService::TextDocumentContext::InferShaderStageFromUri(StringView uri) -> GlslShaderStage
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

    auto LanguageService::ScheduleBackgroundCompilation(TextDocumentContext& ctx) -> void
    {
        ctx.GetAsyncScope().spawn(
            // Switch to background worker thread
            stdexec::schedule(backgroundWorkerCtx.get_scheduler()) |
            // Do the background compilation work
            stdexec::then([&server = server, backgroundCompilation = ctx.GetBackgroundCompilation()]() {
                SimpleTimer timer;
                backgroundCompilation->Run();
                server.LogInfo("Background compilation of ({} version {}) took {} ms", backgroundCompilation->GetUri(),
                               backgroundCompilation->GetVersion(), timer.GetElapsedMilliseconds());
            }));
    }

    auto LanguageService::ScheduleBackgroundDiagnostic(TextDocumentContext& ctx) -> void
    {
        if (ctx.GetBackgroundCompilation()->GetLanguageConfig().stage == GlslShaderStage::Unknown) {
            return;
        }

        ctx.GetAsyncScope().spawn(
            // Wait for long enough to debounce rapid changes
            exec::schedule_after(timedSchedulerCtx.get_scheduler(), std::chrono::seconds(1)) |
            // Switch to background worker thread
            stdexec::continues_on(backgroundWorkerCtx.get_scheduler()) |
            // Do the background diagnostic work
            stdexec::then([&server = server, backgroundCompilation = ctx.GetBackgroundCompilation()] {
                if (backgroundCompilation->IsExpired()) {
                    server.LogInfo("Diagnostic compilation of ({} version {}) is skipped because of expiration",
                                   backgroundCompilation->GetUri(), backgroundCompilation->GetVersion());
                }
                else {
                    SimpleTimer timer;
                    auto diagnostics = HandleDiagnostic(
                        server.GetConfig().languageService.diagnostic, backgroundCompilation->GetUri(),
                        backgroundCompilation->GetVersion(), backgroundCompilation->GetNextLanguageConfig(),
                        backgroundCompilation->GetBuffer());

                    if (!backgroundCompilation->IsExpired()) {
                        // Computing diagnostics may consume some time, so we check expiration again before sending
                        server.SendServerNotification(lsp::LSPMethod_PublishDiagnostic, diagnostics);
                    }
                    else {
                        server.LogInfo("Diagnostic compilation of ({} version {}) is discarded",
                                       backgroundCompilation->GetUri(), backgroundCompilation->GetVersion());
                    }
                    server.LogInfo("Diagnostic compilation of ({} version {}) took {} ms",
                                   backgroundCompilation->GetUri(), backgroundCompilation->GetVersion(),
                                   timer.GetElapsedMilliseconds());
                }
            }));
    }

    auto LanguageService::ScheduleBackgroundClosingDocument(std::unique_ptr<TextDocumentContext> ctx) -> void
    {
        // FIXME: stop operations that are still running on this context
        stdexec::start_detached(
            stdexec::starts_on(backgroundWorkerCtx.get_scheduler(), ctx->GetAsyncScope().on_empty()) |
            stdexec::then([ctx = std::move(ctx)]() {
                // Do nothing. Just wait for async scope to be empty before destroying the context.
            }));
    }

    auto LanguageService::PublishInactiveRegions(StringView uri, const LanguageQueryInfo& queryInfo) -> void
    {
        if (enableGlsldExtensions) {
            lsp::PublishInactiveRegionParams inactiveRegions;
            inactiveRegions.uri = uri.Str();
            inactiveRegions.regions.reserve(queryInfo.GetPreprocessInfo().GetInactiveRegions().size() * 2);
            for (const auto& range : queryInfo.GetPreprocessInfo().GetInactiveRegions()) {
                inactiveRegions.regions.push_back(range.startLine);
                inactiveRegions.regions.push_back(range.endLine);
            }

            server.SendServerNotification(lsp::LSPMethod_PublishInactiveRegion, inactiveRegions);
            server.LogInfo("Published inactive regions for document {}", uri);
        }
    }

    auto LanguageService::OnInitialize(int requestId, lsp::InitializeParams params) -> void
    {
        if (params.initializationOptions && params.initializationOptions->supportGlsldExtensions &&
            *params.initializationOptions->supportGlsldExtensions) {
            enableGlsldExtensions = true;
        }

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
        server.SendServerResponse(requestId, result, false);
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
        server.SendServerResponse(requestId, nullptr, false);
        server.LogInfo("GLSLD shutting down");
    }

    auto LanguageService::OnExit(std::nullptr_t) -> void
    {
        server.Shutdown();
    }

#pragma region Document Synchronization

    auto LanguageService::OnDidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void
    {
        auto& ctx = documentContexts[params.textDocument.uri];
        if (ctx) {
            // Bad notification. Document is already open.
            server.LogInfo("Attempted to open a document that is already open: {}", params.textDocument.uri);
            return;
        }

        server.LogInfo("Opening document: {}. New version is {}", params.textDocument.uri, params.textDocument.version);
        ctx = std::make_unique<TextDocumentContext>(params.textDocument.uri, params.textDocument.version,
                                                    std::move(params.textDocument.text));
        ScheduleBackgroundCompilation(*ctx);
        ScheduleBackgroundDiagnostic(*ctx);

        server.LogDebug("Document updated: {}\n{}", params.textDocument.uri,
                        ctx->GetBackgroundCompilation()->GetBuffer());
    }

    auto LanguageService::OnDidChangeTextDocument(lsp::DidChangeTextDocumentParams params) -> void
    {
        auto& ctx = documentContexts[params.textDocument.uri];
        if (!ctx) {
            // Bad notification. Document is not open.
            return;
        }

        server.LogInfo("Editing document: {}. New version is {}", params.textDocument.uri, params.textDocument.version);
        ctx->UpdateTextDocument(
            params.textDocument.version,
            params.contentChanges | std::views::transform([](const lsp::TextDocumentContentChangeEvent& change) {
                return TextEdit{
                    .range   = change.range.transform(&FromLspRange),
                    .newText = change.text,
                };
            }) | std::ranges::to<std::vector>());
        ScheduleBackgroundCompilation(*ctx);
        ScheduleBackgroundDiagnostic(*ctx);

        server.LogDebug("Document updated: {}\n{}", params.textDocument.uri,
                        ctx->GetBackgroundCompilation()->GetBuffer());
    }

    auto LanguageService::OnDidCloseTextDocument(lsp::DidCloseTextDocumentParams params) -> void
    {
        auto itCtx = documentContexts.Find(params.textDocument.uri);
        if (itCtx == documentContexts.end()) {
            // Bad notification. Document is not open.
            server.LogInfo("Attempted to close a document that was not open: {}", params.textDocument.uri);
            return;
        }

        server.LogInfo("Closing document: {}", params.textDocument.uri);
        ScheduleBackgroundClosingDocument(std::move(itCtx->second));
        documentContexts.Erase(itCtx);
    }

#pragma endregion

#pragma region Language Features

    auto LanguageService::OnDocumentSymbol(int requestId, lsp::DocumentSymbolParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "documentSymbol", uri);
        ScheduleLanguageQuery<std::monostate>(uri, [&server = this->server, requestId, params = std::move(params)](
                                                       const LanguageQueryInfo* queryInfo, std::monostate&) {
            if (!queryInfo) {
                server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                               "documentSymbol", params.textDocument.uri);
                server.SendServerResponse(requestId,
                                          lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                return;
            }

            SimpleTimer timer;
            auto result = HandleDocumentSymbol(server.GetConfig().languageService.documentSymbol, *queryInfo, params);
            server.SendServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "documentSymbol",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnSemanticTokensFull(int requestId, lsp::SemanticTokensParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "semanticTokensFull", uri);
        ScheduleLanguageQuery<SemanticTokenState>(uri, [this, requestId,
                                                        params = std::move(params)](const LanguageQueryInfo* queryInfo,
                                                                                    SemanticTokenState& state) {
            if (!queryInfo) {
                server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                               "semanticTokensFull", params.textDocument.uri);
                server.SendServerResponse(requestId,
                                          lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                return;
            }

            SimpleTimer timer;
            lsp::SemanticTokens result =
                HandleSemanticTokens(server.GetConfig().languageService.semanticTokens, *queryInfo, state, params);
            server.SendServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "semanticTokensFull",
                           timer.GetElapsedMilliseconds());

            PublishInactiveRegions(params.textDocument.uri, *queryInfo);
        });
    }

    auto LanguageService::OnSemanticTokensDelta(int requestId, lsp::SemanticTokensDeltaParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "semanticTokensDelta", uri);
        ScheduleLanguageQuery<SemanticTokenState>(uri, [this, requestId,
                                                        params = std::move(params)](const LanguageQueryInfo* queryInfo,
                                                                                    SemanticTokenState& state) {
            if (!queryInfo) {
                server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                               "semanticTokensDelta", params.textDocument.uri);
                server.SendServerResponse(requestId,
                                          lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                return;
            }

            SimpleTimer timer;
            std::variant<lsp::SemanticTokens, lsp::SemanticTokensDelta> result =
                HandleSemanticTokensDelta(server.GetConfig().languageService.semanticTokens, *queryInfo, state, params);
            std::visit([&](auto&& arg) { server.SendServerResponse(requestId, arg, false); }, result);

            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "semanticTokensDelta",
                           timer.GetElapsedMilliseconds());

            PublishInactiveRegions(params.textDocument.uri, *queryInfo);
        });
    }

    auto LanguageService::OnCompletion(int requestId, lsp::CompletionParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "completion", uri);
        ScheduleLanguageQuery<CompletionState>(uri, [this, requestId, params = std::move(params)](
                                                        const LanguageQueryInfo* queryInfo, CompletionState& state) {
            if (!queryInfo) {
                server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                               "completion", params.textDocument.uri);
                server.SendServerResponse(requestId,
                                          lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                return;
            }

            SimpleTimer timer;
            lsp::CompletionList result =
                HandleCompletion(server.GetConfig().languageService.completion, *queryInfo, state, params);
            server.SendServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "completion",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnSignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "signatureHelp", uri);
        ScheduleLanguageQuery<SignatureHelpState>(uri, [this, requestId,
                                                        params = std::move(params)](const LanguageQueryInfo* queryInfo,
                                                                                    SignatureHelpState& state) {
            if (!queryInfo) {
                server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                               "signatureHelp", params.textDocument.uri);
                server.SendServerResponse(requestId,
                                          lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                return;
            }

            SimpleTimer timer;
            std::optional<lsp::SignatureHelp> result =
                HandleSignatureHelp(server.GetConfig().languageService.signatureHelp, *queryInfo, state, params);
            server.SendServerResponse(requestId, result, false);
            server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "signatureHelp",
                           timer.GetElapsedMilliseconds());
        });
    }

    auto LanguageService::OnHover(int requestId, lsp::HoverParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "hover", uri);
        ScheduleLanguageQuery<std::monostate>(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo* queryInfo, std::monostate&) {
                if (!queryInfo) {
                    server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                                   "hover", params.textDocument.uri);
                    server.SendServerResponse(
                        requestId, lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                    return;
                }

                SimpleTimer timer;
                std::optional<lsp::Hover> result =
                    HandleHover(server.GetConfig().languageService.hover, *queryInfo, params);
                server.SendServerResponse(requestId, result, false);
                server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "hover",
                               timer.GetElapsedMilliseconds());
            });
    }

    auto LanguageService::OnDefinition(int requestId, lsp::DefinitionParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "definition", uri);
        ScheduleLanguageQuery<std::monostate>(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo* queryInfo, std::monostate&) {
                if (!queryInfo) {
                    server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                                   "definition", params.textDocument.uri);
                    server.SendServerResponse(
                        requestId, lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                    return;
                }

                SimpleTimer timer;
                std::vector<lsp::Location> result =
                    HandleDefinition(server.GetConfig().languageService.definition, *queryInfo, params);
                server.SendServerResponse(requestId, result, false);
                server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "definition",
                               timer.GetElapsedMilliseconds());
            });
    }

    auto LanguageService::OnReferences(int requestId, lsp::ReferenceParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "references", uri);
        ScheduleLanguageQuery<std::monostate>(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo* queryInfo, std::monostate&) {
                if (!queryInfo) {
                    server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                                   "references", params.textDocument.uri);
                    server.SendServerResponse(
                        requestId, lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                    return;
                }

                SimpleTimer timer;
                std::vector<lsp::Location> result =
                    HandleReferences(server.GetConfig().languageService.reference, *queryInfo, params);
                server.SendServerResponse(requestId, result, false);
                server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "references",
                               timer.GetElapsedMilliseconds());
            });
    }

    auto LanguageService::OnInlayHint(int requestId, lsp::InlayHintParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "inlayHint", uri);
        ScheduleLanguageQuery<std::monostate>(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo* queryInfo, std::monostate&) {
                if (!queryInfo) {
                    server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                                   "inlayHint", params.textDocument.uri);
                    server.SendServerResponse(
                        requestId, lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                    return;
                }

                SimpleTimer timer;
                std::vector<lsp::InlayHint> result =
                    HandleInlayHints(server.GetConfig().languageService.inlayHint, *queryInfo, params);
                server.SendServerResponse(requestId, result, false);
                server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "inlayHint",
                               timer.GetElapsedMilliseconds());
            });
    }

    auto LanguageService::OnFoldingRange(int requestId, lsp::FoldingRangeParams params) -> void
    {
        auto uri = params.textDocument.uri;
        server.LogInfo("Received request {} {}: {}", requestId, "foldingRange", uri);
        ScheduleLanguageQuery<std::monostate>(
            uri, [this, requestId, params = std::move(params)](const LanguageQueryInfo* queryInfo, std::monostate&) {
                if (!queryInfo) {
                    server.LogInfo("Failed to handle request {} {}: compilation failed for document {}", requestId,
                                   "foldingRange", params.textDocument.uri);
                    server.SendServerResponse(
                        requestId, lsp::ResponseError{.code = -32803, .message = "Compilation failed."}, true);
                    return;
                }

                SimpleTimer timer;
                std::vector<lsp::FoldingRange> result =
                    HandleFoldingRange(server.GetConfig().languageService.foldingRange, *queryInfo, params);
                server.SendServerResponse(requestId, result, false);
                server.LogInfo("Responded to request {} {}. Processing took {} ms", requestId, "foldingRange",
                               timer.GetElapsedMilliseconds());
            });
    }

#pragma endregion

} // namespace glsld

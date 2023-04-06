#pragma once
#include "Protocol.h"
#include "LanguageServerInterface.h"
#include "Compiler.h"
#include "SourceText.h"

#include <mutex>
#include <condition_variable>

namespace glsld
{
    auto ComputeDocumentSymbol(const CompilerObject& compileResult) -> std::vector<lsp::DocumentSymbol>;

    auto GetTokenLegend() -> lsp::SemanticTokensLegend;
    auto ComputeSemanticTokens(const CompilerObject& compileResult) -> lsp::SemanticTokens;
    auto ComputeSemanticTokensDelta(const CompilerObject& compileResult) -> lsp::SemanticTokensDelta;

    auto ComputeCompletion(const CompilerObject& compileResult, lsp::Position position)
        -> std::vector<lsp::CompletionItem>;

    auto ComputeSignatureHelp(const CompilerObject& compileResult, lsp::Position position)
        -> std::optional<lsp::SignatureHelp>;

    auto ComputeHover(const CompilerObject& compileResult, lsp::Position position) -> std::optional<lsp::Hover>;

    // we assume single source file for now
    auto ComputeDeclaration(const CompilerObject& compileResult, const lsp::DocumentUri& uri, lsp::Position position)
        -> std::vector<lsp::Location>;

    auto ComputeInlayHint(const CompilerObject& compileResult, lsp::Range range) -> std::vector<lsp::InlayHint>;

    auto ComputeDocumentColor(const CompilerObject& compileResult) -> std::vector<lsp::ColorInformation>;

    auto GetDefaultLibraryModule() -> std::shared_ptr<CompiledPreamble>;

    class IntellisenseProvider
    {
    public:
        IntellisenseProvider(int version, std::string sourceString)
            : version(version), sourceString(std::move(sourceString))
        {
        }

        auto Setup()
        {
            // compileResult = Compile(sourceString, GetDefaultLibraryModule());
            compilerObject = std::make_unique<CompilerObject>();
            compilerObject->AddIncludePath("e:/Project/glsld/.vscode/");
            compilerObject->Compile(sourceString);

            std::unique_lock<std::mutex> lock{mu};
            available = true;
            cv.notify_all();
        }

        auto WaitAvailable() -> bool
        {
            using namespace std::literals;
            std::unique_lock<std::mutex> lock{mu};
            if (available || cv.wait_for(lock, 1s) == std::cv_status::no_timeout) {
                return available;
            }

            return false;
        }

        auto StealBuffer() -> std::string
        {
            std::unique_lock<std::mutex> lock{mu};
            if (available) {
                // After compilation finishes, the sourceString buffer is no longer needed
                return std::move(sourceString);
            }
            else {
                return sourceString;
            }
        }

        auto GetSourceString() const -> const std::string&
        {
            return sourceString;
        }

        auto GetCompilerObject() const -> const CompilerObject&
        {
            GLSLD_ASSERT(available);
            return *compilerObject;
        }

    private:
        int version;
        std::string sourceString;
        std::unique_ptr<CompilerObject> compilerObject;

        std::atomic<bool> available = false;
        std::mutex mu;
        std::condition_variable cv;
    };

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
                        .colorProvider = true,
                    },
                .serverInfo =
                    {
                        .name    = "glsld",
                        .version = "0.0.1",
                    },
            };
            server->HandleServerResponse(requestId, result, false);
        }

#pragma region Document Synchronization

        auto DidOpenTextDocument(lsp::DidOpenTextDocumentParams params) -> void
        {
            auto& providerEntry = providerLookup[params.textDocument.uri];
            if (providerEntry) {
                // Bad request
                return;
            }

            providerEntry = std::make_shared<IntellisenseProvider>(params.textDocument.version,
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
            providerEntry =
                std::make_shared<IntellisenseProvider>(params.textDocument.version, std::move(sourceBuffer));

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
            auto provider = providerLookup[params.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, provider = std::move(provider)] {
                    if (provider->WaitAvailable()) {
                        auto result = ComputeDocumentSymbol(provider->GetCompilerObject());
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto SemanticTokensFull(int requestId, lsp::SemanticTokensParam params) -> void
        {
            auto provider = providerLookup[params.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, provider = std::move(provider)] {
                    if (provider->WaitAvailable()) {
                        lsp::SemanticTokens result = ComputeSemanticTokens(provider->GetCompilerObject());
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto Completion(int requestId, lsp::CompletionParams params) -> void
        {
            auto provider = providerLookup[params.baseParams.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, params = std::move(params), provider = std::move(provider)] {
                    if (provider->WaitAvailable()) {
                        std::vector<lsp::CompletionItem> result =
                            ComputeCompletion(provider->GetCompilerObject(), params.baseParams.position);
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto SignatureHelp(int requestId, lsp::SignatureHelpParams params) -> void
        {
            auto provider = providerLookup[params.baseParams.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, params = std::move(params), provider = std::move(provider)] {
                    if (provider->WaitAvailable()) {
                        std::optional<lsp::SignatureHelp> result =
                            ComputeSignatureHelp(provider->GetCompilerObject(), params.baseParams.position);
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto Hover(int requestId, lsp::HoverParams params) -> void
        {
            auto provider = providerLookup[params.baseParams.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, params = std::move(params), provider = std::move(provider)] {
                    if (provider->WaitAvailable()) {
                        std::optional<lsp::Hover> result =
                            ComputeHover(provider->GetCompilerObject(), params.baseParams.position);
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto Declaration(int requestId, lsp::DeclarationParams params) -> void
        {
            auto provider = providerLookup[params.baseParams.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, params = std::move(params), provider = std::move(provider)] {
                    if (provider->WaitAvailable()) {
                        std::vector<lsp::Location> result =
                            ComputeDeclaration(provider->GetCompilerObject(), params.baseParams.textDocument.uri,
                                               params.baseParams.position);
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto Definition(int requestId, lsp::DefinitionParams params) -> void
        {
            auto provider = providerLookup[params.baseParams.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, params = std::move(params), provider = std::move(provider)] {
                    if (provider->WaitAvailable()) {
                        std::vector<lsp::Location> result =
                            ComputeDeclaration(provider->GetCompilerObject(), params.baseParams.textDocument.uri,
                                               params.baseParams.position);
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto InlayHint(int requestId, lsp::InlayHintParams params) -> void
        {
            auto provider = providerLookup[params.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, params = std::move(params), provider = std::move(provider)] {
                    if (provider->WaitAvailable()) {
                        std::vector<lsp::InlayHint> result =
                            ComputeInlayHint(provider->GetCompilerObject(), params.range);
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto DocumentColor(int requestId, lsp::DocumentColorParams params) -> void
        {
            auto provider = providerLookup[params.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, provider = std::move(provider)] {
                    if (provider->WaitAvailable()) {
                        auto result = ComputeDocumentColor(provider->GetCompilerObject());
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

#pragma endregion

#pragma region Window Features

        auto ShowMessage(lsp::ShowMessageParams params) -> void
        {
            server->HandleServerNotification(lsp::LSPMethod_ShowMessage, params);
        }

#pragma endregion

    private:
        template <typename F, typename... Args>
        auto ScheduleTask(F&& f, Args&&... args) -> void
        {
            std::thread thd{std::forward<F>(f), std::forward<Args>(args)...};
            thd.detach();
        }

        // uri -> provider
        std::map<std::string, std::shared_ptr<IntellisenseProvider>> providerLookup;

        LanguageServerCallback* server;
    };
} // namespace glsld
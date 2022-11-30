#pragma once
#include "Protocol.h"
#include "LanguageServerInterface.h"
#include "Compiler.h"
#include "TextEdit.h"

namespace glsld
{
    auto ComputeDocumentSymbol(GlsldCompiler& compiler) -> std::vector<lsp::DocumentSymbol>;

    auto GetTokenLegend() -> lsp::SemanticTokensLegend;
    auto ComputeSemanticTokens(GlsldCompiler& compiler) -> lsp::SemanticTokens;
    auto ComputeSemanticTokensDelta(GlsldCompiler& compiler) -> lsp::SemanticTokensDelta;

    auto ComputeHover(GlsldCompiler& compiler, lsp::Position position) -> std::optional<lsp::Hover>;

    class IntellisenseProvider
    {
    public:
        IntellisenseProvider(int version, std::string sourceString) : version(version), sourceString(sourceString)
        {
        }

        auto Setup()
        {
            compiler.Compile(std::move(sourceString));

            std::unique_lock<std::mutex> lock{mu};
            available = true;
            cv.notify_all();
        }

        auto Wait() -> bool
        {
            using namespace std::literals;
            std::unique_lock<std::mutex> lock{mu};
            if (available || cv.wait_for(lock, 1s) == std::cv_status::no_timeout) {
                return available;
            }

            return false;
        }

        auto GetCompiler() -> GlsldCompiler&
        {
            return compiler;
        }

    private:
        int version;
        std::string sourceString;
        GlsldCompiler compiler;

        std::atomic<bool> available = false;
        std::mutex mu;
        std::condition_variable cv;
    };

    class TextReader
    {
    public:
        auto Apply(const lsp::TextDocumentContentChangeEvent& changeEvent)
        {
            int line          = 0;
            int charcter      = 0;
            auto changedRange = changeEvent.range;
            for (char ch : input) {
                // if ((line == changedRange->start.line && charcter >= changedRange->start.character) ||
                //     (line > changedRange->start.line && line < changedRange->start.line) ||
                //     (line == changedRange->end.line && charcter < changedRange->end.character)) {
                //     continue;
                // }
                if (line >= changedRange->start.line && charcter >= changedRange->start.character) {
                    if (line >= changedRange->end.line && charcter >= changedRange->end.character) {
                        break;
                    }
                    continue;
                }

                if (ch == '\n') {
                    line += 1;
                    charcter = 0;
                }
                else {
                    charcter += 1;
                }
            }
        }

        std::string input;
        std::string output;
    };

    class LanguageService
    {
    public:
        LanguageService(LanguageServerCallback* server) : server(server)
        {
        }

        auto Initialize(int requestId, lsp::InitializeParams params) -> void
        {
            auto result = lsp::InitializedResult{
                .capabilities =
                    {
                        .textDocumentSync =
                            {
                                .openClose = true,
                                .change    = lsp::TextDocumentSyncKind::Full,
                            },
                        .hoverProvider          = true,
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
                        // .hoverProvider          = false,
                        // .declarationProvider    = false,
                        // .definitionProvider     = false,
                        // .documentSymbolProvider = false,
                        // .semanticTokensProvider = std::nullopt,
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
            auto provider = (providerLookup[params.textDocument.uri] = std::make_shared<IntellisenseProvider>(
                                 params.textDocument.version, std::move(params.textDocument.text)));

            ScheduleTask([provider = std::move(provider)]() { provider->Setup(); });
        }
        auto DidChangeTextDocument(lsp::DidChangeTextDocumentParams params) -> void
        {
            auto provider = (providerLookup[params.textDocument.uri] = std::make_shared<IntellisenseProvider>(
                                 params.textDocument.version, std::move(params.contentChanges[0].text)));
            ScheduleTask([provider = std::move(provider)]() { provider->Setup(); });
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
                    if (provider->Wait()) {
                        auto result = ComputeDocumentSymbol(provider->GetCompiler());
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
                    if (provider->Wait()) {
                        lsp::SemanticTokens result = ComputeSemanticTokens(provider->GetCompiler());
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto Completion(int requestId, lsp::CompletionParams params) -> void
        {
        }

        auto Hover(int requestId, lsp::HoverParams params) -> void
        {
            auto provider = providerLookup[params.baseParams.textDocument.uri];
            if (provider != nullptr) {
                ScheduleTask([this, requestId, position = params.baseParams.position, provider = std::move(provider)] {
                    if (provider->Wait()) {
                        std::optional<lsp::Hover> result = ComputeHover(provider->GetCompiler(), position);
                        server->HandleServerResponse(requestId, result, false);
                    }
                });
            }
        }

        auto Declaration(int requestId, lsp::DeclarationParams params) -> void
        {
            std::vector<lsp::Location> result;
            server->HandleServerResponse(requestId, result, false);
        }

        auto Definition(int requestId, lsp::DefinitionParams params) -> void
        {
            std::vector<lsp::Location> result;
            server->HandleServerResponse(requestId, result, false);
        }

        // auto InlayHint(int requestId, lsp::InlayHintParams params) -> void {
        // }

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
#pragma once
#include "Server/Protocol.h"
#include "Server/LanguageServer.h"
#include "Server/LanguageQueryInfo.h"
#include "Support/Uri.h"

#include <mutex>
#include <condition_variable>

namespace glsld
{
    class BackgroundCompilation
    {
    private:
        // Document version
        int version;
        std::string uri;
        std::string sourceString;

        std::unique_ptr<CompilerInvocation> compiler = nullptr;
        std::unique_ptr<LanguageQueryInfo> info      = nullptr;

        std::atomic<bool> available = false;
        std::mutex mu;
        std::condition_variable cv;

    public:
        BackgroundCompilation(int version, std::string uri, std::string sourceString)
            : version(version), uri(std::move(uri)), sourceString(std::move(sourceString))
        {
        }

        auto Setup()
        {
            auto ppInfoStore = std::make_unique<PreprocessSymbolStore>();
            auto ppCallback  = ppInfoStore->GetCollectionCallback();

            compiler = std::make_unique<CompilerInvocation>(GetStdlibModule());
            compiler->SetCountUtf16Characters(true);
            compiler->AddIncludePath(std::filesystem::path(Uri::FromString(uri)->GetPath().StdStrView()).parent_path());
            compiler->SetMainFileFromBuffer(sourceString);
            auto result = compiler->CompileMainFile(ppCallback.get());

            info = std::make_unique<LanguageQueryInfo>(std::move(result), std::move(ppInfoStore));

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

        auto GetBuffer() const -> StringView
        {
            return sourceString;
        }

        auto GetProvider() -> const LanguageQueryInfo&
        {
            GLSLD_ASSERT(available);
            return *info;
        }
    };

    class LanguageService
    {
    private:
        LanguageServer& server;

        // uri -> provider
        std::map<std::string, std::shared_ptr<BackgroundCompilation>> providerLookup;

        // Schedule a language query for the given uri in a background thread, which waits for the compilation and then
        // runs the callback.
        auto ScheduleLanguageQuery(const std::string& uri, std::function<auto(const LanguageQueryInfo&)->void> callback)
            -> void;

    public:
        LanguageService(LanguageServer& server) : server(server)
        {
        }

        auto Initialize(int requestId, lsp::InitializeParams params) -> void;

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

        auto Declaration(int requestId, lsp::DeclarationParams params) -> void;

        auto References(int requestId, lsp::ReferenceParams params) -> void;

        auto InlayHint(int requestId, lsp::InlayHintParams params) -> void;

#pragma endregion

#pragma region Window Features

        auto ShowMessage(lsp::ShowMessageParams params) -> void;

#pragma endregion
    };
} // namespace glsld
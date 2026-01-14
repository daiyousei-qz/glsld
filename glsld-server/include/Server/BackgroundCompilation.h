#pragma once
#include "Compiler/CompilerConfig.h"
#include "Compiler/CompilerResult.h"
#include "Server/LanguageQueryInfo.h"

#include "exec/async_scope.hpp"
#include "stdexec/execution.hpp"

#include <atomic>
#include <memory>

namespace glsld
{
    class CompletionPreambleInfo;
    class SignatureHelpPreambleInfo;

    class LanguagePreambleInfo
    {
    private:
        std::shared_ptr<PrecompiledPreamble> preamble = nullptr;
        // We store other pre-computed stuffs here for especially completion and signature help.
        std::unique_ptr<CompletionPreambleInfo> completionData;
        std::unique_ptr<SignatureHelpPreambleInfo> signatureHelpData;

    public:
        LanguagePreambleInfo();
        LanguagePreambleInfo(std::shared_ptr<PrecompiledPreamble> preamble);
        ~LanguagePreambleInfo();

        auto GetPreamble() const -> const std::shared_ptr<PrecompiledPreamble>&
        {
            return preamble;
        }

        auto GetCompletionInfo() const -> const CompletionPreambleInfo&
        {
            return *completionData;
        }

        auto GetSignatureHelpInfo() const -> const SignatureHelpPreambleInfo&
        {
            return *signatureHelpData;
        }
    };

    class BackgroundCompilation : std::enable_shared_from_this<BackgroundCompilation>
    {
    private:
        // Document version
        int version;
        std::string uri;
        std::string sourceString;
        std::shared_ptr<LanguagePreambleInfo> preambleInfo;

        // Protects background compilation
        exec::async_scope scope;

        LanguageConfig nextConfig;
        std::unique_ptr<LanguageQueryInfo> info = nullptr;

        // Set when the compilation result is available
        std::atomic<bool> isAvailable = false;

        // Set if the compilation is expired by a newer version
        std::atomic<bool> isExpired = false;

        // Performs the core background compilation work for this document.
        // This is to be scheduled on a background thread and tracked by the async_scope.
        auto Run() -> void;

    public:
        BackgroundCompilation(int version, std::string uri, std::string sourceString,
                              std::shared_ptr<LanguagePreambleInfo> preambleInfo)
            : version(version), uri(std::move(uri)), sourceString(std::move(sourceString)),
              preambleInfo(std::move(preambleInfo))
        {
        }

        BackgroundCompilation(const BackgroundCompilation&)            = delete;
        BackgroundCompilation(BackgroundCompilation&&)                 = delete;
        BackgroundCompilation& operator=(const BackgroundCompilation&) = delete;
        BackgroundCompilation& operator=(BackgroundCompilation&&)      = delete;

        template <stdexec::scheduler Scheduler>
        auto Setup(Scheduler&& schd) -> void
        {
            // Because compilation may take longer than the lifetime of the document being valid,
            // we capture shared_from_this() here to keep the instance alive until the compilation is done.
            scope.spawn(stdexec::schedule(std::forward<Scheduler>(schd)) |
                        stdexec::then([self = shared_from_this()]() { self->Run(); }));
        }

        auto OnAvailable()
        {
            return scope.on_empty();
        }

        auto MarkExpired() -> void
        {
            isExpired = true;
        }

        auto TestExpired() -> bool
        {
            return isExpired;
        }

        auto GetVersion() const -> int
        {
            return version;
        }

        auto GetUri() const -> StringView
        {
            return uri;
        }

        auto GetBuffer() const -> StringView
        {
            return sourceString;
        }

        auto GetPreambleInfo() const -> const LanguagePreambleInfo&
        {
            return *preambleInfo;
        }

        auto GetLanguageConfig() const -> const LanguageConfig&
        {
            return preambleInfo->GetPreamble()->GetLanguageConfig();
        }

        auto GetNextLanguageConfig() const -> const LanguageConfig&
        {
            if (isAvailable) {
                return nextConfig;
            }
            else {
                // If the compilation has not yet finished, return the current config as a fallback as
                // we need start a new compilation now.
                return preambleInfo->GetPreamble()->GetLanguageConfig();
            }
        }

        // NOTE this must be called after WaitAvailable() returns true
        auto GetLanguageQueryInfo() -> const LanguageQueryInfo&
        {
            GLSLD_ASSERT(isAvailable);
            return *info;
        }
    };

} // namespace glsld
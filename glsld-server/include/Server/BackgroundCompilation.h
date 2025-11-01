#pragma once
#include "Compiler/CompilerConfig.h"
#include "Compiler/CompilerResult.h"
#include "Server/LanguageQueryInfo.h"

#include <memory>
#include <mutex>
#include <condition_variable>

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

    class BackgroundCompilation
    {
    private:
        // Document version
        int version;
        std::string uri;
        std::string sourceString;
        std::shared_ptr<LanguagePreambleInfo> preambleInfo;

        LanguageConfig nextConfig;
        std::unique_ptr<LanguageQueryInfo> info = nullptr;

        std::atomic<bool> available = false;
        std::mutex mu;
        std::condition_variable cv;

    public:
        BackgroundCompilation(int version, std::string uri, std::string sourceString,
                              std::shared_ptr<LanguagePreambleInfo> preambleInfo)
            : version(version), uri(std::move(uri)), sourceString(std::move(sourceString)),
              preambleInfo(std::move(preambleInfo))
        {
        }

        auto Setup() -> void;

        auto WaitAvailable() -> bool;

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

        auto GetNextLanguageConfig() const -> const LanguageConfig&
        {
            // FIXME: this could result in a race condition if the previous compilation is still ongoing
            GLSLD_ASSERT(available);
            return nextConfig;
        }

        auto GetLanguageQueryInfo() -> const LanguageQueryInfo&
        {
            GLSLD_ASSERT(available);
            return *info;
        }
    };

} // namespace glsld
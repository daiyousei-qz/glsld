#pragma once
#include "Compiler/CompilerConfig.h"
#include "Compiler/CompilerResult.h"
#include "Server/LanguageQueryInfo.h"
#include "Support/AsyncLatch.h"

#include <atomic>
#include <memory>

namespace glsld
{
    class BackgroundCompilation : std::enable_shared_from_this<BackgroundCompilation>
    {
    private:
        // Document version
        int version;
        std::string uri;
        std::string sourceString;
        std::shared_ptr<PrecompiledPreamble> preamble;

        // Gates background compilation, including next language config and query info
        AsyncLatch latchCompilation;

        LanguageConfig nextConfig;
        std::unique_ptr<LanguageQueryInfo> info = nullptr;

        // Set when the compilation result is available
        std::atomic<bool> isAvailable = false;

        // Set if the compilation is expired by a newer version
        std::atomic<bool> isExpired = false;

    public:
        BackgroundCompilation(int version, std::string uri, std::string sourceString,
                              std::shared_ptr<PrecompiledPreamble> preamble)
            : version(version), uri(std::move(uri)), sourceString(std::move(sourceString)),
              preamble(std::move(preamble))
        {
        }

        BackgroundCompilation(const BackgroundCompilation&)            = delete;
        BackgroundCompilation(BackgroundCompilation&&)                 = delete;
        BackgroundCompilation& operator=(const BackgroundCompilation&) = delete;
        BackgroundCompilation& operator=(BackgroundCompilation&&)      = delete;

        // Synchronously run the compilation.
        // Once it is done, set the flag and release the latch to signal availability.
        auto Run() -> void;

        auto AsyncWaitAvailable()
        {
            return latchCompilation.AsyncWait();
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

        auto GetLanguageConfig() const -> const LanguageConfig&
        {
            return preamble->GetLanguageConfig();
        }

        auto GetNextLanguageConfig() const -> const LanguageConfig&
        {
            if (isAvailable) {
                return nextConfig;
            }
            else {
                // If the compilation has not yet finished, return the current config as a fallback as
                // we need start a new compilation now.
                return preamble->GetLanguageConfig();
            }
        }

        // NOTE this must be called after availability is signaled
        auto GetLanguageQueryInfo() -> const LanguageQueryInfo&
        {
            if (isAvailable) {
                return *info;
            }
            else {
                GLSLD_ASSERT(false && "LanguageQueryInfo is not yet available");
                GLSLD_UNREACHABLE();
            }
        }
    };

} // namespace glsld
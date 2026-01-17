#pragma once
#include "Compiler/CompilerConfig.h"
#include "Compiler/CompilerResult.h"
#include "Server/LanguageQueryInfo.h"
#include "Support/AsyncLatch.h"

#include <atomic>
#include <memory>

namespace glsld
{
    class BackgroundCompilation
    {
    private:
        // Document version
        const int version;
        const std::string uri;
        const std::string sourceString;
        const LanguageConfig languageConfig;
        const std::shared_ptr<PrecompiledPreamble> preamble = nullptr;

        // Gates background compilation. This releases waiters when compilation is done.
        AsyncLatch latchCompilation;

        // Preamble that is used in this compilation, which may be shared later. This is available and immutable after
        // `isPreambleAvailable` is set.
        std::shared_ptr<PrecompiledPreamble> nextPreamble;

        // Language config collected during this compilation. This is available and immutable after `isAvailable` is
        // set.
        LanguageConfig nextConfig;

        // Compilation result. This is available and immutable after `isAvailable` is set.
        std::unique_ptr<LanguageQueryInfo> info = nullptr;

        // Set when the preamble is available
        std::atomic<bool> isPreambleAvailable = false;

        // Set when the compilation result is available
        std::atomic<bool> isAvailable = false;

        // Set if the compilation is expired by a newer version
        std::atomic<bool> isExpired = false;

    public:
        BackgroundCompilation(int version, std::string uri, std::string sourceString, LanguageConfig languageConfig,
                              std::shared_ptr<PrecompiledPreamble> preamble)
            : version(version), uri(std::move(uri)), sourceString(std::move(sourceString)),
              languageConfig(languageConfig), preamble(std::move(preamble))
        {
            GLSLD_ASSERT(this->preamble == nullptr || languageConfig == this->preamble->GetLanguageConfig());
        }

        // This class is always pinned in heap after creation
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

        auto IsPreambleAvailable() const -> bool
        {
            return isPreambleAvailable.load(std::memory_order_acquire);
        }

        auto IsAvailable() const -> bool
        {
            return isAvailable.load(std::memory_order_acquire);
        }

        auto SetExpired() -> void
        {
            isExpired.store(true, std::memory_order_relaxed);
        }

        auto IsExpired() const -> bool
        {
            return isExpired.load(std::memory_order_relaxed);
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
            return languageConfig;
        }

        auto GetNextPreamble() const -> std::shared_ptr<PrecompiledPreamble>
        {
            if (IsPreambleAvailable()) {
                return nextPreamble;
            }
            else {
                // If the compilation of the preamble has not yet finished, return the current preamble (could be
                // nullptr) as a fallback as we need to start a new compilation now.
                return preamble;
            }
        }

        auto GetNextLanguageConfig() const -> const LanguageConfig&
        {
            if (IsAvailable()) {
                return nextConfig;
            }
            else {
                // If the compilation has not yet finished, return the current config as a fallback as
                // we need to start a new compilation now.
                return languageConfig;
            }
        }

        // NOTE this must be called after availability is signaled
        auto GetLanguageQueryInfo() -> const LanguageQueryInfo&
        {
            if (IsAvailable()) {
                return *info;
            }
            else {
                GLSLD_ASSERT(false && "LanguageQueryInfo is not yet available");
                GLSLD_UNREACHABLE();
            }
        }
    };

} // namespace glsld
#include "Server/BackgroundCompilation.h"

#include "Compiler/CompilerInvocation.h"
#include "Compiler/PPCallback.h"
#include "Language/Extension.h"
#include "Support/Uri.h"

namespace glsld
{
    class LanguageConfigCollector : public PPCallback
    {
    private:
        LanguageConfig& config;

    public:
        LanguageConfigCollector(LanguageConfig& config) : config(config)
        {
        }

        auto OnVersionDirective(ArrayView<PPToken> tokens, GlslVersion version, GlslProfile profile) -> void override
        {
            config.version = version;
            config.profile = profile;
        }

        auto OnExtensionDirective(ArrayView<PPToken> tokens, ExtensionId extension, ExtensionBehavior behavior)
            -> void override
        {
            if (behavior == ExtensionBehavior::Enable || behavior == ExtensionBehavior::Require) {
                config.extensions.EnableExtension(extension);
            }
        }
    };

    auto BackgroundCompilation::Run() -> void
    {
        // First pass:
        std::shared_ptr<PrecompiledPreamble> localPreamble = preamble;
        if (localPreamble == nullptr) {
            CompilerInvocation invocation;
            invocation.ApplyLanguageConfig(languageConfig);
            localPreamble = invocation.CompilePreamble(nullptr);
        }

        nextPreamble = localPreamble;
        isPreambleAvailable.store(true, std::memory_order_release);

        // Second pass:
        auto ppInfoStore    = std::make_unique<PreprocessSymbolStore>();
        auto ppInfoCallback = ppInfoStore->GetCollectionCallback();

        nextConfig                   = localPreamble->GetLanguageConfig();
        auto configCollectorCallback = LanguageConfigCollector{nextConfig};

        auto compiler = std::make_unique<CompilerInvocation>(std::move(localPreamble));
        compiler->SetCountUtf16Characters(true);
        compiler->AddIncludePath(std::filesystem::path(Uri::FromString(uri)->GetPath().StdStrView()).parent_path());
        compiler->SetMainFileFromBuffer(sourceString);

        auto combinedCallback = CombinedPPCallback{&configCollectorCallback, ppInfoCallback.get()};
        auto result           = compiler->CompileMainFile(&combinedCallback);

        info = std::make_unique<LanguageQueryInfo>(std::move(result), std::move(ppInfoStore));
        isAvailable.store(true, std::memory_order_release);

        // Signal availability
        latchCompilation.CountDown();
    }

} // namespace glsld
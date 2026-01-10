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

        auto OnVersionDirective(FileID file, TextRange range, GlslVersion version, GlslProfile profile) -> void override
        {
            config.version = version;
            config.profile = profile;
        }

        auto OnExtensionDirective(FileID file, TextRange range, ExtensionId extension, ExtensionBehavior behavior)
            -> void override
        {
            if (behavior == ExtensionBehavior::Enable || behavior == ExtensionBehavior::Require) {
                config.extensions.EnableExtension(extension);
            }
        }
    };

    auto BackgroundCompilation::Setup() -> void
    {
        auto ppInfoStore    = std::make_unique<PreprocessSymbolStore>();
        auto ppInfoCallback = ppInfoStore->GetCollectionCallback();

        nextConfig                   = preambleInfo->GetPreamble()->GetLanguageConfig();
        auto configCollectorCallback = LanguageConfigCollector{nextConfig};

        auto compiler = std::make_unique<CompilerInvocation>(preambleInfo->GetPreamble());
        compiler->SetCountUtf16Characters(true);
        compiler->AddIncludePath(std::filesystem::path(Uri::FromString(uri)->GetPath().StdStrView()).parent_path());
        compiler->SetMainFileFromBuffer(sourceString);

        auto combinedCallback = CombinedPPCallback{&configCollectorCallback, ppInfoCallback.get()};
        auto result           = compiler->CompileMainFile(&combinedCallback);

        info = std::make_unique<LanguageQueryInfo>(std::move(result), std::move(ppInfoStore));

        std::unique_lock<std::mutex> lock{mu};
        isAvailable = true;
        cv.notify_all();
    }

    auto BackgroundCompilation::WaitAvailable() -> bool
    {
        using namespace std::literals;
        std::unique_lock<std::mutex> lock{mu};
        if (isAvailable || cv.wait_for(lock, 1s) == std::cv_status::no_timeout) {
            return isAvailable;
        }

        return false;
    }
} // namespace glsld
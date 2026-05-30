#include "Server/BackgroundCompilation.h"

#include "Compiler/CompilerInvocation.h"
#include "Compiler/PPCallback.h"
#include "Language/Extension.h"
#include "Support/ScopeExit.h"
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
        ScopeExit onExit([this]() {
            // Signal availability
            latchCompilation.CountDown();
        });

        auto parsedUri = ParsedUri::Parse(uri);
        if (!parsedUri) {
            // FIXME: handle this case properly
            return;
        }

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
        auto ppInfoStore    = std::make_unique<PreprocessInfoStore>();
        auto ppInfoCallback = ppInfoStore->CreateCollectionCallback(&localPreamble->GetMacroTable());

        nextConfig                   = localPreamble->GetLanguageConfig();
        auto configCollectorCallback = LanguageConfigCollector{nextConfig};

        auto compiler = std::make_unique<CompilerInvocation>(std::move(localPreamble));
        compiler->SetCountUtf16Characters(true);
        compiler->SetMainFileFromBuffer(sourceString);

        if (auto filePath = parsedUri->ToFileSystemPath(); filePath) {
            compiler->AddIncludePath(filePath->parent_path());
        }

        auto combinedCallback = CombinedPPCallback{&configCollectorCallback, ppInfoCallback.get()};
        auto result           = compiler->CompileMainFile(&combinedCallback);

        info = std::make_unique<LanguageQueryInfo>(std::move(result), std::move(ppInfoStore));
        isAvailable.store(true, std::memory_order_release);
    }

    auto BackgroundCompilation::UpdateWithEdits(int newVersion, ArrayView<TextEdit> edits)
        -> std::shared_ptr<BackgroundCompilation>
    {
        // TODO: Could have a buffer manager so we don't keep allocating new buffers if user types faster than
        // compilation
        // TODO: Research whether adding a line-offset hints vector speeds up editing
        auto sourceBuffer = sourceString;
        for (const auto& change : edits) {
            if (change.range) {
                ApplySourceChange(sourceBuffer, *change.range, StringView{change.newText});
            }
            else {
                sourceBuffer = change.newText.Str();
            }
        }

        auto nextConfig   = GetNextLanguageConfig();
        auto nextPreamble = GetNextPreamble();
        if (nextPreamble && nextPreamble->GetLanguageConfig() != nextConfig) {
            // Preamble is outdated, discard it
            nextPreamble = nullptr;
        }
        auto result =
            std::make_shared<BackgroundCompilation>(newVersion, uri, std::move(sourceBuffer), nextConfig, nextPreamble);

        isExpired.store(true, std::memory_order_relaxed);
        return result;
    }
} // namespace glsld
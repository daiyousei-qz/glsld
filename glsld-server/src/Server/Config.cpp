#include "Server/Config.h"
#include "Support/JsonSerializer.h"

namespace glsld
{
    auto GetDefaultLanguageServerConfig() -> LanguageServerConfig
    {
        return LanguageServerConfig{
            .languageService =
                LanguageServiceConfig{
                    //.declaration   = DeclarationConfig{},
                    .definition =
                        DefinitionConfig{
                            .enable = true,
                        },
                    .reference =
                        ReferenceConfig{
                            .enable = true,
                        },
                    .hover =
                        HoverConfig{
                            .enable = true,
                        },
                    .foldingRange =
                        FoldingRangeConfig{
                            // Although folding range is implemented, it seems it's not better than default VSCode
                            // heuristics.
                            // We disable it by default, and users can enable it in config if needed.
                            .enable = false,
                        },
                    .documentSymbol =
                        DocumentSymbolConfig{
                            .enable = true,
                        },
                    .semanticTokens =
                        SemanticTokenConfig{
                            .enable = true,
                        },
                    .inlayHint =
                        InlayHintConfig{
                            .enable                      = true,
                            .enableArgumentNameHint      = true,
                            .enableInitializerHint       = true,
                            .enableImplicitArraySizeHint = true,
                            .enableImplicitCastHint      = true,
                            .enableBlockEndHint          = true,
                            .blockEndHintLineThreshold   = 2,
                        },
                    .completion =
                        CompletionConfig{
                            .enable = true,
                        },
                    .diagnostic =
                        DiagnosticConfig{
                            // This is still experimental, disable by default for now.
                            .enable = false,
                        },
                    .signatureHelp =
                        SignatureHelpConfig{
                            .enable = true,
                        },
                },
            .loggingLevel = LoggingLevel::Info,
        };
    }

    auto ParseLanguageServerConfig(StringView configFile) -> std::optional<LanguageServerConfig>
    {
        auto json = nlohmann::json::parse(configFile, nullptr, false, true);
        if (!json.is_discarded()) {
            LanguageServerConfig result = GetDefaultLanguageServerConfig();
            if (JsonSerializer<LanguageServerConfig>::Deserialize(result, json)) {
                return result;
            }
        }

        return std::nullopt;
    }
} // namespace glsld
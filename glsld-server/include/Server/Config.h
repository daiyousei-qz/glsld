#pragma once

#include "Support/StringView.h"
#include "Support/SerializerUtils.h"

#include <optional>

namespace glsld
{
    struct DeclarationConfig
    {
        // Master toggle for declaration.
        bool enable = false;
    };

    struct DefinitionConfig
    {
        // Master toggle for definition.
        bool enable = false;
    };

    struct ReferenceConfig
    {
        // Master toggle for reference.
        bool enable = false;
    };

    struct HoverConfig
    {
        // Master toggle for hover.
        bool enable = false;
    };

    struct FoldingRangeConfig
    {
        // Master toggle for folding range.
        bool enable = false;
    };

    struct DocumentSymbolConfig
    {
        // Master toggle for document symbol.
        bool enable = false;
    };

    struct SemanticTokenConfig
    {
        // Master toggle for semantic token.
        bool enable = false;
    };

    struct InlayHintConfig
    {
        // Master toggle for inlay hint.
        bool enable = false;
        // Show the argument name hint for function calls.
        bool enableArgumentNameHint = false;
        // Show the initializer hint for constructor calls and initializer lists.
        bool enableInitializerHint = false;
        // Show the implicit array size hint for array declarations and constructor calls.
        bool enableImplicitArraySizeHint = false;
        // Show the implicit cast hint for function calls.
        bool enableImplicitCastHint = false;
        // Show the block end hint for function body.
        bool enableBlockEndHint = false;

        // The threshold of lines to show the block end hint.
        size_t blockEndHintLineThreshold = 0;
    };

    struct CompletionConfig
    {
        // Master toggle for completion.
        bool enable = false;
    };

    struct DiagnosticConfig
    {
        // Master toggle for diagnostic.
        bool enable = false;
    };

    struct SignatureHelpConfig
    {
        // Master toggle for signature help.
        bool enable = false;
    };

    struct LanguageServiceConfig
    {
        // DeclarationConfig declaration;
        DefinitionConfig definition;
        ReferenceConfig reference;
        HoverConfig hover;
        FoldingRangeConfig foldingRange;
        DocumentSymbolConfig documentSymbol;
        SemanticTokenConfig semanticTokens;
        InlayHintConfig inlayHint;
        CompletionConfig completion;
        DiagnosticConfig diagnostic;
        SignatureHelpConfig signatureHelp;
    };

    enum class LoggingLevel
    {
        Debug,
        Info,
        Warn,
        Error,
    };

    struct LanguageServerConfig
    {
        LanguageServiceConfig languageService;
        StringEnum<LoggingLevel> loggingLevel;
    };

    auto GetDefaultLanguageServerConfig() -> LanguageServerConfig;

    auto ParseLanguageServerConfig(StringView configFile) -> std::optional<LanguageServerConfig>;
} // namespace glsld
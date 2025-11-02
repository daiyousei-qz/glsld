#pragma once

#include "Support/StringView.h"
#include "Support/SerializerUtils.h"

#include <optional>

namespace glsld
{
    struct DeclarationConfig
    {
        // Master toggle for declaration.
        bool enable = true;
    };

    struct DefinitionConfig
    {
        // Master toggle for definition.
        bool enable = true;
    };

    struct ReferenceConfig
    {
        // Master toggle for reference.
        bool enable = true;
    };

    struct HoverConfig
    {
        // Master toggle for hover.
        bool enable = true;
    };

    struct FoldingRangeConfig
    {
        // Master toggle for folding range.
        bool enable = false;
    };

    struct DocumentSymbolConfig
    {
        // Master toggle for document symbol.
        bool enable = true;
    };

    struct SemanticTokenConfig
    {
        // Master toggle for semantic token.
        bool enable = true;
    };

    struct InlayHintConfig
    {
        // Master toggle for inlay hint.
        bool enable = true;
        // Show the argument name hint for function calls.
        bool enableArgumentNameHint = true;
        // Show the implicit cast hint for function calls.
        bool enableImplicitCastHint = false;
        // Show the block end hint for function body.
        bool enableBlockEndHint = true;

        // The threshold of lines to show the block end hint.
        size_t blockEndHintLineThreshold = 0;
    };

    struct CompletionConfig
    {
        // Master toggle for completion.
        bool enable = true;
    };

    struct DiagnosticConfig
    {
        // Master toggle for diagnostic.
        bool enable = false;
    };

    struct SignatureHelpConfig
    {
        // Master toggle for signature help.
        bool enable = true;
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
        StringEnum<LoggingLevel> loggingLevel = LoggingLevel::Info;
    };

    auto ParseLanguageServerConfig(StringView configFile) -> std::optional<LanguageServerConfig>;
} // namespace glsld
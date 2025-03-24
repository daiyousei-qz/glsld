#pragma once

#include "Basic/StringView.h"
#include "Support/SerializerUtils.h"

#include <optional>
#include <string>

namespace glsld
{
    struct DeclarationConfig
    {
    };

    struct DefinitionConfig
    {
    };

    struct ReferenceConfig
    {
    };

    struct HoverConfig
    {
    };

    struct FoldingRangeConfig
    {
    };

    struct DocumentSymbolConfig
    {
    };

    struct SemanticTokenConfig
    {
    };

    struct InlayHintConfig
    {
        bool enableArgumentNameHint = false;
        bool enableImplicitCastHint = false;
        bool enableBlockEndHint     = false;

        // The threshold of lines to show the block end hint.
        size_t blockEndHintLineThreshold = 0;
    };

    struct CompletionConfig
    {
    };

    struct DiagnosticConfig
    {
    };

    struct SignatureHelpConfig
    {
    };

    struct LanguageServiceConfig
    {
        DeclarationConfig declaration;
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
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical,
    };

    struct LanguageServerConfig
    {
        LanguageServiceConfig languageService;
        StringEnum<LoggingLevel> loggingLevel;
    };

    auto ParseLanguageServerConfig(StringView configFile) -> std::optional<LanguageServerConfig>;
} // namespace glsld
#pragma once

#include "ObjectMapper.h"

#include <string>

namespace glsld
{
    struct DeclarationConfig
    {
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, DeclarationConfig& value) -> bool
    {
        return true;
    }

    struct ReferenceConfig
    {
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, ReferenceConfig& value) -> bool
    {
        return true;
    }

    struct HoverConfig
    {
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, HoverConfig& value) -> bool
    {
        return true;
    }

    struct FoldingRangeConfig
    {
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, FoldingRangeConfig& value) -> bool
    {
        return true;
    }

    struct DocumentSymbolConfig
    {
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, DocumentSymbolConfig& value) -> bool
    {
        return true;
    }

    struct SemanticTokenConfig
    {
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, SemanticTokenConfig& value) -> bool
    {
        return true;
    }

    struct InlayHintConfig
    {
        bool enableArgumentNameHint = false;
        bool enableImplicitCastHint = false;
        bool enableBlockEndHint     = false;

        // The threshold of lines to show the block end hint.
        size_t blockEndHintLineThreshold = 0;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, InlayHintConfig& value) -> bool
    {
        return mapper.Map("enableArgumentNameHint", value.enableArgumentNameHint, true) &&
               mapper.Map("enableImplicitCastHint", value.enableImplicitCastHint, true) &&
               mapper.Map("enableBlockEndHint", value.enableBlockEndHint, true) &&
               mapper.Map("blockEndHintLineThreshold", value.blockEndHintLineThreshold, true);
    }

    struct CompletionConfig
    {
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, CompletionConfig& value) -> bool
    {
        return true;
    }

    struct DiagnosticConfig
    {
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, DiagnosticConfig& value) -> bool
    {
        return true;
    }

    struct SignatureHelpConfig
    {
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, SignatureHelpConfig& value) -> bool
    {
        return true;
    }

    struct LanguageServiceConfig
    {
        DeclarationConfig declaration;
        ReferenceConfig reference;
        HoverConfig hover;
        FoldingRangeConfig foldingRange;
        DocumentSymbolConfig documentSymbol;
        SemanticTokenConfig semanticToken;
        InlayHintConfig inlayHint;
        CompletionConfig completion;
        DiagnosticConfig diagnostic;
        SignatureHelpConfig signatureHelp;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, LanguageServiceConfig& value) -> bool
    {
        return mapper.Map("declaration", value.declaration, true) && mapper.Map("reference", value.reference, true) &&
               mapper.Map("hover", value.hover, true) && mapper.Map("foldingRange", value.foldingRange, true) &&
               mapper.Map("documentSymbol", value.documentSymbol, true) &&
               mapper.Map("semanticToken", value.semanticToken, true) &&
               mapper.Map("inlayHint", value.inlayHint, true) && mapper.Map("completion", value.completion, true) &&
               mapper.Map("diagnostic", value.diagnostic, true) &&
               mapper.Map("signatureHelp", value.signatureHelp, true);
    }

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
        LoggingLevel loggingLevel;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, LanguageServerConfig& value) -> bool
    {
        if (!mapper.Map("languageService", value.languageService, true)) {
            return false;
        }

        std::string loggingLevel;
        if (!mapper.Map("loggingLevel", loggingLevel, true)) {
            return false;
        }

        if (loggingLevel == "trace") {
            value.loggingLevel = LoggingLevel::Trace;
        }
        else if (loggingLevel == "debug") {
            value.loggingLevel = LoggingLevel::Debug;
        }
        else if (loggingLevel == "info") {
            value.loggingLevel = LoggingLevel::Info;
        }
        else if (loggingLevel == "warn") {
            value.loggingLevel = LoggingLevel::Warn;
        }
        else if (loggingLevel == "error") {
            value.loggingLevel = LoggingLevel::Error;
        }
        else if (loggingLevel == "critical") {
            value.loggingLevel = LoggingLevel::Critical;
        }
        else {
            return false;
        }

        return true;
    }

    inline auto ParseLanguageServerConfig(StringView configFile) -> std::optional<LanguageServerConfig>
    {
        auto json = ParseJson(configFile);
        if (!json) {
            return std::nullopt;
        }

        LanguageServerConfig result;
        if (JsonSerializer<LanguageServerConfig>::FromJson(*json, result)) {
            return result;
        }

        return std::nullopt;
    }
} // namespace glsld
#pragma once

namespace glsld
{
    struct DeclarationConfig
    {

        static auto Default() -> DeclarationConfig
        {
            return DeclarationConfig{};
        }
    };

    struct ReferenceConfig
    {

        static auto Default() -> ReferenceConfig
        {
            return ReferenceConfig{};
        }
    };

    struct HoverConfig
    {

        static auto Default() -> HoverConfig
        {
            return HoverConfig{};
        }
    };

    struct FoldingRangeConfig
    {

        static auto Default() -> FoldingRangeConfig
        {
            return FoldingRangeConfig{};
        }
    };

    struct DocumentSymbolConfig
    {

        static auto Default() -> DocumentSymbolConfig
        {
            return DocumentSymbolConfig{};
        }
    };

    struct SemanticTokenConfig
    {

        static auto Default() -> SemanticTokenConfig
        {
            return SemanticTokenConfig{};
        }
    };

    struct InlayHintConfig
    {
        bool enableArgumentNameHint = false;
        bool enableImplicitCastHint = false;
        bool enableBlockEndHint     = false;

        // The threshold of lines to show the block end hint.
        size_t blockEndHintLineThreshold = 0;

        static auto Default() -> InlayHintConfig
        {
            return InlayHintConfig{
                .enableArgumentNameHint    = true,
                .enableImplicitCastHint    = true,
                .enableBlockEndHint        = true,
                .blockEndHintLineThreshold = 4,
            };
        }
    };

    struct CompletionConfig
    {

        static auto Default() -> CompletionConfig
        {
            return CompletionConfig{};
        }
    };

    struct DiagnosticConfig
    {

        static auto Default() -> DiagnosticConfig
        {
            return DiagnosticConfig{};
        }
    };

    struct SignatureHelpConfig
    {

        static auto Default() -> SignatureHelpConfig
        {
            return SignatureHelpConfig{};
        }
    };

    struct LanguageServerConfig
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

        static auto Default() -> LanguageServerConfig
        {
            return LanguageServerConfig{
                .declaration    = DeclarationConfig::Default(),
                .reference      = ReferenceConfig::Default(),
                .hover          = HoverConfig::Default(),
                .foldingRange   = FoldingRangeConfig::Default(),
                .documentSymbol = DocumentSymbolConfig::Default(),
                .semanticToken  = SemanticTokenConfig::Default(),
                .inlayHint      = InlayHintConfig::Default(),
                .completion     = CompletionConfig::Default(),
                .diagnostic     = DiagnosticConfig::Default(),
                .signatureHelp  = SignatureHelpConfig::Default(),
            };
        }
    };
} // namespace glsld
#pragma once
#include "LanguageQueryProvider.h"
#include "Protocol.h"

namespace glsld
{
    enum class SemanticTokenType
    {
        Type      = 0,
        Struct    = 1,
        Parameter = 2,
        Variable  = 3,
        Function  = 4,
        Keyword   = 5,
        Comment   = 6,
        Number    = 7,
        Macro     = 8,
        String    = 9,
    };

    enum class SemanticTokenModifier
    {
        None        = 0,
        Readonly    = 1 << 0,
        Declaration = 1 << 1,
    };

    constexpr inline auto operator|(SemanticTokenModifier lhs, SemanticTokenModifier rhs) -> SemanticTokenModifier
    {
        using EnumInt = std::underlying_type_t<SemanticTokenModifier>;
        return static_cast<SemanticTokenModifier>(static_cast<EnumInt>(lhs) | static_cast<EnumInt>(rhs));
    }
    constexpr inline auto operator|=(SemanticTokenModifier& lhs, SemanticTokenModifier rhs) -> SemanticTokenModifier&
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline auto GetTokenTypeIndex(SemanticTokenType type) -> int
    {
        return static_cast<int>(type);
    }

    inline auto GetTokenModifierMask(SemanticTokenModifier modifier) -> int
    {
        return static_cast<int>(modifier);
    }

    inline auto GetTokenLegend() -> lsp::SemanticTokensLegend
    {
        return lsp::SemanticTokensLegend{
            .tokenTypes =
                {
                    "type",
                    "struct",
                    "parameter",
                    "variable",
                    "function",
                    "keyword",
                    "comment",
                    "number",
                    "macro",
                    "string",
                },
            .tokenModifiers =
                {
                    "readonly",
                    "declaration",
                },
        };
    }

    struct SemanticTokenInfo
    {
        int line;
        int character;
        int length;
        SemanticTokenType type;
        SemanticTokenModifier modifier;
    };

    auto ComputeSemanticTokens(const LanguageQueryProvider& provider) -> lsp::SemanticTokens;
    auto ComputeSemanticTokensDelta(const LanguageQueryProvider& provider) -> lsp::SemanticTokensDelta;
} // namespace glsld
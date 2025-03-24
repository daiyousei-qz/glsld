#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld::lsp
{

} // namespace glsld::lsp

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

    struct SemanticTokenInfo
    {
        int line;
        int character;
        int length;
        SemanticTokenType type;
        SemanticTokenModifier modifier;
    };

    auto CollectSemanticTokens(const SemanticTokenConfig& config, const LanguageQueryInfo& info)
        -> std::vector<SemanticTokenInfo>;

    auto GetSemanticTokensOptions(const SemanticTokenConfig& config) -> lsp::SemanticTokensOptions;

    auto HandleSemanticTokens(const SemanticTokenConfig& config, const LanguageQueryInfo& info,
                              const lsp::SemanticTokensParams& params) -> lsp::SemanticTokens;
    auto HandleSemanticTokensDelta(const SemanticTokenConfig& config, const LanguageQueryInfo& info,
                                   const lsp::SemanticTokensDeltaParams& params) -> lsp::SemanticTokensDelta;

} // namespace glsld
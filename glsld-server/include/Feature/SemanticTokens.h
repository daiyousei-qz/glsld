#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"
#include "Support/EnumReflection.h"

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
        Readonly    = 0,
        Declaration = 1,
    };

    using SemanticTokenModifierBits = EnumBitFlags<SemanticTokenModifier>;

    inline auto GetTokenTypeIndex(SemanticTokenType type) -> int
    {
        return static_cast<int>(type);
    }

    struct SemanticTokenInfo
    {
        int line;
        int character;
        int length;
        SemanticTokenType type;
        SemanticTokenModifierBits modifiers;
    };

    auto CollectSemanticTokens(const SemanticTokenConfig& config, const LanguageQueryInfo& info)
        -> std::vector<SemanticTokenInfo>;

    auto GetSemanticTokensOptions(const SemanticTokenConfig& config) -> std::optional<lsp::SemanticTokensOptions>;

    struct SemanticTokenState
    {
        int resultId = 0;
        std::vector<lsp::uinteger> cachedTokens;
    };

    auto HandleSemanticTokens(const SemanticTokenConfig& config, const LanguageQueryInfo& info,
                              SemanticTokenState& state, const lsp::SemanticTokensParams& params)
        -> lsp::SemanticTokens;
    auto HandleSemanticTokensDelta(const SemanticTokenConfig& config, const LanguageQueryInfo& info,
                                   SemanticTokenState& state, const lsp::SemanticTokensDeltaParams& params)
        -> std::variant<lsp::SemanticTokens, lsp::SemanticTokensDelta>;

} // namespace glsld
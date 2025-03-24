#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetInlayHintsOptions(const InlayHintConfig& config) -> lsp::InlayHintOptions;

    auto HandleInlayHints(const InlayHintConfig& config, const LanguageQueryInfo& info,
                          const lsp::InlayHintParams& params) -> std::vector<lsp::InlayHint>;
} // namespace glsld
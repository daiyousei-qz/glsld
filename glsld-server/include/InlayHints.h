#pragma once
#include "Config.h"
#include "LanguageQueryProvider.h"
#include "Protocol.h"

namespace glsld
{
    auto ComputeInlayHint(const LanguageQueryProvider& provider, const InlayHintConfig& config, lsp::Range displayRange)
        -> std::vector<lsp::InlayHint>;
} // namespace glsld
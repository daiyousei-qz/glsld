#pragma once
#include "LanguageQueryProvider.h"
#include "Protocol.h"

namespace glsld
{
    auto ComputeInlayHint(const LanguageQueryProvider& provider, lsp::Range range) -> std::vector<lsp::InlayHint>;
} // namespace glsld
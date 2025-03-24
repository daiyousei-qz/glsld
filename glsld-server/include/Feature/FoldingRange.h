#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetFoldingRangeOptions(const FoldingRangeConfig& config) -> lsp::FoldingRangeOptions
    {
        return {};
    }

    auto HandleFoldingRange(const FoldingRangeConfig& config, const LanguageQueryInfo& info,
                            const lsp::FoldingRangeParams& params) -> std::vector<lsp::FoldingRange>
    {
        return {};
    }

} // namespace glsld
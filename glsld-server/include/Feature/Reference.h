#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetReferenceOptions(const ReferenceConfig& config) -> lsp::ReferenceOptions;

    auto HandleReferences(const ReferenceConfig& config, const LanguageQueryInfo& info,
                          const lsp::ReferenceParams& params) -> std::vector<lsp::Location>;
} // namespace glsld
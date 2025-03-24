#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetCompletionOptions(const CompletionConfig& config) -> std::optional<lsp::CompletionOptions>;

    auto HandleCompletion(const CompletionConfig& config, const LanguageQueryInfo& info,
                          const lsp::CompletionParams& params) -> std::vector<lsp::CompletionItem>;
} // namespace glsld
#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetCompletionOptions(const CompletionConfig& config) -> std::optional<lsp::CompletionOptions>;

    struct CompletionState
    {
        std::shared_ptr<PrecompiledPreamble> preamble            = nullptr;
        std::vector<lsp::CompletionItem> preambleCompletionItems = {};
    };

    auto HandleCompletion(const CompletionConfig& config, const LanguageQueryInfo& queryInfo, CompletionState& state,
                          const lsp::CompletionParams& params) -> lsp::CompletionList;

} // namespace glsld
#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    struct CompletionPreambleInfo
    {
        std::vector<lsp::CompletionItem> builtinCompletionItems;
    };
    auto ComputeCompletionPreambleInfo(const PrecompiledPreamble& preamble) -> std::unique_ptr<CompletionPreambleInfo>;

    auto GetCompletionOptions(const CompletionConfig& config) -> std::optional<lsp::CompletionOptions>;
    auto HandleCompletion(const CompletionConfig& config, const CompletionPreambleInfo& preambleInfo,
                          const LanguageQueryInfo& queryInfo, const lsp::CompletionParams& params)
        -> std::vector<lsp::CompletionItem>;
} // namespace glsld
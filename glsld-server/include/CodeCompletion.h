#pragma once
#include "LanguageQueryProvider.h"
#include "Protocol.h"

namespace glsld
{
    auto ComputeCompletion(const LanguageQueryProvider& provider, lsp::Position position)
        -> std::vector<lsp::CompletionItem>;
}
#pragma once
#include "LanguageQueryProvider.h"
#include "Protocol.h"

namespace glsld
{
    // we assume single source file for now
    auto ComputeDeclaration(const LanguageQueryProvider& provider, const lsp::DocumentUri& uri, lsp::Position position)
        -> std::vector<lsp::Location>;
} // namespace glsld
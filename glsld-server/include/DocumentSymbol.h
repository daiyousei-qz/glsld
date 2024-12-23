#pragma once
#include "LanguageQueryProvider.h"
#include "Protocol.h"

namespace glsld
{
    auto ComputeDocumentSymbol(const LanguageQueryProvider& provider) -> std::vector<lsp::DocumentSymbol>;
}
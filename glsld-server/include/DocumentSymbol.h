#pragma once
#include "LanguageQueryProvider.h"
#include "Protocol.h"

namespace glsld
{
    namespace lsp
    {
        auto ComputeDocumentSymbol(const LanguageQueryProvider& provider) -> std::vector<DocumentSymbol>;
    }
} // namespace glsld
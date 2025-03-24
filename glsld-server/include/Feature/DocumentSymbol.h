#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetDocumentSymbolOptions(const DocumentSymbolConfig& config) -> std::optional<lsp::DocumentSymbolOptions>;

    auto HandleDocumentSymbol(const DocumentSymbolConfig& config, const LanguageQueryInfo& info,
                              const lsp::DocumentSymbolParams& params) -> std::vector<lsp::DocumentSymbol>;
} // namespace glsld
#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetDefinitionOptions(const DefinitionConfig& config) -> std::optional<lsp::DefinitionOptions>;

    auto HandleDefinition(const DefinitionConfig& config, const LanguageQueryInfo& info,
                          const lsp::DefinitionParams& params) -> std::vector<lsp::Location>;

} // namespace glsld
#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetDeclarationOptions(const DeclarationConfig& config) -> lsp::DeclarationOptions;

    // we assume single source file for now
    auto HandleDeclaration(const DeclarationConfig& config, const LanguageQueryInfo& info,
                           const lsp::DeclarationParams& params) -> std::vector<lsp::Location>;

} // namespace glsld
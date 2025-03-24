#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetSignatureHelpOptions(const SignatureHelpConfig& config) -> std::optional<lsp::SignatureHelpOptions>;
    auto HandleSignatureHelp(const SignatureHelpConfig& config, const LanguageQueryInfo& info,
                             const lsp::SignatureHelpParams& params) -> std::optional<lsp::SignatureHelp>;

} // namespace glsld
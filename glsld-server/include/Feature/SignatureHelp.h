#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    auto GetSignatureHelpOptions(const SignatureHelpConfig& config) -> std::optional<lsp::SignatureHelpOptions>;

    struct SignatureHelpState
    {
        std::shared_ptr<PrecompiledPreamble> preamble                                       = nullptr;
        std::unordered_multimap<AtomString, const AstFunctionDecl*> preambleFunctionDeclMap = {};
    };

    auto HandleSignatureHelp(const SignatureHelpConfig& config, const LanguageQueryInfo& queryInfo,
                             SignatureHelpState& state, const lsp::SignatureHelpParams& params)
        -> std::optional<lsp::SignatureHelp>;

} // namespace glsld
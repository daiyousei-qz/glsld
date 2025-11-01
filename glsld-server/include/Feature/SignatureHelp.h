#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    struct SignatureHelpPreambleInfo
    {
        std::unordered_multimap<AtomString, const AstFunctionDecl*> builtinFunctionDeclMap;
    };
    auto ComputeSignatureHelpPreambleInfo(const PrecompiledPreamble& preamble)
        -> std::unique_ptr<SignatureHelpPreambleInfo>;

    auto GetSignatureHelpOptions(const SignatureHelpConfig& config) -> std::optional<lsp::SignatureHelpOptions>;
    auto HandleSignatureHelp(const SignatureHelpConfig& config, const SignatureHelpPreambleInfo& preambleInfo,
                             const LanguageQueryInfo& queryInfo, const lsp::SignatureHelpParams& params)
        -> std::optional<lsp::SignatureHelp>;

} // namespace glsld
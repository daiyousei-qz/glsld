#pragma once
#include "Compiler/CompilerConfig.h"
#include "Server/Config.h"
#include "Server/Protocol.h"
#include "Support/StringView.h"

namespace glsld
{
    auto HandleDiagnostic(const DiagnosticConfig& config, StringView uri, int version,
                          const LanguageConfig& languageConfig, StringView sourceBuffer)
        -> lsp::PublishDiagnosticParams;
} // namespace glsld
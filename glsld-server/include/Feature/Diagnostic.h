#pragma once
#include "Basic/StringView.h"
#include "Server/Config.h"
#include "Server/Protocol.h"
#include "Server/LanguageQueryInfo.h"

namespace glsld
{
    auto HandleDiagnostic(const DiagnosticConfig& config, const LanguageQueryInfo& info, StringView sourceBuffer)
        -> lsp::PublishDiagnosticParams;
} // namespace glsld
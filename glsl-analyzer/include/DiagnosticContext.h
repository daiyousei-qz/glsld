#pragma once
#include "SyntaxToken.h"

#include <vector>
#include <fmt/format.h>

namespace glsld
{
    struct DiagnosticMessage
    {
        AstSyntaxRange range;
        std::string message;
    };

    class DiagnosticContext
    {
    public:
        auto ReportError(AstSyntaxRange range, std::string message) -> void
        {
            // errorStream.push_back({range, std::move(message)});
        }
        auto ReportWarning(AstSyntaxRange range, std::string message) -> void
        {
            // errorStream.push_back({range, std::move(message)});
        }

    private:
        std::vector<DiagnosticMessage> errorStream;
    };
} // namespace glsld
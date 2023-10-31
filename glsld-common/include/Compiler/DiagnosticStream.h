#pragma once
#include "Basic/Common.h"
#include "Compiler/SyntaxToken.h"

#include <vector>

namespace glsld
{
    struct DiagnosticMessage
    {
        AstSyntaxRange range;
        std::string message;
    };

    class DiagnosticStream
    {
    private:
        std::vector<DiagnosticMessage> errorStream;

    public:
        auto ReportError(AstSyntaxRange range, std::string message) -> void
        {
            // errorStream.push_back({range, std::move(message)});
        }
        auto ReportWarning(AstSyntaxRange range, std::string message) -> void
        {
            // errorStream.push_back({range, std::move(message)});
        }
    };
} // namespace glsld
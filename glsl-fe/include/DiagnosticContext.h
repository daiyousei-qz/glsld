#pragma once
#include "SyntaxToken.h"

#include <vector>
#include <fmt/format.h>

namespace glsld
{
    struct DiagnosticMessage
    {
        SyntaxTokenRange range;
        std::string message;
    };

    class DiagnosticContext
    {
    public:
        void ReportError(SyntaxTokenRange range, std::string message)
        {
            // TODO: implement this
        }
        void ReportWarning(SyntaxTokenRange range, std::string message)
        {
            // TODO: implement this
        }

    private:
        std::vector<DiagnosticMessage> errorStream;
    };
} // namespace glsld
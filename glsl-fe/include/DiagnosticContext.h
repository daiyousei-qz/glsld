#pragma once
#include "SyntaxToken.h"

#include <vector>
#include <fmt/format.h>

namespace glsld
{
    struct DiagnosticLocation
    {
        int line;
        int column;
    };

    struct DiagnosticMessage
    {
        SyntaxRange range;
        std::string message;
    };

    class DiagnosticContext
    {
    public:
        void ReportError(DiagnosticLocation range, std::string message)
        {
            // TODO: implement this
            fmt::print("[ERROR@{}:{}] {}\n", range.line + 1, range.column + 1, message);
        }
        void ReportWarning(DiagnosticLocation range, std::string message)
        {
            // TODO: implement this
            fmt::print("[WARNING@{}:{}] {}\n", range.line + 1, range.column + 1, message);
        }

    private:
        std::vector<DiagnosticMessage> errorStream;
    };
} // namespace glsld
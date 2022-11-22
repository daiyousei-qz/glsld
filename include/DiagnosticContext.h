#pragma once
#include "SyntaxToken.h"

#include <vector>
#include <fmt/format.h>

namespace glsld
{
    struct DiagnosticMessage
    {
        SyntaxRange range;
        std::string message;
    };

    class DiagnosticContext
    {
    public:
        void ReportError(SyntaxRange range, std::string message)
        {
            // TODO: implement this
            fmt::print("[ERROR@{}:{}] {}\n", range.begin.GetIndex(), range.end.GetIndex(), message);
        }
        void ReportWarning(SyntaxRange range, std::string message)
        {
            // TODO: implement this
            fmt::print("[WARNING@{}:{}] {}\n", range.begin.GetIndex(), range.end.GetIndex(), message);
        }

    private:
        std::vector<DiagnosticMessage> errorStream;
    };
} // namespace glsld
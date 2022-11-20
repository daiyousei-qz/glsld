#pragma once
#include "SyntaxTree.h"

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
            fmt::print("[{}:{}] {}\n", range.begin.GetIndex(), range.end.GetIndex(), message);
        }

    private:
        std::vector<DiagnosticMessage> errorStream;
    };
} // namespace glsld
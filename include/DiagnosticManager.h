#pragma once
#include "SyntaxTree.h"

namespace glsld
{
    struct DiagnosticMessage
    {
        SyntaxRange range;
        std::string message;
    };

    class DiagnosticManager
    {
    public:
        void ReportError(SyntaxRange range, std::string message)
        {
        }

    private:
        std::vector<DiagnosticMessage> errorStream;
    };
} // namespace glsld
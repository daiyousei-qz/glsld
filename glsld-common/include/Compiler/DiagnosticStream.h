#pragma once
#include "Basic/Common.h"
#include "Compiler/SyntaxToken.h"
#include "SyntaxToken.h"

#include <fmt/base.h>
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
        std::vector<DiagnosticMessage> warningStream;

    public:
        auto ReportError(AstSyntaxRange range, std::string message) -> void
        {
            errorStream.push_back({range, std::move(message)});
        }

        auto ReportWarning(AstSyntaxRange range, std::string message) -> void
        {
            warningStream.push_back({range, std::move(message)});
        }
    };

    class DiagnosticReportor
    {
    private:
        DiagnosticStream& stream;

    public:
        DiagnosticReportor(DiagnosticStream& stream) : stream(stream)
        {
        }

        auto GetStream() -> DiagnosticStream&
        {
            return stream;
        }

        auto ReportError(AstSyntaxRange range, std::string message) -> void
        {
            stream.ReportError(range, std::move(message));
        }

        template <typename... Args>
        auto ReportError(AstSyntaxRange range, fmt::format_string<Args...> format, Args&&... args) -> void
        {
            stream.ReportError(range, fmt::format(format, std::forward<Args>(args)...));
        }

        auto ReportWarning(AstSyntaxRange range, std::string message) -> void
        {
            stream.ReportWarning(range, std::move(message));
        }

        template <typename... Args>
        auto ReportWarning(AstSyntaxRange range, fmt::format_string<Args...> format, Args&&... args) -> void
        {
            stream.ReportWarning(range, fmt::format(format, std::forward<Args>(args)...));
        }

#pragma region PP Diag
        auto UnterminatedMacroExpansion(AstSyntaxRange range) -> void
        {
        }

        auto UnterminatedPPRegion(AstSyntaxRange range) -> void
        {
        }
#pragma endregion

#pragma region Parse Diag

#pragma endregion
    };
} // namespace glsld
#include "Feature/Diagnostic.h"
#include "Server/Protocol.h"

#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"

#include <regex>
#include <sys/types.h>

namespace glsld
{
    // FIXME: move to a better place
    static auto GetShaderStage(StringView path) -> EShLanguage
    {
        if (path.EndWith(".vert"))
            return EShLangVertex;
        if (path.EndWith(".frag"))
            return EShLangFragment;
        if (path.EndWith(".geom"))
            return EShLangGeometry;
        if (path.EndWith(".comp"))
            return EShLangCompute;
        if (path.EndWith(".tesc"))
            return EShLangTessControl;
        if (path.EndWith(".tese"))
            return EShLangTessEvaluation;
        if (path.EndWith(".rgen"))
            return EShLangRayGen;
        if (path.EndWith(".rint"))
            return EShLangIntersect;
        if (path.EndWith(".rahit"))
            return EShLangAnyHit;
        if (path.EndWith(".rchit"))
            return EShLangClosestHit;
        if (path.EndWith(".rmiss"))
            return EShLangMiss;
        if (path.EndWith(".rcall"))
            return EShLangCallable;
        if (path.EndWith(".task"))
            return EShLangTask;
        if (path.EndWith(".mesh"))
            return EShLangMesh;
        return EShLangVertex; // Default
    }

    auto HandleDiagnostic(const DiagnosticConfig& config, const LanguageQueryInfo& info, StringView sourceBuffer)
        -> lsp::PublishDiagnosticParams
    {
        if (!config.enable) {
            return {};
        }

        EShLanguage stage = GetShaderStage(info.GetUri());
        glslang::TShader shader(stage);

        const char* sourceCStr = sourceBuffer.data();
        int sourceLen          = static_cast<int>(sourceBuffer.Size());
        shader.setStringsWithLengths(&sourceCStr, &sourceLen, 1);

        // Configure glslang parser
        int version = 460;
        // TBuiltInResource can be customized based on target environment
        const TBuiltInResource* resources = GetDefaultResources();
        EShMessages messages              = static_cast<EShMessages>(EShMsgRelaxedErrors);

        if (!shader.parse(resources, version, false, messages)) {
            // Parsing failed, diagnostics are in the info log
        }

        std::vector<lsp::Diagnostic> diagnostics;
        std::string infoLog = shader.getInfoLog();
        std::regex re(R"((ERROR|WARNING): (\d+):(\d+): (.*))");
        std::smatch match;
        std::string::const_iterator searchStart(infoLog.cbegin());

        while (std::regex_search(searchStart, infoLog.cend(), match, re)) {
            lsp::DiagnosticSeverity severity =
                (match[1] == "ERROR") ? lsp::DiagnosticSeverity::Error : lsp::DiagnosticSeverity::Warning;
            int line            = std::stoi(match[3]) - 1; // glslang is 1-based, LSP is 0-based
            std::string message = match[4];

            diagnostics.push_back(lsp::Diagnostic{
                .range =
                    lsp::Range{
                        .start = lsp::Position{.line = static_cast<lsp::uinteger>(line), .character = 0},
                        .end   = lsp::Position{.line = static_cast<lsp::uinteger>(line), .character = 255}
                        // Highlight whole line for now
                    },
                .severity = severity,
                .message  = std::move(message),
            });

            searchStart = match.suffix().first;
        }

        return lsp::PublishDiagnosticParams{
            .uri         = info.GetUri().Str(),
            .version     = info.GetVersion(),
            .diagnostics = std::move(diagnostics),
        };
    }
} // namespace glsld
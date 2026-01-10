#include "Feature/Diagnostic.h"
#include "Server/Protocol.h"
#include "Support/StringView.h"

#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"

#include <regex>

namespace glsld
{
    static auto GetGlslangVersion(GlslVersion version) -> int
    {
        return static_cast<int>(version);
    }

    static auto GetGlslangShaderStage(GlslShaderStage stage) -> EShLanguage
    {
        switch (stage) {
        case GlslShaderStage::Vertex:
            return EShLangVertex;
        case GlslShaderStage::Fragment:
            return EShLangFragment;
        case GlslShaderStage::Geometry:
            return EShLangGeometry;
        case GlslShaderStage::Compute:
            return EShLangCompute;
        case GlslShaderStage::TessControl:
            return EShLangTessControl;
        case GlslShaderStage::TessEvaluation:
            return EShLangTessEvaluation;
        case GlslShaderStage::RayGeneration:
            return EShLangRayGen;
        case GlslShaderStage::RayIntersection:
            return EShLangIntersect;
        case GlslShaderStage::RayAnyHit:
            return EShLangAnyHit;
        case GlslShaderStage::RayClosestHit:
            return EShLangClosestHit;
        case GlslShaderStage::RayMiss:
            return EShLangMiss;
        case GlslShaderStage::RayCallable:
            return EShLangCallable;
        case GlslShaderStage::Task:
            return EShLangTask;
        case GlslShaderStage::Mesh:
            return EShLangMesh;
        default:
            return EShLangVertex; // Default
        }
    }

    static auto ParseGlslangDiagnostics(StringView infoLog) -> std::vector<lsp::Diagnostic>
    {
        static const std::regex reGlslangDiagInfo(R"((ERROR|WARNING): (\d+):(\d+): (.*))");

        std::vector<lsp::Diagnostic> diagnostics;
        for (auto match : std::ranges::subrange(std::regex_iterator(infoLog.begin(), infoLog.end(), reGlslangDiagInfo),
                                                std::regex_iterator<StringView::ConstIterator>())) {
            lsp::DiagnosticSeverity severity =
                (match[1] == "ERROR") ? lsp::DiagnosticSeverity::Error : lsp::DiagnosticSeverity::Warning;
            lsp::uinteger line =
                static_cast<lsp::uinteger>(std::stoi(match[3]) - 1); // glslang is 1-based, LSP is 0-based
            std::string message = match[4];

            diagnostics.push_back(lsp::Diagnostic{
                .range =
                    lsp::Range{
                        // Highlight whole line for now
                        .start = lsp::Position{.line = line, .character = 0},
                        .end   = lsp::Position{.line = line, .character = 255},
                    },
                .severity = severity,
                .message  = std::move(message),
            });
        }

        return diagnostics;
    }

    auto HandleDiagnostic(const DiagnosticConfig& config, StringView uri, int version,
                          const LanguageConfig& languageConfig, StringView sourceBuffer) -> lsp::PublishDiagnosticParams
    {
        if (!config.enable) {
            return {};
        }

        // Configure glslang parser
        const int defaultVersion          = GetGlslangVersion(languageConfig.version);
        const EShLanguage stage           = GetGlslangShaderStage(languageConfig.stage);
        const TBuiltInResource* resources = GetDefaultResources();

        glslang::TShader shader(stage);

        const char* sourceCStr = sourceBuffer.data();
        int sourceLen          = static_cast<int>(sourceBuffer.Size());
        shader.setStringsWithLengths(&sourceCStr, &sourceLen, 1);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
        shader.setAutoMapBindings(true);
        shader.setAutoMapLocations(true);
        shader.parse(resources, defaultVersion, false, EShMsgDefault);

        return lsp::PublishDiagnosticParams{
            .uri         = uri.Str(),
            .version     = version,
            .diagnostics = ParseGlslangDiagnostics(shader.getInfoLog()),
        };
    }
} // namespace glsld
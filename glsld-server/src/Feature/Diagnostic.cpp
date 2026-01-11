#include "Feature/Diagnostic.h"
#include "Server/Protocol.h"
#include "Support/StringView.h"

#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"

#include <regex>

namespace glsld
{
    class GlslangLoader
    {
    public:
        GlslangLoader()
        {
            glslang::InitializeProcess();
        }
        ~GlslangLoader()
        {
            glslang::FinalizeProcess();
        }
    };
    static GlslangLoader glslangLoaderInstance;

    static auto GetGlslangVersion(GlslVersion version) -> int
    {
        return static_cast<int>(version);
    }

    static auto GetGlslangProfile(GlslProfile profile) -> EProfile
    {
        switch (profile) {
        case GlslProfile::Core:
            return ECoreProfile;
        case GlslProfile::Compatibility:
            return ECompatibilityProfile;
        case GlslProfile::Es:
            return EEsProfile;
        default:
            return ENoProfile;
        }
    }

    static auto GetGlslangShaderStage(GlslShaderStage stage) -> std::optional<EShLanguage>
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
            return std::nullopt;
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
                        // GLSLANG does not provide a range for the diagnostic, we just highlight the entire line.
                        .start = lsp::Position{.line = line, .character = 0},
                        .end   = lsp::Position{.line = line, .character = std::numeric_limits<lsp::uinteger>::max()},
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
        const int defaultVersion               = GetGlslangVersion(languageConfig.version);
        const EProfile defaultProfile          = GetGlslangProfile(languageConfig.profile);
        const std::optional<EShLanguage> stage = GetGlslangShaderStage(languageConfig.stage);
        const TBuiltInResource* resources      = GetDefaultResources();

        if (!stage) {
            // Cannot perform diagnostic without a valid shader stage
            return {};
        }

        glslang::TShader shader(*stage);

        const char* sourceDataPtr = sourceBuffer.data();
        int sourceLen             = static_cast<int>(sourceBuffer.Size());
        shader.setStringsWithLengths(&sourceDataPtr, &sourceLen, 1);
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_4);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
        shader.setAutoMapBindings(true);
        shader.setAutoMapLocations(true);
        shader.parse(resources, defaultVersion, defaultProfile, false, false, EShMsgDefault);

        return lsp::PublishDiagnosticParams{
            .uri         = uri.Str(),
            .version     = version,
            .diagnostics = ParseGlslangDiagnostics(shader.getInfoLog()),
        };
    }
} // namespace glsld
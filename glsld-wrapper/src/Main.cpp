#include "Basic/Print.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/PPCallback.h"

#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace glsld
{
    static const std::string GLSLD_FLAG_STAGE_VERTEX          = "vertex";
    static const std::string GLSLD_FLAG_STAGE_FRAGMENT        = "fragment";
    static const std::string GLSLD_FLAG_STAGE_COMPUTE         = "compute";
    static const std::string GLSLD_FLAG_STAGE_GEOMETRY        = "geometry";
    static const std::string GLSLD_FLAG_STAGE_TESS_CONTROL    = "tess_control";
    static const std::string GLSLD_FLAG_STAGE_TESS_EVALUATION = "tess_evaluation";
    static const std::string GLSLD_FLAG_STAGE_MESH            = "mesh";
    static const std::string GLSLD_FLAG_STAGE_TASK            = "task";
    static const std::string GLSLD_FLAG_STAGE_RAYGEN          = "raygen";
    static const std::string GLSLD_FLAG_STAGE_ANYHIT          = "anyhit";
    static const std::string GLSLD_FLAG_STAGE_CLOSESTHIT      = "closesthit";
    static const std::string GLSLD_FLAG_STAGE_MISS            = "miss";
    static const std::string GLSLD_FLAG_STAGE_INTERSECTION    = "intersection";
    static const std::string GLSLD_FLAG_STAGE_CALLABLE        = "callable";
    static const std::string GLSLD_FLAG_STAGE_INFER           = "infer";

    namespace
    {
        struct ProgramArgs
        {
            std::string inputFile;
            bool dumpTokens;
            bool dumpAst;
            bool noStdlib;
            std::string stage;
        };
    } // namespace

    static auto ParseArguments(int argc, char** argv) -> ProgramArgs
    {
        using namespace argparse;

        ProgramArgs result;

        ArgumentParser program("glsld-wrapper");
        program.add_argument("input-file").help("Input file to be compiled").required().store_into(result.inputFile);
        program.add_argument("--dump-token")
            .help("Dumps tokens when a translation unit is preprocessed.")
            .flag()
            .default_value(false)
            .store_into(result.dumpTokens);
        program.add_argument("--dump-ast")
            .help("Dumps AST when a translation unit is parsed.")
            .flag()
            .default_value(false)
            .store_into(result.dumpAst);
        program.add_argument("--no-stdlib")
            .help("Don't include standard library module.")
            .flag()
            .default_value(false)
            .store_into(result.noStdlib);
        program.add_argument("--stage")
            .help("Specifies the shader stage of the input file.")
            .choices(GLSLD_FLAG_STAGE_VERTEX, GLSLD_FLAG_STAGE_FRAGMENT, GLSLD_FLAG_STAGE_COMPUTE,
                     GLSLD_FLAG_STAGE_GEOMETRY, GLSLD_FLAG_STAGE_TESS_CONTROL, GLSLD_FLAG_STAGE_TESS_EVALUATION,
                     GLSLD_FLAG_STAGE_MESH, GLSLD_FLAG_STAGE_TASK, GLSLD_FLAG_STAGE_RAYGEN, GLSLD_FLAG_STAGE_ANYHIT,
                     GLSLD_FLAG_STAGE_CLOSESTHIT, GLSLD_FLAG_STAGE_MISS, GLSLD_FLAG_STAGE_INTERSECTION,
                     GLSLD_FLAG_STAGE_CALLABLE, GLSLD_FLAG_STAGE_INFER)
            .default_value(GLSLD_FLAG_STAGE_INFER)
            .store_into(result.stage);
        // TODO: -IXXX -DXXX

        program.parse_args(argc, argv);
        return result;
    }

    class VersionExtensionCollector : public PPCallback
    {
    private:
        CompilerInvocation& compiler;

    public:
        VersionExtensionCollector(CompilerInvocation& compiler) : compiler(compiler)
        {
        }

        auto OnVersionDirective(FileID file, TextRange range, GlslVersion version, GlslProfile profile) -> void override
        {
            compiler.SetGlslVersion(version, profile);
        }

        auto OnExtensionDirective(FileID file, TextRange range, ExtensionId extension, ExtensionBehavior behavior)
            -> void override
        {
            if (behavior == ExtensionBehavior::Enable || behavior == ExtensionBehavior::Require) {
                compiler.EnableExtension(extension);
            }
        }
    };

    static auto ParseShaderStage(const ProgramArgs& args) -> GlslShaderStage
    {
        std::unordered_map<std::string, GlslShaderStage> stageMap = {
            {GLSLD_FLAG_STAGE_VERTEX, GlslShaderStage::Vertex},
            {GLSLD_FLAG_STAGE_FRAGMENT, GlslShaderStage::Fragment},
            {GLSLD_FLAG_STAGE_COMPUTE, GlslShaderStage::Compute},
            {GLSLD_FLAG_STAGE_GEOMETRY, GlslShaderStage::Geometry},
            {GLSLD_FLAG_STAGE_TESS_CONTROL, GlslShaderStage::TessControl},
            {GLSLD_FLAG_STAGE_TESS_EVALUATION, GlslShaderStage::TessEvaluation},
            {GLSLD_FLAG_STAGE_MESH, GlslShaderStage::Mesh},
            {GLSLD_FLAG_STAGE_TASK, GlslShaderStage::Task},
            {GLSLD_FLAG_STAGE_RAYGEN, GlslShaderStage::RayGeneration},
            {GLSLD_FLAG_STAGE_ANYHIT, GlslShaderStage::RayAnyHit},
            {GLSLD_FLAG_STAGE_CLOSESTHIT, GlslShaderStage::RayClosestHit},
            {GLSLD_FLAG_STAGE_MISS, GlslShaderStage::RayMiss},
            {GLSLD_FLAG_STAGE_INTERSECTION, GlslShaderStage::RayIntersection},
            {GLSLD_FLAG_STAGE_CALLABLE, GlslShaderStage::RayCallable},
        };

        if (auto it = stageMap.find(args.stage); it != stageMap.end()) {
            return it->second;
        }

        // FIXME: infer from file name or content
        return GlslShaderStage::Unknown;
    }

    static auto DoMain(ProgramArgs args) -> void
    {
        std::filesystem::path inputFilePath = args.inputFile;

        auto compiler = std::make_unique<CompilerInvocation>();
        if (args.noStdlib) {
            compiler->SetNoStdlib(true);
        }

        compiler->AddIncludePath(inputFilePath.parent_path());
        if (args.dumpTokens) {
            compiler->SetDumpTokens(true);
        }
        if (args.dumpAst) {
            compiler->SetDumpAst(true);
        }

        compiler->SetShaderStage(ParseShaderStage(args));
        compiler->SetMainFileFromFile(args.inputFile);

        VersionExtensionCollector ppCallback{*compiler};
        compiler->ScanVersionAndExtension(&ppCallback);
        compiler->CompileMainFile(nullptr);

        Print("succussfully parsed input file\n");
    }
} // namespace glsld

auto main(int argc, char* argv[]) -> int
{
    glsld::DoMain(glsld::ParseArguments(argc, argv));
    return 0;
}

#include "Basic/Print.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/PPCallback.h"

#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>

namespace glsld
{
    namespace
    {
        struct ProgramArgs
        {
            std::string inputFile;
            bool dumpTokens;
            bool dumpAst;
            bool noStdlib;
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
            .default_value(false)
            .store_into(result.dumpTokens);
        program.add_argument("--dump-ast")
            .help("Dumps AST when a translation unit is parsed.")
            .default_value(false)
            .store_into(result.dumpAst);
        program.add_argument("--no-stdlib")
            .help("Don't include standard library module.")
            .default_value(false)
            .store_into(result.noStdlib);
        // TODO: -IXXX -DXXX
        // TODO: -vs -fs -cs ...

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

        compiler->SetShaderStage(GlslShaderStage ::Unknown);
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

#include "Basic/CommandLine.h"
#include "Basic/Print.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/PPCallback.h"

#include <nlohmann/json.hpp>

namespace glsld
{
    cl::Opt<std::string> inputFile(cl::Positional, cl::Desc("input file"), cl::ValueDesc("input file"));

    cl::Opt<bool> dumpTokens("dump-tokens", cl::Desc("Dumping result of the lexing and preprocessing only."));
    cl::Opt<bool> dumpAst("dump-ast", cl::Desc("Dumping result of the parsing only."));
    cl::Opt<bool> noStdlib("no-stdlib", cl::Desc("Don't link standard library module."));
    // cl::Opt<bool> version("version", cl::Desc("Print the version of glsld-wrapper."));

    // TODO: -IXXX -DXXX
    // TODO: -vs -fs -cs ...

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

    auto DoMain() -> void
    {
        if (!inputFile.HasValue()) {
            Print("need a input file\n");
            return;
        }

        std::filesystem::path inputFilePath = inputFile.GetValue();

        auto compiler = std::make_unique<CompilerInvocation>();
        if (noStdlib.HasValue()) {
            compiler->SetNoStdlib(noStdlib.GetValue());
        }

        compiler->AddIncludePath(inputFilePath.parent_path());
        if (dumpTokens.HasValue()) {
            compiler->SetDumpTokens(dumpTokens.GetValue());
        }
        if (dumpAst.HasValue()) {
            compiler->SetDumpAst(dumpAst.GetValue());
        }

        compiler->SetShaderStage(GlslShaderStage ::Unknown);
        compiler->SetMainFileFromFile(inputFile.GetValue());

        VersionExtensionCollector ppCallback{*compiler};
        compiler->ScanVersionAndExtension(&ppCallback);
        compiler->CompileMainFile(nullptr);

        Print("succussfully parsed input file\n");
    }
} // namespace glsld

auto main(int argc, char* argv[]) -> int
{
    glsld::cl::ParseArguments(argc, argv);
    glsld::DoMain();
    return 0;
}

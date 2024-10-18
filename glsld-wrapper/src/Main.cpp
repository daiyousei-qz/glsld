#include "Basic/CommandLine.h"
#include "Basic/Print.h"
#include "Compiler/CompilerObject.h"

#include <nlohmann/json.hpp>

namespace glsld
{
    cl::Opt<std::string> intputFile(cl::Positional, cl::Desc("input file"), cl::ValueDesc("input file"));

    cl::Opt<bool> dumpTokens("dump-tokens", cl::Desc("Dumping result of the lexing and preprocessing only."));
    cl::Opt<bool> dumpAst("dump-ast", cl::Desc("Dumping result of the parsing only."));
    cl::Opt<bool> noStdlib("no-stdlib", cl::Desc("Don't link standard library module."));
    // cl::Opt<bool> version("version", cl::Desc("Print the version of glsld-wrapper."));

    auto DoMain() -> void
    {
        if (!intputFile.HasValue()) {
            Print("need a input file\n");
            return;
        }

        std::filesystem::path inputFilePath = intputFile.GetValue();

        std::shared_ptr<CompiledPreamble> stdlibPreamble;
        if (!noStdlib.HasValue() || !noStdlib.GetValue()) {
            stdlibPreamble = GetStandardLibraryModule();
        }

        CompilerObject compiler;
        compiler.AddIncludePath(inputFilePath.parent_path());
        if (dumpTokens.HasValue()) {
            compiler.SetDumpTokens(dumpTokens.GetValue());
        }
        if (dumpAst.HasValue()) {
            compiler.SetDumpAst(dumpAst.GetValue());
        }
        compiler.SetPrecompiledPreamble(stdlibPreamble);
        compiler.CompileFromFile(intputFile.GetValue(), nullptr);

        Print("succussfully parsed input file\n");
    }
} // namespace glsld

auto main(int argc, char* argv[]) -> int
{
    glsld::cl::ParseArguments(argc, argv);
    glsld::DoMain();
    return 0;
}

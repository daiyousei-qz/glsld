#include "Basic/CommandLine.h"
#include "Basic/Print.h"
#include "Compiler/CompilerObject.h"
#include "Compiler/PPCallback.h"

#include <nlohmann/json.hpp>

namespace glsld
{
    cl::Opt<std::string> inputFile(cl::Positional, cl::Desc("input file"), cl::ValueDesc("input file"));

    cl::Opt<bool> dumpTokens("dump-tokens", cl::Desc("Dumping result of the lexing and preprocessing only."));
    cl::Opt<bool> dumpAst("dump-ast", cl::Desc("Dumping result of the parsing only."));
    cl::Opt<bool> noStdlib("no-stdlib", cl::Desc("Don't link standard library module."));
    // cl::Opt<bool> version("version", cl::Desc("Print the version of glsld-wrapper."));

    auto DoMain() -> void
    {
        if (!inputFile.HasValue()) {
            Print("need a input file\n");
            return;
        }

        std::filesystem::path inputFilePath = inputFile.GetValue();

        std::shared_ptr<CompiledPreamble> stdlibPreamble;
        if (!noStdlib.HasValue() || !noStdlib.GetValue()) {
            stdlibPreamble = GetStandardLibraryModule();
        }

        std::unique_ptr<CompilerObject> compiler = nullptr;
        if (!noStdlib.HasValue() || !noStdlib.GetValue()) {
            compiler = std::make_unique<CompilerObject>();
        }
        else {
            // FIXME: support compile with no stdlib
            GLSLD_NO_IMPL();
        }

        compiler->AddIncludePath(inputFilePath.parent_path());
        if (dumpTokens.HasValue()) {
            compiler->SetDumpTokens(dumpTokens.GetValue());
        }
        if (dumpAst.HasValue()) {
            compiler->SetDumpAst(dumpAst.GetValue());
        }

        compiler->SetMainFileFromFile(inputFile.GetValue());

        {
            class VersionExtensionCollector : public PPCallback
            {
            private:
                CompilerObject& compilerObject;
                GlslShaderStage stage;

            public:
                VersionExtensionCollector(CompilerObject& compilerObject, GlslShaderStage stage)
                    : compilerObject(compilerObject), stage(stage)
                {
                }

                auto OnVersionDirective(FileID file, TextRange range, GlslVersion version,
                                        GlslProfile profile) -> void override
                {
                    compilerObject.SetTarget({version, profile, stage});
                }

                auto OnExtensionDirective(FileID file, TextRange range, ExtensionId extension,
                                          ExtensionBehavior behavior) -> void override
                {
                    if (behavior == ExtensionBehavior::Enable || behavior == ExtensionBehavior::Require) {
                        compilerObject.EnableExtension(extension);
                    }
                }
            };

            VersionExtensionCollector ppCallback{*compiler, GlslShaderStage::Unknown};
            compiler->ScanVersionAndExtension(&ppCallback);
        }

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

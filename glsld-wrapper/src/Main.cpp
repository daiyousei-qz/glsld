#include "Basic/CommandLine.h"
#include "Compiler/CompilerObject.h"

#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

namespace glsld
{
    cl::Opt<std::string> intputFile(cl::Positional, cl::Desc("input file"), cl::ValueDesc("input file"));

    auto ReadFile(const std::string& fileName) -> std::optional<std::string>
    {
        std::ifstream file(fileName, std::ios::in);
        if (!file) {
            return std::nullopt;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    auto DoMain() -> void
    {
        if (!intputFile.HasValue()) {
            fmt::print("need a input file\n");
            return;
        }

        auto inputData = ReadFile(intputFile.GetValue());
        if (!inputData) {
            fmt::print("failed to read file\n");
        }

        std::filesystem::path inputFilePath = intputFile.GetValue();

        CompilerObject compiler;
        compiler.AddIncludePath(inputFilePath.parent_path());
        // compiler.Compile(*inputData, GetStandardLibraryModule(), nullptr);
        compiler.Compile(*inputData, nullptr, nullptr);

        fmt::print("succussfully parsed input file\n");
    }
} // namespace glsld

auto main(int argc, char* argv[]) -> int
{
    glsld::cl::ParseArguments(argc, argv);
    glsld::DoMain();
    return 0;
}

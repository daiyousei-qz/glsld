#include "CommandLine.h"
#include "Compiler.h"
#include "AstVisitor.h"

#include <string_view>
#include <fstream>
#include <vector>
#include <fmt/format.h>
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
        }
        auto inputData = ReadFile(intputFile.GetValue());
        if (!inputData) {
            fmt::print("failed to read file\n");
        }

        // std::filesystem::directory_entry dir{"e:/Project/glslang/Test2"};
        // for (const auto& entry : std::filesystem::directory_iterator{dir}) {
        //     if (entry.is_regular_file()) {
        //         fmt::print("[] Compiling {}\n", entry.path().string());

        //         auto inputData = ReadFile(entry.path().string());
        //         glsld::Compile(StringView{*inputData});
        //         fmt::print("succussfully parsed input file\n\n");
        //     }
        // }

        CompilerObject compiler;
        compiler.AddIncludePath("E:/Project/glsld/.vscode/");
        compiler.Compile(*inputData);

        fmt::print("succussfully parsed input file\n");
    }
} // namespace glsld

auto main(int argc, char* argv[]) -> int
{
    cl::ParseArguments(argc, argv);
    glsld::DoMain();
    return 0;
}

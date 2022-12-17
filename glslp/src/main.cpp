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

        //         glsld::CompiledModule compiler;
        //         auto inputData = ReadFile(entry.path().string());
        //         compiler.Compile(*inputData);
        //         fmt::print("succussfully parsed input file\n\n");
        //     }
        // }

        auto result = glsld::Compile(StringView{*inputData});

        fmt::print("succussfully parsed input file\n");
    }

    auto CreateLexingAutomata() -> void;
} // namespace glsld

auto main(int argc, char* argv[]) -> int
{
    //     auto testText =
    //         R"(
    // hello
    // world
    // !
    // )";

    //     auto v     = std::vector<SourcePiece>{SourcePiece{.range = std::nullopt, .text = testText}};
    //     auto edit1 = ApplyTextChange(v, TextRange{.start = {1, 0}, .end = {1, 1}}, "hh");
    //     auto edit2 = ApplyTextChange(edit1, TextRange{.start = {2, 1}, .end = {2, 5}}, "iii");
    //     auto x     = ToString(edit2);
    //     fmt::print("{}", x);

    cl::ParseArguments(argc, argv);
    glsld::DoMain();
    return 0;
}

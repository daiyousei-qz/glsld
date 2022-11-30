#include "CommandLine.h"
#include "Compiler.h"
#include "AstVisitor.h"

#include <string_view>
#include <fstream>
#include <vector>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

cl::Opt<std::string> intputFile(cl::Positional, cl::Desc("input file"), cl::ValueDesc("input file"));

// const char* kTestShader = R"(
// in float x;
// in float y;
// out float test;

// void main() {
//     float tmp = 0;
//     if (x > 0) {
//         tmp += x;
//     }
//     if (y < 0) {
//         tmp += z;
//     }

//     test = tmp;
// }
// )";
// const char* kTestShader = R"(
// {
//     if (x & (1 << 2)) {
//         tmp = x;
//     }
//     else if (y < 0) {
//         tmp = z;
//     }

//     test = tmp;
// }
// )";
// const char* testShader = R"(
//     in float x, y;
//     out float z;

//     float foo(const float x, const float y) {
//         return x + y
//     }
//     void main(int x)
//     {
//         if (x & (1 << 2) xxx) {
//             tmp = x;
//         }
//         else if (y < 0) {
//             tmp = z;
//         }

//         test = tmp;
//     }
// )";

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

    glsld::GlsldCompiler compiler;
    compiler.Compile(*inputData);
    fmt::print("succussfully parsed input file\n");
}

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

    // cl::ParseArguments(argc, argv);
    // DoMain();
    return 0;
}

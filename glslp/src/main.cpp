#include "CommandLine.h"
#include "ParseContext.h"

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

// class MyAstVisitor : public glsld::AstVisitor<MyAstVisitor>
// {
// public:
//     auto EnterAstExpr(glsld::AstExpr* expr) -> glsld::AstVisitPolicy
//     {
//         depth += 1;
//         return glsld::AstVisitPolicy::Traverse;
//     }
//     auto ExitAstExpr(glsld::AstExpr* expr) -> void
//     {
//         depth -= 1;
//     }
//     auto VisitAstExpr(glsld::AstExpr* expr) -> void
//     {
//         PrintIdent();
//         fmt::print("{}\n", glsld::ExprOpToString(expr->GetOp()));
//     }

// private:
//     auto PrintIdent() -> void
//     {
//         for (int i = 0; i < depth; ++i) {
//             fmt::print("  ");
//         }
//     }

//     int depth = 0;
// };

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

    glsld::DiagnosticContext diagCtx{};
    glsld::LexContext lexer{*inputData};
    glsld::ParseContext parser{&diagCtx, &lexer};
    parser.DoParseTranslationUnit();
    fmt::print("succussfully parsed input file\n");
}

auto main(int argc, char* argv[]) -> int
{
    cl::ParseArguments(argc, argv);
    DoMain();
    return 0;
}

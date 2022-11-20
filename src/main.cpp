#include "LexContext.h"
#include "Tokenizer.h"
#include "ParseContext.h"
#include "AstVisitor.h"
#include "LanguageServer.h"
#include "Protocol.h"

#include <string_view>
#include <vector>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

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
const char* testShader = R"(
    in float x, y;
    out float z;

    float foo(const float x, const float y) {
        return x + y
    }
    void main(int x)
    {
        if (x & (1 << 2)) {
            tmp = x;
        }
        else if (y < 0) {
            tmp = z;
        }

        test = tmp;
    }
)";

class MyAstVisitor : public glsld::AstVisitor<MyAstVisitor>
{
public:
    auto EnterAstExpr(glsld::AstExpr* expr) -> glsld::AstVisitPolicy
    {
        depth += 1;
        return glsld::AstVisitPolicy::Traverse;
    }
    auto ExitAstExpr(glsld::AstExpr* expr) -> void
    {
        depth -= 1;
    }
    auto VisitAstExpr(glsld::AstExpr* expr) -> void
    {
        PrintIdent();
        fmt::print("{}\n", glsld::ExprOpToString(expr->GetOp()));
    }

private:
    auto PrintIdent() -> void
    {
        for (int i = 0; i < depth; ++i) {
            fmt::print("  ");
        }
    }

    int depth = 0;
};

int main()
{
    glsld::LanguageServer{}.Run();

    auto json = R"(
       {
            "textDocument": {
                "uri": "test_uri",
                "version": 0
            },
            "contentChanges": [
                { "text": "content" }
            ]
       } 
    )"_json;

    glsld::lsp::DidChangeTextDocumentParams c;
    bool success = glsld::lsp::JsonSerializer<glsld::lsp::DidChangeTextDocumentParams>::FromJson(json, c);

    glsld::DiagnosticContext diagCtx{};
    glsld::LexContext lexer{testShader};
    glsld::ParseContext parser{&diagCtx, &lexer};
    parser.DoParseTranslationUnit();

    auto ast = parser.GetAst();
    // MyAstVisitor{}.TraverseInternal(expr);
    return 0;
}

#include "ServerTestFixture.h"

#include "SourceText.h"
#include "DocumentSymbol.h"

using namespace glsld;

TEST_CASE_METHOD(ServerTestFixture, "DocumentSymbol")
{
    auto sourceText = R"(
        struct ^[S.begin]S^[S.end]
        {
            int ^[a.begin]a^[a.end];
            int ^[b.begin]b^[b.end];
        };

        void ^[foo.begin]foo^[foo.end](S s)
        {
        }

        void ^[main.begin]main^[main.end]()
        {
        }
    )";

    auto ctx             = CompileLabelledSource(sourceText);
    auto documentSymbols = ComputeDocumentSymbol(ctx.GetProvider());
    auto checkSymbol     = [&](const lsp::DocumentSymbol& symbol, StringView name, lsp::SymbolKind kind,
                           StringView labelBegin, StringView labelEnd) {
        REQUIRE(symbol.name == name);
        REQUIRE(symbol.kind == kind);
        REQUIRE(FromLspRange(symbol.range) == ctx.GetRange(labelBegin, labelEnd));
    };

    REQUIRE(documentSymbols.size() == 3);
    checkSymbol(documentSymbols[0], "S", lsp::SymbolKind::Struct, "S.begin", "S.end");
    REQUIRE(documentSymbols[0].children.size() == 2);
    checkSymbol(documentSymbols[0].children[0], "a", lsp::SymbolKind::Field, "a.begin", "a.end");
    checkSymbol(documentSymbols[0].children[1], "b", lsp::SymbolKind::Field, "b.begin", "b.end");
    checkSymbol(documentSymbols[1], "foo", lsp::SymbolKind::Function, "foo.begin", "foo.end");
    checkSymbol(documentSymbols[2], "main", lsp::SymbolKind::Function, "main.begin", "main.end");
}
#include "ServerTestFixture.h"

#include "Feature/DocumentSymbol.h"
#include "Support/SourceText.h"

using namespace glsld;

static auto MockDocumentSymbol(const ServerTestFixture& fixture, const DocumentSymbolConfig& config = {.enable = true})
    -> std::vector<lsp::DocumentSymbol>
{
    return HandleDocumentSymbol(config, fixture.GetLanguageQueryInfo(),
                                lsp::DocumentSymbolParams{
                                    .textDocument = {"MockDocument"},
                                });
}

TEST_CASE_METHOD(ServerTestFixture, "DocumentSymbolTest")
{
    auto checkSymbol = [this](const lsp::DocumentSymbol& symbol, StringView name, lsp::SymbolKind kind,
                              StringView labelBegin, StringView labelEnd) {
        REQUIRE(symbol.name == name);
        REQUIRE(symbol.kind == kind);
        REQUIRE(FromLspRange(symbol.range) == GetLabelledRange(labelBegin, labelEnd));
    };

    SECTION("Config")
    {
        CompileLabelledSource(R"(
            void foo()
            {
            }
        )");
        auto documentSymbols = MockDocumentSymbol(*this, DocumentSymbolConfig{.enable = false});

        REQUIRE(documentSymbols.empty());
    }

    SECTION("Macro")
    {
        CompileLabelledSource(R"(
            #define ^[MACRO.begin]MACRO^[MACRO.end]
            #define ^[ID.begin]ID^[ID.end](X) X
            #define ^[ADD.begin]ADD^[ADD.end](X, Y) ((X) + (Y))
        )");
        auto documentSymbols = MockDocumentSymbol(*this);

        REQUIRE(documentSymbols.size() == 3);
        checkSymbol(documentSymbols[0], "MACRO", lsp::SymbolKind::String, "MACRO.begin", "MACRO.end");
        checkSymbol(documentSymbols[1], "ID", lsp::SymbolKind::String, "ID.begin", "ID.end");
        checkSymbol(documentSymbols[2], "ADD", lsp::SymbolKind::String, "ADD.begin", "ADD.end");
    }

    SECTION("Function")
    {
        CompileLabelledSource(R"(
            void ^[baz.begin]baz^[baz.end](float b);

            void ^[foo.begin]foo^[foo.end]()
            {
            }

            void ^[bar.begin]bar^[bar.end](int a)
            {
            }

            void ^[baz.2.begin]baz^[baz.2.end](float b) { }
        )");
        auto documentSymbols = MockDocumentSymbol(*this);

        REQUIRE(documentSymbols.size() == 4);
        checkSymbol(documentSymbols[0], "baz", lsp::SymbolKind::Function, "baz.begin", "baz.end");
        checkSymbol(documentSymbols[1], "foo", lsp::SymbolKind::Function, "foo.begin", "foo.end");
        checkSymbol(documentSymbols[2], "bar", lsp::SymbolKind::Function, "bar.begin", "bar.end");
        checkSymbol(documentSymbols[3], "baz", lsp::SymbolKind::Function, "baz.2.begin", "baz.2.end");
    }

    SECTION("Struct")
    {
        CompileLabelledSource(R"(
            struct ^[S.begin]S^[S.end]
            {
                int ^[a.begin]a^[a.end];
                int ^[b.begin]b^[b.end][2];
            };

            struct ^[T.begin]T^[T.end]
            {
                float ^[c.begin]c^[c.end];
                float ^[d.begin]d^[d.end], ^[e.begin]e^[e.end];
            } test;
        )");
        auto documentSymbols = MockDocumentSymbol(*this);

        REQUIRE(documentSymbols.size() == 3);
        checkSymbol(documentSymbols[0], "S", lsp::SymbolKind::Struct, "S.begin", "S.end");
        REQUIRE(documentSymbols[0].children.size() == 2);
        checkSymbol(documentSymbols[0].children[0], "a", lsp::SymbolKind::Field, "a.begin", "a.end");
        checkSymbol(documentSymbols[0].children[1], "b", lsp::SymbolKind::Field, "b.begin", "b.end");
        checkSymbol(documentSymbols[1], "T", lsp::SymbolKind::Struct, "T.begin", "T.end");
        REQUIRE(documentSymbols[1].children.size() == 3);
        checkSymbol(documentSymbols[1].children[0], "c", lsp::SymbolKind::Field, "c.begin", "c.end");
        checkSymbol(documentSymbols[1].children[1], "d", lsp::SymbolKind::Field, "d.begin", "d.end");
        checkSymbol(documentSymbols[1].children[2], "e", lsp::SymbolKind::Field, "e.begin", "e.end");
    }

    SECTION("Variable")
    {
        CompileLabelledSource(R"(
            int ^[a.begin]a^[a.end], ^[b.begin]b^[b.end] = 1;

            struct S
            {
                int b;
            } ^[s.begin]s^[s.end];
        )");
        auto documentSymbols = MockDocumentSymbol(*this);

        REQUIRE(documentSymbols.size() == 4);
        checkSymbol(documentSymbols[0], "a", lsp::SymbolKind::Variable, "a.begin", "a.end");
        checkSymbol(documentSymbols[1], "b", lsp::SymbolKind::Variable, "b.begin", "b.end");
        checkSymbol(documentSymbols[3], "s", lsp::SymbolKind::Variable, "s.begin", "s.end");
    }

    SECTION("Block")
    {
        CompileLabelledSource(R"(
            buffer SSBO {
                int ^[x.begin]x^[x.end];
            } ^[ssbo.begin]ssbo^[ssbo.end];

            uniform UBO {
                int ^[y.begin]y^[y.end];
            };
        )");
        auto documentSymbols = MockDocumentSymbol(*this);

        REQUIRE(documentSymbols.size() == 2);
        checkSymbol(documentSymbols[0], "ssbo", lsp::SymbolKind::Variable, "ssbo.begin", "ssbo.end");
        REQUIRE(documentSymbols[0].children.size() == 1);
        checkSymbol(documentSymbols[0].children[0], "x", lsp::SymbolKind::Field, "x.begin", "x.end");
        checkSymbol(documentSymbols[1], "y", lsp::SymbolKind::Variable, "y.begin", "y.end");
    }
}
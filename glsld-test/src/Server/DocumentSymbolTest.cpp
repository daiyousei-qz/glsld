#include "ServerTestFixture.h"

#include "Feature/DocumentSymbol.h"
#include "Support/SourceText.h"

using namespace glsld;

static auto MockDocumentSymbol(const ServerTestFixture& fixture, const DocumentSymbolConfig& config = {})
    -> std::vector<lsp::DocumentSymbol>
{
    return HandleDocumentSymbol(config, fixture.GetLanguageQueryInfo(), {});
}

TEST_CASE_METHOD(ServerTestFixture, "DocumentSymbolTest")
{
    auto checkSymbol = [&](const lsp::DocumentSymbol& symbol, StringView name, lsp::SymbolKind kind,
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
        )");
        auto documentSymbols = MockDocumentSymbol(*this);

        REQUIRE(documentSymbols.size() == 2);
        checkSymbol(documentSymbols[0], "MACRO", lsp::SymbolKind::String, "MACRO.begin", "MACRO.end");
        checkSymbol(documentSymbols[1], "ID", lsp::SymbolKind::String, "ID.begin", "ID.end");
    }

    SECTION("Function")
    {
        CompileLabelledSource(R"(
            void ^[foo.begin]foo^[foo.end]()
            {
            }

            void ^[bar.begin]bar^[bar.end](int a)
            {
            }
        )");
        auto documentSymbols = MockDocumentSymbol(*this);

        REQUIRE(documentSymbols.size() == 2);
        checkSymbol(documentSymbols[0], "foo", lsp::SymbolKind::Function, "foo.begin", "foo.end");
        checkSymbol(documentSymbols[1], "bar", lsp::SymbolKind::Function, "bar.begin", "bar.end");
    }

    SECTION("Variable")
    {
        CompileLabelledSource(R"(
            int ^[a.begin]a^[a.end];

            struct ^[S.begin]S^[S.end]
            {
                int ^[b.begin]b^[b.end];
            } ^[s.begin]s^[s.end];
        )");
        auto documentSymbols = MockDocumentSymbol(*this);

        REQUIRE(documentSymbols.size() == 3);
        checkSymbol(documentSymbols[0], "a", lsp::SymbolKind::Variable, "a.begin", "a.end");
        checkSymbol(documentSymbols[1], "S", lsp::SymbolKind::Struct, "S.begin", "S.end");
        REQUIRE(documentSymbols[1].children.size() == 1);
        checkSymbol(documentSymbols[1].children[0], "b", lsp::SymbolKind::Field, "b.begin", "b.end");
        checkSymbol(documentSymbols[2], "s", lsp::SymbolKind::Variable, "s.begin", "s.end");
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
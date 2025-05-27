#include "ServerTestFixture.h"

#include "Feature/InlayHint.h"
#include "Support/SourceText.h"

using namespace glsld;

static auto MockInlayHints(const ServerTestFixture& fixture, TextRange range, const InlayHintConfig& config = {})
    -> std::vector<lsp::InlayHint>
{
    return HandleInlayHints(config, fixture.GetLanguageQueryInfo(),
                            lsp::InlayHintParams{
                                .textDocument =
                                    lsp::TextDocumentIdentifier{
                                        .uri = "mockuri",
                                    },
                                .range = ToLspRange(range),
                            });
}

TEST_CASE_METHOD(ServerTestFixture, "InlayHints")
{
    SECTION("Config")
    {
        SECTION("Disabled")
        {
            CompileLabelledSource(R"(
                void foo(int x)
                {
                    float a = 1;
                }
            )");
            auto config = InlayHintConfig{.enable = false};

            auto hints = MockInlayHints(*this, GetLabelledRange("source.begin", "source.end"), config);
            REQUIRE(hints.empty());
        }

        SECTION("DisplayRange")
        {
            CompileLabelledSource(R"(
            ^[source.begin]
            void foo(int x)
            ^[source.mid1]
            {
                float a = /* hint */1;
            } /* hint */
            ^[source.mid2]
            void bar()
            {
                foo(/* hint */41);
            } /* hint */
            ^[source.end]
        )");
            auto config = InlayHintConfig{
                .enableArgumentNameHint    = true,
                .enableImplicitCastHint    = true,
                .enableBlockEndHint        = true,
                .blockEndHintLineThreshold = 4,
            };

            {
                auto hints = MockInlayHints(*this, GetLabelledRange("source.begin", "source.mid1"), config);
                REQUIRE(hints.size() == 0);
            }

            {
                auto hints = MockInlayHints(*this, GetLabelledRange("source.begin", "source.mid2"), config);
                REQUIRE(hints.size() == 2);
            }

            {
                auto hints = MockInlayHints(*this, GetLabelledRange("source.begin", "source.end"), config);
                REQUIRE(hints.size() == 4);
            }
        }
    }

    SECTION("ArgumentNameHint")
    {
        CompileLabelledSource(R"(
            ^[source.begin]
            void foo(int a, out int b, inout int c) {
                b = a + c;
            }

            void bar() {
                int x;
                foo(^[arg1]1, ^[arg2]2, ^[arg3]x);
            }
            ^[source.end]
        )");
        auto config = InlayHintConfig{
            .enableArgumentNameHint = true,
            .enableImplicitCastHint = false,
            .enableBlockEndHint     = false,
        };

        auto hints = MockInlayHints(*this, GetLabelledRange("source.begin", "source.end"), config);
        REQUIRE(hints.size() == 3);
        REQUIRE(hints[0].label == "a:");
        REQUIRE(FromLspPosition(hints[0].position) == GetLabelledPosition("arg1"));
        REQUIRE(hints[1].label == "&b:");
        REQUIRE(FromLspPosition(hints[1].position) == GetLabelledPosition("arg2"));
        REQUIRE(hints[2].label == "&c:");
        REQUIRE(FromLspPosition(hints[2].position) == GetLabelledPosition("arg3"));
    }

    SECTION("ImplicitCastHint")
    {
        CompileLabelledSource(R"(
            ^[source.begin]
            void foo(float x) {
                double y = ^[hint.vardecl]x;
                double z = x + ^[hint.binary.expr]1;
                double w = x > 0 ? x : ^[hint.select.expr]0;
            }

            void bar() {
                foo(^[hint.call.arg]1);
                vec2 v = {^[hint.init1]1, ^[hint.init2]2};
            }
            ^[source.end]
        )");
        auto config = InlayHintConfig{
            .enableArgumentNameHint = false,
            .enableImplicitCastHint = true,
            .enableBlockEndHint     = false,
        };

        auto hints = MockInlayHints(*this, GetLabelledRange("source.begin", "source.end"), config);
        REQUIRE(hints.size() == 6);
        REQUIRE(hints[0].label == "(double)");
        REQUIRE(FromLspPosition(hints[0].position) == GetLabelledPosition("hint.vardecl"));
        REQUIRE(hints[1].label == "(float)");
        REQUIRE(FromLspPosition(hints[1].position) == GetLabelledPosition("hint.binary.expr"));
        REQUIRE(hints[2].label == "(float)");
        REQUIRE(FromLspPosition(hints[2].position) == GetLabelledPosition("hint.select.expr"));
        REQUIRE(hints[3].label == "(float)");
        REQUIRE(FromLspPosition(hints[3].position) == GetLabelledPosition("hint.call.arg"));
        REQUIRE(hints[4].label == "(float)");
        REQUIRE(FromLspPosition(hints[4].position) == GetLabelledPosition("hint.init1"));
        REQUIRE(hints[5].label == "(float)");
        REQUIRE(FromLspPosition(hints[5].position) == GetLabelledPosition("hint.init2"));
    }

    SECTION("BlockEndHint")
    {
        CompileLabelledSource(R"(
            ^[source.begin]
            // one-liner
            void foo() { }^[foo.end]

            // two-liner
            void bar() {
            }^[bar.end]

            // three-liner
            void baz() {

            }^[baz.end]

            ^[source.end]
        )");

        {
            auto config = InlayHintConfig{
                .enableBlockEndHint        = true,
                .blockEndHintLineThreshold = 0,
            };

            auto hints = MockInlayHints(*this, GetLabelledRange("source.begin", "source.end"), config);
            REQUIRE(hints.size() == 3);
            REQUIRE(hints[0].label == "// foo");
            REQUIRE(FromLspPosition(hints[0].position) == GetLabelledPosition("foo.end"));
            REQUIRE(hints[1].label == "// bar");
            REQUIRE(FromLspPosition(hints[1].position) == GetLabelledPosition("bar.end"));
            REQUIRE(hints[2].label == "// baz");
            REQUIRE(FromLspPosition(hints[2].position) == GetLabelledPosition("baz.end"));
        }

        {
            auto config = InlayHintConfig{
                .enableBlockEndHint        = true,
                .blockEndHintLineThreshold = 3,
            };

            auto hints = MockInlayHints(*this, GetLabelledRange("source.begin", "source.end"), config);
            REQUIRE(hints.size() == 1);
            REQUIRE(hints[0].label == "// baz");
            REQUIRE(FromLspPosition(hints[0].position) == GetLabelledPosition("baz.end"));
        }
    }
}

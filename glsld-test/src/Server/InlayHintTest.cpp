#include "ServerTestFixture.h"

#include "InlayHints.h"
#include "SourceText.h"

using namespace glsld;

TEST_CASE_METHOD(ServerTestFixture, "InlayHints")
{
    SECTION("DisplayRange")
    {
        auto sourceText = R"(
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
    )";

        auto ctx    = CompileLabelledSource(sourceText);
        auto config = InlayHintConfig{
            .enableArgumentNameHint    = true,
            .enableImplicitCastHint    = true,
            .enableBlockEndHint        = true,
            .blockEndHintLineThreshold = 4,
        };

        {
            auto hints =
                ComputeInlayHint(ctx.GetProvider(), config, ToLspRange(ctx.GetRange("source.begin", "source.mid1")));
            REQUIRE(hints.size() == 0);
        }

        {
            auto hints =
                ComputeInlayHint(ctx.GetProvider(), config, ToLspRange(ctx.GetRange("source.begin", "source.mid2")));
            REQUIRE(hints.size() == 2);
        }

        {
            auto hints =
                ComputeInlayHint(ctx.GetProvider(), config, ToLspRange(ctx.GetRange("source.begin", "source.end")));
            REQUIRE(hints.size() == 4);
        }
    }

    SECTION("ArgumentNameHint")
    {
        StringView sourceText = R"(
        ^[source.begin]
        void foo(int a, out int b, inout int c) {
            b = a + c;
        }

        void bar() {
            int x;
            foo(^[arg1]1, ^[arg2]2, ^[arg3]x);
        }
        ^[source.end]
    )";

        auto ctx    = CompileLabelledSource(sourceText);
        auto config = InlayHintConfig{
            .enableArgumentNameHint = true,
        };

        auto hints =
            ComputeInlayHint(ctx.GetProvider(), config, ToLspRange(ctx.GetRange("source.begin", "source.end")));
        REQUIRE(hints.size() == 3);
        REQUIRE(hints[0].label == "a:");
        REQUIRE(FromLspPosition(hints[0].position) == ctx.GetPosition("arg1"));
        REQUIRE(hints[1].label == "&b:");
        REQUIRE(FromLspPosition(hints[1].position) == ctx.GetPosition("arg2"));
        REQUIRE(hints[2].label == "&c:");
        REQUIRE(FromLspPosition(hints[2].position) == ctx.GetPosition("arg3"));
    }

    SECTION("ImplicitCastHint")
    {
        StringView sourceText = R"(
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
    )";

        auto ctx    = CompileLabelledSource(sourceText);
        auto config = InlayHintConfig{
            .enableImplicitCastHint = true,
        };

        auto hints =
            ComputeInlayHint(ctx.GetProvider(), config, ToLspRange(ctx.GetRange("source.begin", "source.end")));
        REQUIRE(hints.size() == 6);
        REQUIRE(hints[0].label == "(double)");
        REQUIRE(FromLspPosition(hints[0].position) == ctx.GetPosition("hint.vardecl"));
        REQUIRE(hints[1].label == "(float)");
        REQUIRE(FromLspPosition(hints[1].position) == ctx.GetPosition("hint.binary.expr"));
        REQUIRE(hints[2].label == "(float)");
        REQUIRE(FromLspPosition(hints[2].position) == ctx.GetPosition("hint.select.expr"));
        REQUIRE(hints[3].label == "(float)");
        REQUIRE(FromLspPosition(hints[3].position) == ctx.GetPosition("hint.call.arg"));
        REQUIRE(hints[4].label == "(float)");
        REQUIRE(FromLspPosition(hints[4].position) == ctx.GetPosition("hint.init1"));
        REQUIRE(hints[5].label == "(float)");
        REQUIRE(FromLspPosition(hints[5].position) == ctx.GetPosition("hint.init2"));
    }

    SECTION("BlockEndHint")
    {
        StringView sourceText = R"(
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
    )";

        auto ctx = CompileLabelledSource(sourceText);

        {
            auto config = InlayHintConfig{
                .enableBlockEndHint        = true,
                .blockEndHintLineThreshold = 0,
            };

            auto hints =
                ComputeInlayHint(ctx.GetProvider(), config, ToLspRange(ctx.GetRange("source.begin", "source.end")));
            REQUIRE(hints.size() == 3);
            REQUIRE(hints[0].label == "// foo");
            REQUIRE(FromLspPosition(hints[0].position) == ctx.GetPosition("foo.end"));
            REQUIRE(hints[1].label == "// bar");
            REQUIRE(FromLspPosition(hints[1].position) == ctx.GetPosition("bar.end"));
            REQUIRE(hints[2].label == "// baz");
            REQUIRE(FromLspPosition(hints[2].position) == ctx.GetPosition("baz.end"));
        }

        {
            auto config = InlayHintConfig{
                .enableBlockEndHint        = true,
                .blockEndHintLineThreshold = 3,
            };

            auto hints =
                ComputeInlayHint(ctx.GetProvider(), config, ToLspRange(ctx.GetRange("source.begin", "source.end")));
            REQUIRE(hints.size() == 1);
            REQUIRE(hints[0].label == "// baz");
            REQUIRE(FromLspPosition(hints[0].position) == ctx.GetPosition("baz.end"));
        }
    }
}

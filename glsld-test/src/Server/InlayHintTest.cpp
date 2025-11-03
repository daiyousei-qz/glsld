#include "ServerTestFixture.h"

#include "Feature/InlayHint.h"
#include "Support/SourceText.h"
#include "Support/StringView.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <optional>

using namespace glsld;

static auto MockInlayHints(const ServerTestFixture& fixture, TextRange range,
                           const InlayHintConfig& config = {.enable = true}) -> std::vector<lsp::InlayHint>
{
    return HandleInlayHints(config, fixture.GetLanguageQueryInfo(),
                            lsp::InlayHintParams{
                                .textDocument = {"MockDocument"},
                                .range        = ToLspRange(range),
                            });
}

struct OneInlayHintExpectedResult
{
    StringView positionLabel;

    std::optional<StringView> labelText = std::nullopt;

    std::optional<bool> paddingLeft = std::nullopt;

    std::optional<bool> paddingRight = std::nullopt;
};

struct InlayHintsExpectedResult
{
    std::optional<size_t> numberOfHints = std::nullopt;

    std::vector<OneInlayHintExpectedResult> hints;
};

TEST_CASE_METHOD(ServerTestFixture, "InlayHints")
{
    auto checkInlayHints = [this](StringView labelBegin, StringView labelEnd, InlayHintsExpectedResult expectedResult,
                                  const InlayHintConfig& config = {.enable = true}) {
        auto hints = MockInlayHints(*this, GetLabelledRange(labelBegin, labelEnd), config);
        if (expectedResult.numberOfHints.has_value()) {
            REQUIRE(hints.size() == expectedResult.numberOfHints.value());
        }

        for (const auto& expectedHint : expectedResult.hints) {
            auto it = std::ranges::find_if(hints, [&](const lsp::InlayHint& hint) {
                return FromLspPosition(hint.position) == GetLabelledPosition(expectedHint.positionLabel);
            });

            REQUIRE(it != hints.end());
            if (expectedHint.labelText.has_value()) {
                REQUIRE(it->label == expectedHint.labelText.value());
            }
            if (expectedHint.paddingLeft.has_value()) {
                REQUIRE(it->paddingLeft == expectedHint.paddingLeft.value());
            }
            if (expectedHint.paddingRight.has_value()) {
                REQUIRE(it->paddingRight == expectedHint.paddingRight.value());
            }
        }
    };

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

            const auto config = InlayHintConfig{.enable = false};
            checkInlayHints("source.begin", "source.end", InlayHintsExpectedResult{.numberOfHints = 0}, config);
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
            const auto config = InlayHintConfig{
                .enable                      = true,
                .enableArgumentNameHint      = true,
                .enableInitializerHint       = true,
                .enableImplicitArraySizeHint = true,
                .enableImplicitCastHint      = true,
                .enableBlockEndHint          = true,
                .blockEndHintLineThreshold   = 4,
            };

            checkInlayHints("source.begin", "source.mid1", InlayHintsExpectedResult{.numberOfHints = 0}, config);
            checkInlayHints("source.begin", "source.mid2", InlayHintsExpectedResult{.numberOfHints = 2}, config);
            checkInlayHints("source.begin", "source.end", InlayHintsExpectedResult{.numberOfHints = 4}, config);
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
        const auto config = InlayHintConfig{
            .enable                 = true,
            .enableArgumentNameHint = true,
        };

        checkInlayHints("source.begin", "source.end",
                        InlayHintsExpectedResult{.numberOfHints = 3,
                                                 .hints =
                                                     {
                                                         {.positionLabel = "arg1", .labelText = "a:"},
                                                         {.positionLabel = "arg2", .labelText = "&b:"},
                                                         {.positionLabel = "arg3", .labelText = "&c:"},
                                                     }},
                        config);
    }

    SECTION("InitializerHint")
    {
        const auto config = InlayHintConfig{
            .enable                = true,
            .enableInitializerHint = true,
        };

        SECTION("InitializerListOnVector")
        {
            CompileLabelledSource(R"(
                ^[source.begin]
                void foo() {
                    vec3 v = {^[v.init.x]1, ^[v.init.y]2, ^[v.init.z]3, /* error */4};
                    vec3 w = {^[w.init.x]5 /* error */};
                }
                ^[source.end]
            )");

            checkInlayHints("source.begin", "source.end",
                            InlayHintsExpectedResult{.numberOfHints = 4,
                                                     .hints =
                                                         {
                                                             {.positionLabel = "v.init.x", .labelText = ".x:"},
                                                             {.positionLabel = "v.init.y", .labelText = ".y:"},
                                                             {.positionLabel = "v.init.z", .labelText = ".z:"},
                                                             {.positionLabel = "w.init.x", .labelText = ".x:"},
                                                         }},
                            config);
        }

        SECTION("InitializerListOnMatrix")
        {
            CompileLabelledSource(R"(
                ^[source.begin]
                void foo() {
                    mat2 m = { ^[m.column.0]{1, 0}, ^[m.column.1]{0, 1}, /* error */{0, 0} };
                }
                ^[source.end]
            )");

            checkInlayHints("source.begin", "source.end",
                            InlayHintsExpectedResult{.numberOfHints = 6,
                                                     .hints =
                                                         {
                                                             {.positionLabel = "m.column.0", .labelText = "[0]:"},
                                                             {.positionLabel = "m.column.1", .labelText = "[1]:"},
                                                         }},
                            config);
        }

        SECTION("InitializerListOnArray")
        {
            CompileLabelledSource(R"(
                ^[source.begin]
                void foo() {
                    int arr[2] = {^[arr.init.0]4, ^[arr.init.1]5, /* error */6};
                }
                ^[source.end]
            )");

            checkInlayHints("source.begin", "source.end",
                            InlayHintsExpectedResult{.numberOfHints = 2,
                                                     .hints =
                                                         {
                                                             {.positionLabel = "arr.init.0", .labelText = "[0]:"},
                                                             {.positionLabel = "arr.init.1", .labelText = "[1]:"},
                                                         }},
                            config);
        }

        SECTION("InitializerListOnStruct")
        {
            CompileLabelledSource(R"(
                ^[source.begin]
                struct S {
                    float xx;
                    float yy;
                    float zz;
                };

                void foo() {
                    S s = {^[s.init.xx]1, ^[s.init.yy]2, ^[s.init.zz]3, /* error */4};
                }
                ^[source.end]
            )");

            checkInlayHints("source.begin", "source.end",
                            InlayHintsExpectedResult{.numberOfHints = 3,
                                                     .hints =
                                                         {
                                                             {.positionLabel = "s.init.xx", .labelText = ".xx:"},
                                                             {.positionLabel = "s.init.yy", .labelText = ".yy:"},
                                                             {.positionLabel = "s.init.zz", .labelText = ".zz:"},
                                                         }},
                            config);
        }

        SECTION("ConstructorOnVector")
        {
            CompileLabelledSource(R"(
                ^[source.begin]
                void foo() {
                    vec2 v = vec2(^[v.init.x]1, ^[v.init.y]2, /* error */3);
                    vec3 w = vec3(^[w.init.xyz]4);
                    vec3 u = vec3(^[u.init.xy]v, ^[u.init.z]5);
                }
                ^[source.end]
            )");

            checkInlayHints("source.begin", "source.end",
                            InlayHintsExpectedResult{.numberOfHints = 5,
                                                     .hints =
                                                         {
                                                             {.positionLabel = "v.init.x", .labelText = ".x:"},
                                                             {.positionLabel = "v.init.y", .labelText = ".y:"},
                                                             {.positionLabel = "w.init.xyz", .labelText = ".xyz:"},
                                                             {.positionLabel = "u.init.xy", .labelText = ".xy:"},
                                                             {.positionLabel = "u.init.z", .labelText = ".z:"},
                                                         }},
                            config);
        }

        SECTION("ConstructorOnMatrix")
        {
            CompileLabelledSource(R"(
                ^[source.begin]
                void foo() {
                    mat4 m1 = mat4(^[m1.diag]1.0);
                    mat3 m2 = mat3(^[m2.m]m1);
                }
                ^[source.end]
            )");

            checkInlayHints("source.begin", "source.end",
                            InlayHintsExpectedResult{.numberOfHints = 2,
                                                     .hints =
                                                         {
                                                             {.positionLabel = "m1.diag", .labelText = "diag:"},
                                                             {.positionLabel = "m2.m", .labelText = "m:"},
                                                         }},
                            config);
        }

        SECTION("ConstructorOnArray")
        {
            CompileLabelledSource(R"(
                ^[source.begin]
                void foo() {
                    int arr[2] = int[2](^[arr.init.0]1, ^[arr.init.1]2,  /* error */3);
                }
                ^[source.end]
            )");

            checkInlayHints("source.begin", "source.end",
                            InlayHintsExpectedResult{.numberOfHints = 2,
                                                     .hints =
                                                         {
                                                             {.positionLabel = "arr.init.0", .labelText = "[0]:"},
                                                             {.positionLabel = "arr.init.1", .labelText = "[1]:"},
                                                         }},
                            config);
        }

        SECTION("ConstructorOnStruct")
        {
            CompileLabelledSource(R"(
                ^[source.begin]
                struct S {
                    float xx;
                    float yy;
                    float zz;
                };

                void foo() {
                    S s = S(^[s.init.xx]1, ^[s.init.yy]2, ^[s.init.zz]3, /* error */4);
                }
                ^[source.end]
            )");

            checkInlayHints("source.begin", "source.end",
                            InlayHintsExpectedResult{.numberOfHints = 3,
                                                     .hints =
                                                         {
                                                             {.positionLabel = "s.init.xx", .labelText = ".xx:"},
                                                             {.positionLabel = "s.init.yy", .labelText = ".yy:"},
                                                             {.positionLabel = "s.init.zz", .labelText = ".zz:"},
                                                         }},
                            config);
        }
    }

    SECTION("ImplicitArraySizeHint")
    {
        CompileLabelledSource(R"(
            ^[source.begin]
            in float xs[]; // Here we shouldn't add hint

            void foo() {
                int arr1[^[arr1.size]] = {1, 2, 3};
                int arr2[^[arr2.decl.size]] = int[^[arr2.ctor.size]](4, 5, 6);
                int arr3[^[arr3.size]] = arr2;
            }
            ^[source.end]
        )");
        const auto config = InlayHintConfig{
            .enable                      = true,
            .enableImplicitArraySizeHint = true,
        };

        checkInlayHints("source.begin", "source.end",
                        InlayHintsExpectedResult{.numberOfHints = 4,
                                                 .hints =
                                                     {
                                                         {.positionLabel = "arr1.size", .labelText = "3"},
                                                         {.positionLabel = "arr2.decl.size", .labelText = "3"},
                                                         {.positionLabel = "arr2.ctor.size", .labelText = "3"},
                                                         {.positionLabel = "arr3.size", .labelText = "3"},
                                                     }},
                        config);
    }

    SECTION("ImplicitCastHint")
    {
        CompileLabelledSource(R"(
            ^[source.begin]
            void foo(float x) {
                double y = ^[hint.vardecl]x;
                double z = x + ^[hint.binary.expr]1;
                double w = x > 0 ? x : ^[hint.select.expr]0;
                double u = double(/* no hint here */x);
            }

            void bar() {
                foo(^[hint.call.arg]1);
                vec2 v = {^[hint.init1]1, ^[hint.init2]2};
            }
            ^[source.end]
        )");
        const auto config = InlayHintConfig{
            .enable                 = true,
            .enableImplicitCastHint = true,
        };

        checkInlayHints("source.begin", "source.end",
                        InlayHintsExpectedResult{.numberOfHints = 6,
                                                 .hints =
                                                     {
                                                         {.positionLabel = "hint.vardecl", .labelText = "(double)"},
                                                         {.positionLabel = "hint.binary.expr", .labelText = "(float)"},
                                                         {.positionLabel = "hint.select.expr", .labelText = "(float)"},
                                                         {.positionLabel = "hint.call.arg", .labelText = "(float)"},
                                                         {.positionLabel = "hint.init1", .labelText = "(float)"},
                                                         {.positionLabel = "hint.init2", .labelText = "(float)"},
                                                     }},
                        config);
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

        checkInlayHints("source.begin", "source.end",
                        InlayHintsExpectedResult{.numberOfHints = 3,
                                                 .hints =
                                                     {
                                                         {.positionLabel = "foo.end", .labelText = "// foo"},
                                                         {.positionLabel = "bar.end", .labelText = "// bar"},
                                                         {.positionLabel = "baz.end", .labelText = "// baz"},
                                                     }},
                        InlayHintConfig{
                            .enable                    = true,
                            .enableBlockEndHint        = true,
                            .blockEndHintLineThreshold = 0,
                        });

        checkInlayHints("source.begin", "source.end",
                        InlayHintsExpectedResult{.numberOfHints = 1,
                                                 .hints =
                                                     {
                                                         {.positionLabel = "baz.end", .labelText = "// baz"},
                                                     }},
                        InlayHintConfig{
                            .enable                    = true,
                            .enableBlockEndHint        = true,
                            .blockEndHintLineThreshold = 3,
                        });
    }
}

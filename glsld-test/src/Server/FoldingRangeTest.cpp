#include "ServerTestFixture.h"

#include "Feature/FoldingRange.h"

#include <catch2/catch_test_macros.hpp>

using namespace glsld;

static auto MockFoldingRange(const ServerTestFixture& fixture, const FoldingRangeConfig& config = {.enable = true})
    -> std::vector<lsp::FoldingRange>
{
    return HandleFoldingRange(config, fixture.GetLanguageQueryInfo(),
                              lsp::FoldingRangeParams{
                                  .textDocument = {"MockDocument"},
                              });
}

struct OneFoldingRangeExpectedResult
{
    StringView startLabel;
    StringView endLabel;
};

struct FoldingRangeExpectedResult
{
    std::optional<size_t> numberOfRanges = std::nullopt;

    std::vector<OneFoldingRangeExpectedResult> ranges;
};

TEST_CASE_METHOD(ServerTestFixture, "FoldingRangeTest")
{
    auto checkFoldingRange = [this](FoldingRangeExpectedResult expectedResult,
                                    const FoldingRangeConfig& config = {.enable = true}) {
        auto foldingRanges = MockFoldingRange(*this, config);
        if (expectedResult.numberOfRanges.has_value()) {
            REQUIRE(foldingRanges.size() == expectedResult.numberOfRanges.value());
        }

        for (const auto& expectedRange : expectedResult.ranges) {
            auto expectedStartLine = static_cast<uint32_t>(GetLabelledPosition(expectedRange.startLabel).line);
            auto expectedEndLine   = static_cast<uint32_t>(GetLabelledPosition(expectedRange.endLabel).line);

            auto it = std::ranges::find_if(foldingRanges, [=](const lsp::FoldingRange& range) {
                return range.startLine == expectedStartLine && range.endLine == expectedEndLine;
            });

            if (it == foldingRanges.end()) {
                for (const auto& range : foldingRanges) {
                    // FIXME: this isn't printed on failure for some reason
                    INFO(fmt::format("Folding Range: {} to {}", range.startLine, range.endLine));
                }
                FAIL(fmt::format("Folding range from {} to {} not found", expectedRange.startLabel,
                                 expectedRange.endLabel));
            }
        }
    };

    SECTION("Config")
    {
        CompileLabelledSource(R"(
            void foo()
            {
            }
        )");

        checkFoldingRange(
            FoldingRangeExpectedResult{
                .numberOfRanges = 0,
            },
            FoldingRangeConfig{.enable = false});
    }

    SECTION("FunctionBody")
    {
        CompileLabelledSource(R"(
            void foo()
            ^[foo.body.begin]{
                int x = 1;
            }^[foo.body.end]

            void bar() ^[bar.body.begin]{
                float y = 2.0;
            }^[bar.body.end]
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            .numberOfRanges = 2,
            .ranges =
                {
                    {.startLabel = "foo.body.begin", .endLabel = "foo.body.end"},
                    {.startLabel = "bar.body.begin", .endLabel = "bar.body.end"},
                },
        });
    }

    SECTION("NestedBlocks")
    {
        CompileLabelledSource(R"(
            void main()
            {
                if (true)
                ^[if.body.begin]{
                    int x = 1;
                }^[if.body.end]
                else
                ^[else.body.begin]{
                    int y = 2;
                }^[else.body.end]
            }
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            .numberOfRanges = 3,
            .ranges =
                {
                    {.startLabel = "if.body.begin", .endLabel = "if.body.end"},
                    {.startLabel = "else.body.begin", .endLabel = "else.body.end"},
                },
        });
    }

    SECTION("ForLoop")
    {
        CompileLabelledSource(R"(
            void main()
            {
                for (int i = 0; i < 10; i++)
                ^[for.body.begin]{
                    int x = i * 2;
                }^[for.body.end]
            }
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            .numberOfRanges = 2,
            .ranges =
                {
                    {.startLabel = "for.body.begin", .endLabel = "for.body.end"},
                },
        });
    }

    SECTION("WhileLoop")
    {
        CompileLabelledSource(R"(
            void main()
            {
                int i = 0;
                while (i < 10)
                ^[while.body.begin]{
                    i++;
                }^[while.body.end]
            }
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            .numberOfRanges = 2,
            .ranges =
                {
                    {.startLabel = "while.body.begin", .endLabel = "while.body.end"},
                },
        });
    }

    SECTION("DoWhileLoop")
    {
        CompileLabelledSource(R"(
            void main()
            {
                int i = 0;
                do
                ^[do.body.begin]{
                    i++;
                }^[do.body.end] while (i < 10);
            }
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            .numberOfRanges = 2,
            .ranges =
                {
                    {.startLabel = "do.body.begin", .endLabel = "do.body.end"},
                },
        });
    }

    SECTION("SwitchStatement")
    {
        CompileLabelledSource(R"(
            void main()
            {
                int x = 1;
                switch (x)
                ^[switch.body.begin]{
                    case 0:
                        break;
                    case 1:
                        break;
                    default:
                        break;
                }^[switch.body.end]
            }
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            .numberOfRanges = 2,
            .ranges =
                {
                    {.startLabel = "switch.body.begin", .endLabel = "switch.body.end"},
                },
        });
    }

    SECTION("StructDeclaration")
    {
        CompileLabelledSource(R"(
            ^[struct.body.begin]struct Material
            {
                vec3 ambient;
                vec3 diffuse;
                vec3 specular;
                float shininess;
            }^[struct.body.end];
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            .numberOfRanges = 1,
            .ranges =
                {
                    {.startLabel = "struct.body.begin", .endLabel = "struct.body.end"},
                },
        });
    }

    SECTION("InterfaceBlock")
    {
        CompileLabelledSource(R"(
            ^[interface.block.body.begin]uniform Material
            {
                vec3 ambient;
                vec3 diffuse;
            }^[interface.block.body.end] material;
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            .numberOfRanges = 1,
            .ranges =
                {
                    {.startLabel = "interface.block.body.begin", .endLabel = "interface.block.body.end"},
                },
        });
    }

    SECTION("ComplexNesting")
    {
        CompileLabelledSource(R"(
            struct Light
            {
                vec3 position;
                vec3 color;
            };

            void calculateLighting()
            {
                Light light;
                for (int i = 0; i < 3; i++)
                {
                    if (i == 0)
                    {
                        light.color = vec3(1.0);
                    }
                    else
                    {
                        light.color = vec3(0.5);
                    }
                }
            }

            void main()
            {
                calculateLighting();
            }
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            // Should have multiple folding ranges:
            // - struct Light body
            // - calculateLighting() body
            // - for loop body
            // - if body
            // - else body
            // - main() body
            .numberOfRanges = 6,
        });
    }

    SECTION("SingleLineNoFold")
    {
        CompileLabelledSource(R"(
            void foo() { int x = 1; }
            struct S { int a; };
        )");

        checkFoldingRange(FoldingRangeExpectedResult{
            .numberOfRanges = 0,
        });
    }
}

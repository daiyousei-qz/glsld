#include "ServerTestFixture.h"

#include "Feature/FoldingRange.h"
#include "Support/SourceText.h"

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

TEST_CASE_METHOD(ServerTestFixture, "FoldingRangeTest")
{
    auto checkFoldingRange = [](const lsp::FoldingRange& range, uint32_t startLine, uint32_t endLine) {
        REQUIRE(range.startLine == startLine);
        REQUIRE(range.endLine == endLine);
    };

    auto findFoldingRange = [](const std::vector<lsp::FoldingRange>& ranges, uint32_t startLine) {
        for (const auto& range : ranges) {
            if (range.startLine == startLine) {
                return &range;
            }
        }
        return static_cast<const lsp::FoldingRange*>(nullptr);
    };

    SECTION("Config")
    {
        CompileLabelledSource(R"(
            void foo()
            {
            }
        )");
        auto foldingRanges = MockFoldingRange(*this, FoldingRangeConfig{.enable = false});

        REQUIRE(foldingRanges.empty());
    }

    SECTION("FunctionBody")
    {
        CompileLabelledSource(R"(
            void foo()
            {
                int x = 1;
            }

            void bar() {
                float y = 2.0;
            }
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Should have 2 folding ranges for the two function bodies
        REQUIRE(foldingRanges.size() == 2);
        
        // foo() body spans lines 2-4
        checkFoldingRange(foldingRanges[0], 2, 4);
        
        // bar() body spans lines 6-8
        checkFoldingRange(foldingRanges[1], 6, 8);
    }

    SECTION("NestedBlocks")
    {
        CompileLabelledSource(R"(
            void main()
            {
                if (true)
                {
                    int x = 1;
                }
                else
                {
                    int y = 2;
                }
            }
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Should have 3 folding ranges: main body, if body, else body
        REQUIRE(foldingRanges.size() == 3);
        
        // main() body spans lines 2-11
        auto mainRange = findFoldingRange(foldingRanges, 2);
        REQUIRE(mainRange != nullptr);
        checkFoldingRange(*mainRange, 2, 11);
        
        // if body spans lines 4-6
        auto ifRange = findFoldingRange(foldingRanges, 4);
        REQUIRE(ifRange != nullptr);
        checkFoldingRange(*ifRange, 4, 6);
        
        // else body spans lines 8-10
        auto elseRange = findFoldingRange(foldingRanges, 8);
        REQUIRE(elseRange != nullptr);
        checkFoldingRange(*elseRange, 8, 10);
    }

    SECTION("ForLoop")
    {
        CompileLabelledSource(R"(
            void main()
            {
                for (int i = 0; i < 10; i++)
                {
                    int x = i * 2;
                }
            }
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Should have 2 folding ranges: main body and for loop body
        REQUIRE(foldingRanges.size() == 2);
    }

    SECTION("WhileLoop")
    {
        CompileLabelledSource(R"(
            void main()
            {
                int i = 0;
                while (i < 10)
                {
                    i++;
                }
            }
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Should have 2 folding ranges: main body and while loop body
        REQUIRE(foldingRanges.size() == 2);
    }

    SECTION("DoWhileLoop")
    {
        CompileLabelledSource(R"(
            void main()
            {
                int i = 0;
                do
                {
                    i++;
                } while (i < 10);
            }
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Should have 2 folding ranges: main body and do-while loop body
        REQUIRE(foldingRanges.size() == 2);
    }

    SECTION("SwitchStatement")
    {
        CompileLabelledSource(R"(
            void main()
            {
                int x = 1;
                switch (x)
                {
                    case 0:
                        break;
                    case 1:
                        break;
                    default:
                        break;
                }
            }
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Should have 2 folding ranges: main body and switch body
        REQUIRE(foldingRanges.size() == 2);
    }

    SECTION("StructDeclaration")
    {
        CompileLabelledSource(R"(
            struct Material
            {
                vec3 ambient;
                vec3 diffuse;
                vec3 specular;
                float shininess;
            };
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Should have 1 folding range for the struct body
        REQUIRE(foldingRanges.size() == 1);
        
        // Struct body spans lines 2-7
        checkFoldingRange(foldingRanges[0], 2, 7);
    }

    SECTION("InterfaceBlock")
    {
        CompileLabelledSource(R"(
            uniform Material
            {
                vec3 ambient;
                vec3 diffuse;
            } material;
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Should have 1 folding range for the interface block body
        REQUIRE(foldingRanges.size() == 1);
        
        // Interface block body spans lines 2-5
        checkFoldingRange(foldingRanges[0], 2, 5);
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
        auto foldingRanges = MockFoldingRange(*this);

        // Should have multiple folding ranges:
        // - struct Light body
        // - calculateLighting() body
        // - for loop body
        // - if body
        // - else body
        // - main() body
        REQUIRE(foldingRanges.size() >= 6);
    }

    SECTION("SingleLineNoFold")
    {
        CompileLabelledSource(R"(
            void foo() { int x = 1; }
            struct S { int a; };
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Single-line blocks should not create folding ranges
        REQUIRE(foldingRanges.empty());
    }

    SECTION("EmptyBlocks")
    {
        CompileLabelledSource(R"(
            void foo()
            {
            }

            struct S
            {
            };
        )");
        auto foldingRanges = MockFoldingRange(*this);

        // Empty multi-line blocks should still create folding ranges
        REQUIRE(foldingRanges.size() == 2);
    }
}

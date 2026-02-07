#include "ServerTestFixture.h"

#include "Feature/Completion.h"
#include "Support/SourceText.h"

using namespace glsld;

static auto MockCompletion(const ServerTestFixture& fixture, TextPosition pos,
                           const CompletionConfig& config = {.enable = true}) -> lsp::CompletionList
{
    CompletionState state;
    return HandleCompletion(config, fixture.GetLanguageQueryInfo(), state,
                            lsp::CompletionParams{
                                .textDocument = {"MockDocument"},
                                .position     = ToLspPosition(pos),
                            });
}

struct CompletionItemExpectedResult
{
    StringView label;
    lsp::CompletionItemKind kind;
};

struct CompletionExpectedResult
{
    std::optional<bool> isEmpty = std::nullopt;

    std::optional<bool> isIncomplete = std::nullopt;

    // This is non-exhaustive check
    std::vector<CompletionItemExpectedResult> items;
};

TEST_CASE_METHOD(ServerTestFixture, "CompletionTest")
{
    auto checkCompletion = [this](StringView cursorLabel, CompletionExpectedResult expectedResult,
                                  const CompletionConfig& config = {.enable = true}) {
        auto completionList = MockCompletion(*this, GetLabelledPosition(cursorLabel), config);

        if (expectedResult.isEmpty && *expectedResult.isEmpty != completionList.items.empty()) {
            if (*expectedResult.isEmpty) {
                FAIL(fmt::format("Expected completion list to be empty, but it has {} items",
                                 completionList.items.size()));
            }
            else {
                FAIL("Expected completion list to be non-empty");
            }
        }
        if (expectedResult.isIncomplete) {
            REQUIRE(completionList.isIncomplete == *expectedResult.isIncomplete);
        }

        for (const auto& itemExpectedResult : expectedResult.items) {
            auto it =
                std::ranges::find_if(completionList.items, [&itemExpectedResult](const lsp::CompletionItem& item) {
                    return item.label == itemExpectedResult.label && item.kind == itemExpectedResult.kind;
                });
            if (it == completionList.items.end()) {
                FAIL(fmt::format("Failed to find completion item: label='{}', kind={}", itemExpectedResult.label,
                                 EnumToString(itemExpectedResult.kind)));
            }
        }
    };

    SECTION("Config")
    {
        CompileLabelledSource(R"(
            void main() { ^[cursor.pos] }
        )");

        checkCompletion("cursor.pos", CompletionExpectedResult{.isEmpty = true}, CompletionConfig{.enable = false});
    }

    SECTION("Keyword")
    {
        CompileLabelledSource(R"(
            ^[cursor.pos.1]
            void main() { ^[cursor.pos.2] }
        )");
        checkCompletion("cursor.pos.1", CompletionExpectedResult{
                                            .items =
                                                {
                                                    {.label = "struct", .kind = lsp::CompletionItemKind::Keyword},
                                                    {.label = "void", .kind = lsp::CompletionItemKind::Keyword},
                                                },
                                        });
        checkCompletion("cursor.pos.2", CompletionExpectedResult{
                                            .items =
                                                {
                                                    {.label = "return", .kind = lsp::CompletionItemKind::Keyword},
                                                    {.label = "if", .kind = lsp::CompletionItemKind::Keyword},
                                                },
                                        });
    }

    SECTION("StructMemberAccess")
    {
        CompileLabelledSource(R"(
            struct S {
                vec4 data;
                vec4 anotherData;
            };

            void main() {
                S s;
                s.^[field.use1];
                s.d^[field.use2]
            }
        )");

        checkCompletion("field.use1", CompletionExpectedResult{
                                          .items =
                                              {
                                                  {.label = "data", .kind = lsp::CompletionItemKind::Field},
                                                  {.label = "anotherData", .kind = lsp::CompletionItemKind::Field},
                                              },
                                      });

        checkCompletion("field.use2", CompletionExpectedResult{
                                          .items =
                                              {
                                                  {.label = "data", .kind = lsp::CompletionItemKind::Field},
                                                  {.label = "anotherData", .kind = lsp::CompletionItemKind::Field},
                                              },
                                      });
    }

    SECTION("BlockMemberAccess")
    {
        CompileLabelledSource(R"(
            uniform UBO {
                vec4 dataUBO;
            };

            buffer SSBO {
                vec4 dataSSBO;
            } ssbo;

            vec4 foo() {
                return ssbo.^[blockMember.use1] + ^[blockMember.use2];
            }
        )");

        checkCompletion("blockMember.use1", CompletionExpectedResult{
                                                .items =
                                                    {
                                                        {.label = "dataSSBO", .kind = lsp::CompletionItemKind::Field},
                                                    },
                                            });

        checkCompletion("blockMember.use2", CompletionExpectedResult{
                                                .items =
                                                    {
                                                        {.label = "dataUBO", .kind = lsp::CompletionItemKind::Variable},
                                                    },
                                            });
    }

    SECTION("LengthOperator")
    {
        CompileLabelledSource(R"(
            int xs[2] = {1, 2};
            void foo(vec3 v) {
                mat3 m;
                xs.^[length.use1];
                v.le^[length.use2];
                m.le^[length.use3]ngth;
            }
        )");

        checkCompletion("length.use1", CompletionExpectedResult{
                                           .items =
                                               {
                                                   {.label = "length", .kind = lsp::CompletionItemKind::Method},
                                               },
                                       });
        checkCompletion("length.use2", CompletionExpectedResult{
                                           .items =
                                               {
                                                   {.label = "length", .kind = lsp::CompletionItemKind::Method},
                                               },
                                       });
        checkCompletion("length.use3", CompletionExpectedResult{
                                           .items =
                                               {
                                                   {.label = "length", .kind = lsp::CompletionItemKind::Method},
                                               },
                                       });
    }

    SECTION("SwizzleAccess")
    {
        CompileLabelledSource(R"(
            void foo(vec4 v1, vec3 v2) {
                v1.^[swizzle.use1];
                v1.x^[swizzle.use2];
                v2.x^[swizzle.use3]y
            }
        )");

        checkCompletion("swizzle.use1", CompletionExpectedResult{
                                            .isIncomplete = true,
                                            .items =
                                                {
                                                    // xyzw
                                                    {.label = "x", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "y", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "z", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "w", .kind = lsp::CompletionItemKind::Field},
                                                    // rgba
                                                    {.label = "r", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "g", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "b", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "a", .kind = lsp::CompletionItemKind::Field},
                                                    // pqst
                                                    {.label = "p", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "q", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "s", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "t", .kind = lsp::CompletionItemKind::Field},
                                                },
                                        });
        checkCompletion("swizzle.use2", CompletionExpectedResult{
                                            .isIncomplete = true,
                                            .items =
                                                {
                                                    // Current swizzle
                                                    {.label = "x", .kind = lsp::CompletionItemKind::Field},
                                                    // Extending the current swizzle
                                                    {.label = "xx", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "xy", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "xz", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "xw", .kind = lsp::CompletionItemKind::Field},
                                                },
                                        });
        checkCompletion("swizzle.use3", CompletionExpectedResult{
                                            .isIncomplete = true,
                                            .items =
                                                {
                                                    // Current swizzle
                                                    {.label = "xy", .kind = lsp::CompletionItemKind::Field},
                                                    // Extending the current swizzle
                                                    {.label = "xyx", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "xyy", .kind = lsp::CompletionItemKind::Field},
                                                    {.label = "xyz", .kind = lsp::CompletionItemKind::Field},
                                                    // No 'w' as v2 is vec3
                                                },
                                        });
    }
}

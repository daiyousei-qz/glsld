#include "ServerTestFixture.h"

#include "Feature/Reference.h"
#include "Support/SourceText.h"

#include <ranges>

using namespace glsld;

static auto MockReferences(const ServerTestFixture& fixture, TextPosition pos, bool includeDeclaration,
                           const ReferenceConfig& config = {}) -> std::vector<lsp::Location>
{
    return HandleReferences(config, fixture.GetLanguageQueryInfo(),
                            lsp::ReferenceParams{
                                .textDocument = {"MockDocument"},
                                .position     = ToLspPosition(pos),
                                .context      = {.includeDeclaration = includeDeclaration},
                            });
}

TEST_CASE_METHOD(ServerTestFixture, "ReferenceTest")
{
    auto checkNoReference = [this](StringView labelPos, bool includeDeclaration = false,
                                   const ReferenceConfig& config = {}) {
        auto refs = MockReferences(*this, GetLabelledPosition(labelPos), includeDeclaration, config);
        REQUIRE(refs.empty());
    };

    auto checkReference = [this](StringView labelPos,
                                 const std::vector<std::pair<StringView, StringView>>& expectedRefLabels,
                                 bool includeDeclaration = false, const ReferenceConfig& config = {}) {
        auto refs = MockReferences(*this, GetLabelledPosition(labelPos), includeDeclaration, config);
        REQUIRE(refs.size() == expectedRefLabels.size());
        for (auto [ref, labelPair] : std::views::zip(refs, expectedRefLabels)) {
            auto [labelBegin, labelEnd] = labelPair;
            REQUIRE(FromLspRange(ref.range) == GetLabelledRange(labelBegin, labelEnd));
        }
    };

    SECTION("Config")
    {
        CompileLabelledSource(R"(
            float ^[var.decl]var^[var.decl.end];

            void foo() {
                ^[var.use]var^[var.use.end] = 1.0;
            }
        )");

        {
            checkNoReference("var.decl", false, ReferenceConfig{.enable = false});
            checkNoReference("var.use", false, ReferenceConfig{.enable = false});
        }
    }

    SECTION("Macro")
    {
        CompileLabelledSource(R"(
            #ifdef ^[MACRO.unknown.use.begin]MACRO^[MACRO.unknown.use.end]
            #endif

            #define ^[MACRO.def.begin]MACRO^[MACRO.def.end] 1

            #ifdef ^[MACRO.use1.begin]MACRO^[MACRO.use1.end]
            #endif

            #undef ^[MACRO.use2.begin]MACRO^[MACRO.use2.end]

            #define ^[MACRO.redef.begin]MACRO^[MACRO.redef.end] 2

            #if defined ^[MACRO.use3.begin]MACRO^[MACRO.use3.end] && ^[MACRO.use4.begin]MACRO^[MACRO.use4.end]
            #endif

            int x = ^[MACRO.use5.begin]MACRO^[MACRO.use5.end];
        )");

        // FIXME: should return none for unknown macro?
        checkReference("MACRO.unknown.use.begin", {
                                                      {"MACRO.unknown.use.begin", "MACRO.unknown.use.end"},
                                                  });

        checkReference("MACRO.def.begin", {
                                              {"MACRO.use1.begin", "MACRO.use1.end"},
                                              {"MACRO.use2.begin", "MACRO.use2.end"},
                                          });

        checkReference("MACRO.redef.begin", {
                                                {"MACRO.use3.begin", "MACRO.use3.end"},
                                                {"MACRO.use4.begin", "MACRO.use4.end"},
                                                {"MACRO.use5.begin", "MACRO.use5.end"},
                                            });

        checkReference("MACRO.use3.begin",
                       {
                           {"MACRO.redef.begin", "MACRO.redef.end"},
                           {"MACRO.use3.begin", "MACRO.use3.end"},
                           {"MACRO.use4.begin", "MACRO.use4.end"},
                           {"MACRO.use5.begin", "MACRO.use5.end"},
                       },
                       true);
    }

    SECTION("Struct")
    {
        CompileLabelledSource(R"(
            struct ^[s.decl.begin]S^[s.decl.end]
            {
            };

            ^[s.use1.begin]S^[s.use1.end] foo(^[s.use2.begin]S^[s.use2.end] s) {
                return s;
            }
        )");

        checkReference("s.decl.begin", {
                                           {"s.use1.begin", "s.use1.end"},
                                           {"s.use2.begin", "s.use2.end"},
                                       });

        checkReference("s.use1.begin", {
                                           {"s.use1.begin", "s.use1.end"},
                                           {"s.use2.begin", "s.use2.end"},
                                       });

        checkReference("s.use2.begin",
                       {
                           {"s.decl.begin", "s.decl.end"},
                           {"s.use1.begin", "s.use1.end"},
                           {"s.use2.begin", "s.use2.end"},
                       },
                       true);
    }

    SECTION("StructField")
    {
        CompileLabelledSource(R"(
            struct S
            {
                float ^[field.decl.begin]field^[field.decl.end];
            };

            void foo() {
                S s;
                s.^[field.use.begin]field^[field.use.end] = 1.0;
            }
        )");

        checkReference("field.decl.begin", {
                                               {"field.use.begin", "field.use.end"},
                                           });

        checkReference("field.use.begin",
                       {
                           {"field.decl.begin", "field.decl.end"},
                           {"field.use.begin", "field.use.end"},
                       },
                       true);
    }

    SECTION("Variable")
    {
        CompileLabelledSource(R"(
            float ^[global.decl.begin]global^[global.decl.end];

            void foo() {
                ^[global.use1.begin]global^[global.use1.end] = 1.0;
                float ^[local.decl.begin]local^[local.decl.end] = ^[global.use2.begin]global^[global.use2.end];
                ^[local.use1.begin]local^[local.use1.end] += 2.0;
            }
        )");

        checkReference("global.decl.begin", {
                                                {"global.use1.begin", "global.use1.end"},
                                                {"global.use2.begin", "global.use2.end"},
                                            });
        checkReference("local.decl.begin", {
                                               {"local.use1.begin", "local.use1.end"},
                                           });
        checkReference("global.use1.begin",
                       {
                           {"global.decl.begin", "global.decl.end"},
                           {"global.use1.begin", "global.use1.end"},
                           {"global.use2.begin", "global.use2.end"},
                       },
                       true);
    }

    SECTION("Block")
    {
        CompileLabelledSource(R"(
            uniform ^[ubo.decl.begin]UBO^[ubo.decl.end] {
                int blockVar;
            } ^[ubo.instance.decl.begin]ubo^[ubo.instance.decl.end];

            int foo() {
                // this UBO doesn't refer to the declaration because block doesn't introduce a type
                UBO test;
                return ^[ubo.instance.use.begin]ubo^[ubo.instance.use.end].blockVar;
            }
        )");

        // FIXME: enable this test
        // checkNoReference("ubo.decl.begin");

        checkReference("ubo.instance.decl.begin", {
                                                      {"ubo.instance.use.begin", "ubo.instance.use.end"},
                                                  });
        checkReference("ubo.instance.use.begin",
                       {
                           {"ubo.instance.decl.begin", "ubo.instance.decl.end"},
                           {"ubo.instance.use.begin", "ubo.instance.use.end"},
                       },
                       true);
    }

    SECTION("BlockField")
    {
        CompileLabelledSource(R"(
            uniform UBO {
                int ^[index.decl.begin]index^[index.decl.end];
            };

            buffer SSBO {
                float ^[data.decl.begin]data^[data.decl.end][];
            } ssbo;

            int foo() {
                return ssbo.^[data.use.begin]data^[data.use.end][^[index.use.begin]index^[index.use.end]];
            }
        )");

        checkReference("index.decl.begin", {
                                               {"index.use.begin", "index.use.end"},
                                           });
        checkReference("data.decl.begin", {
                                              {"data.use.begin", "data.use.end"},
                                          });
        checkReference("data.use.begin",
                       {
                           {"data.decl.begin", "data.decl.end"},
                           {"data.use.begin", "data.use.end"},
                       },
                       true);
        checkReference("index.use.begin",
                       {
                           {"index.decl.begin", "index.decl.end"},
                           {"index.use.begin", "index.use.end"},
                       },
                       true);
    }

    SECTION("Parameter")
    {
        CompileLabelledSource(R"(
            float param;

            float foo(float ^[param.decl.begin]param^[param.decl.end]) {
                return ^[param.use1.begin]param^[param.use1.end];
            }
        )");

        checkReference("param.decl.begin", {
                                               {"param.use1.begin", "param.use1.end"},
                                           });
        checkReference("param.use1.begin",
                       {
                           {"param.decl.begin", "param.decl.end"},
                           {"param.use1.begin", "param.use1.end"},
                       },
                       true);
    }

    SECTION("Function")
    {
        CompileLabelledSource(R"(
            float ^[func.decl.begin]func^[func.decl.end](float x) {
                return x;
            }

            void bar() {
                float y = ^[func.use1.begin]func^[func.use1.end](1.0) + ^[func.use2.begin]func^[func.use2.end](2.0);
            }
        )");

        checkReference("func.decl.begin", {
                                              {"func.use1.begin", "func.use1.end"},
                                              {"func.use2.begin", "func.use2.end"},
                                          });
        checkReference("func.use1.begin", {
                                              {"func.use1.begin", "func.use1.end"},
                                              {"func.use2.begin", "func.use2.end"},
                                          });
        checkReference("func.use2.begin",
                       {
                           {"func.decl.begin", "func.decl.end"},
                           {"func.use1.begin", "func.use1.end"},
                           {"func.use2.begin", "func.use2.end"},
                       },
                       true);
    }
}
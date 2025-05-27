#include "ServerTestFixture.h"

#include "Feature/Definition.h"
#include "Support/SourceText.h"

using namespace glsld;

static auto MockDefinition(const ServerTestFixture& fixture, TextPosition pos, const DefinitionConfig& config = {})
    -> std::vector<lsp::Location>
{
    return HandleDefinition(config, fixture.GetLanguageQueryInfo(),
                            lsp::DefinitionParams{
                                .textDocument = {"MockDocument"},
                                .position     = ToLspPosition(pos),
                            });
}

TEST_CASE_METHOD(ServerTestFixture, "DefinitionTest")
{
    auto checkNoDefinition = [&](StringView labelPos) {
        auto locations = MockDefinition(*this, GetLabelledPosition(labelPos));
        REQUIRE(locations.empty());
    };

    auto checkDefinition = [&](StringView labelPos, StringView labelDefBegin, StringView labelDefEnd) {
        auto locations = MockDefinition(*this, GetLabelledPosition(labelPos));
        REQUIRE(locations.size() == 1);
        REQUIRE(FromLspRange(locations[0].range) == GetLabelledRange(labelDefBegin, labelDefEnd));
    };

    SECTION("Config")
    {
        CompileLabelledSource(R"(
            void ^[foo.decl.pos]foo() {
            }
        )");

        auto definitionLocations =
            MockDefinition(*this, GetLabelledPosition("foo.decl.pos"), DefinitionConfig{.enable = false});
        REQUIRE(definitionLocations.empty());
    }

    SECTION("Macro")
    {
        CompileLabelledSource(R"(
            #ifdef ^[MACRO.bad.use1]MACRO
            #endif

            #define ^[MACRO.def.begin]MACRO^[MACRO.def.end] 1

            #ifdef ^[MACRO.use1]MACRO
            #endif

            #if defined ^[MACRO.use2]MACRO && ^[MACRO.use3]MACRO
            #endif

            int x = ^[MACRO.use4]MACRO;

            #undef ^[MACRO.use5]MACRO
        )");

        checkNoDefinition("MACRO.bad.use1");

        checkDefinition("MACRO.use1", "MACRO.def.begin", "MACRO.def.end");
        checkDefinition("MACRO.use2", "MACRO.def.begin", "MACRO.def.end");
        checkDefinition("MACRO.use3", "MACRO.def.begin", "MACRO.def.end");
        checkDefinition("MACRO.use4", "MACRO.def.begin", "MACRO.def.end");
        checkDefinition("MACRO.use5", "MACRO.def.begin", "MACRO.def.end");
    }

    SECTION("Function")
    {
        CompileLabelledSource(R"(
            void bar() {
                ^[foo.bad.use1]foo();
            }

            void ^[foo.def.begin]foo^[foo.def.end]()
            {
                // GLSL doesn't have recursion
                ^[foo.bad.use2]foo();
            }

            void main() {
                ^[foo.use1]foo();
            }
        )");

        checkNoDefinition("foo.bad.use1");
        checkNoDefinition("foo.bad.use2");

        checkDefinition("foo.use1", "foo.def.begin", "foo.def.end");
    }

    SECTION("FunctionParameter")
    {
        CompileLabelledSource(R"(
            void foo(int ^[x.def.begin]x^[x.def.end],
                     out int ^[y.def.begin]y^[y.def.end]) {
                ^[y.use1]y = ^[x.use1]x;
            }
        )");

        checkDefinition("x.use1", "x.def.begin", "x.def.end");
        checkDefinition("y.use1", "y.def.begin", "y.def.end");
    }

    SECTION("GlobalVariable")
    {
        CompileLabelledSource(R"(
            int bad = ^[x.bad.use1]x;

            int ^[x.def.begin]x^[x.def.end] = 1;
            int y = ^[x.use1]x;
            int z = {^[x.use2]x};

            void foo() {
                ^[x.use3]x + y + z;
            }
        )");

        checkNoDefinition("x.bad.use1");

        checkDefinition("x.use1", "x.def.begin", "x.def.end");
        checkDefinition("x.use2", "x.def.begin", "x.def.end");
        checkDefinition("x.use3", "x.def.begin", "x.def.end");
    }

    SECTION("LocalVariable")
    {
        CompileLabelledSource(R"(
            void foo() {
                {
                    ^[x.bad.use1]x;
                    int ^[x.def.begin]x^[x.def.end] = 1;
                    int y = ^[x.use1]x;
                    int z = {^[x.use2]x};
                    ^[x.use3]x + y + z;
                }
                ^[x.bad.use2]x;
            }
        )");

        checkNoDefinition("x.bad.use1");
        checkNoDefinition("x.bad.use2");

        checkDefinition("x.use1", "x.def.begin", "x.def.end");
        checkDefinition("x.use2", "x.def.begin", "x.def.end");
        checkDefinition("x.use3", "x.def.begin", "x.def.end");
    }

    SECTION("Struct")
    {
        CompileLabelledSource(R"(
            struct ^[S.def.begin]S^[S.def.end] {
                int value;
            };

            struct T {
                ^[S.use1]S s;
            };

            void foo(^[S.use2]S x) {
                ^[S.use3]S y = x;
                ^[S.use4]S[1] z = {x};
                y = ^[S.use5]S(1);
                z = ^[S.use6]S[1](y);
            }
        )");

        checkDefinition("S.use1", "S.def.begin", "S.def.end");
        checkDefinition("S.use2", "S.def.begin", "S.def.end");
        checkDefinition("S.use3", "S.def.begin", "S.def.end");
        checkDefinition("S.use4", "S.def.begin", "S.def.end");
        checkDefinition("S.use5", "S.def.begin", "S.def.end");
        checkDefinition("S.use6", "S.def.begin", "S.def.end");
    }

    SECTION("StructField")
    {
        CompileLabelledSource(R"(
            struct S {
                int ^[value.def.begin]value^[value.def.end];
            };

            void foo(S x) {
                x.^[value.use1]value = 1;
                int y = x.^[value.use2]value;
            }
        )");

        checkDefinition("value.use1", "value.def.begin", "value.def.end");
        checkDefinition("value.use2", "value.def.begin", "value.def.end");
    }

    SECTION("BlockInstance")
    {
    }

    SECTION("BlockField")
    {
    }
}
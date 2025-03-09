#include "ServerTestFixture.h"

#include "Hover.h"

using namespace glsld;

TEST_CASE_METHOD(ServerTestFixture, "HoverTest")
{
    auto checkHover = [](ServerTestContext& ctx, TextPosition pos) {

    };

    SECTION("MacroName")
    {
        auto sourceText = R"(
        #define ^[MACRO.def.pos]MACRO 1
        ^[MACRO.use.pos]MACRO;
        #undef ^[MACRO.undef.pos]MACRO
        )";
        auto ctx        = CompileLabelledSource(sourceText);
    }

    SECTION("TypeName")
    {
        auto sourceText = R"(
        struct ^[struct.decl.pos]S
        {
        };

        void foo()
        {
            ^[struct.use.pos]S s;
            ^[unknown.use.pos]unknown u;
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);
    }

    SECTION("FunctionName")
    {
        auto sourceText = R"(
        void ^[foo.decl.pos]foo(S s)
        {
        }

        void ^[main.pos]main()
        {
            ^[foo.use.pos]foo();
        }
    )";
        auto ctx        = CompileLabelledSource(sourceText);
    }

    SECTION("BlockName")
    {
        auto sourceText = R"(
        uniform ^[ubo.decl.pos]UBO
        {
            int ^[x.decl.pos]x;
        } ^[block.decl.pos]block;
        void foo() {
            ^[block.use.pos]block.^[x.use.pos]x;
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);
    }

    SECTION("VariableName")
    {
        auto sourceText = R"(
        int ^[global.decl.pos]global;
        int foo(int ^[param.decl.pos]param)
        {
            int ^[local.decl.pos]local^[local.decl.end];

            return ^[local.use.pos]local
                 + ^[param.use.pos]param
                 + ^[global.use.pos]global
                 + ^[unknown.use.pos]unknown;
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);
    }
}
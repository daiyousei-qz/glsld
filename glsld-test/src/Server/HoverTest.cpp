#include "ServerTestFixture.h"

#include "Feature/Hover.h"
#include "Support/SourceText.h"

using namespace glsld;

TEST_CASE_METHOD(ServerTestFixture, "HoverTest")
{
    auto checkHover = [&](TextPosition pos) {

    };

    SECTION("Macro")
    {
        auto sourceText = R"(
            #ifdef ^[MACRO.unknown.use]MACRO
            #endif

            #define ^[MACRO.def]MACRO 1

            #ifdef ^[MACRO.use1]MACRO
            #endif

            #if defined ^[MACRO.use2]MACRO && ^[MACRO.use3]MACRO
            #endif

            int x = ^[MACRO.use4]MACRO;

            #undef ^[MACRO.use5]MACRO
        )";

        CompileLabelledSource(sourceText);
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

        CompileLabelledSource(sourceText);
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
        CompileLabelledSource(sourceText);
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

        CompileLabelledSource(sourceText);
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

        CompileLabelledSource(sourceText);
    }
}
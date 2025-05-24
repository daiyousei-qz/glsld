#include "ServerTestFixture.h"

#include "Server/LanguageQueryInfo.h"

using namespace glsld;

TEST_CASE_METHOD(ServerTestFixture, "SymbolQueryTest")
{
    auto checkSymbol = [this](StringView labelPos, SymbolDeclType type, StringView name, bool unknown = false) {
        auto queryResult = GetLanguageQueryInfo().QuerySymbolByPosition(GetLabelledPosition(labelPos));
        REQUIRE(queryResult.has_value());
        REQUIRE(queryResult->symbolType == type);
        REQUIRE(queryResult->spelledText == name);
    };

    SECTION("HeaderName")
    {
        CompileLabelledSource(R"(
            #include ^[pos.1]"test.h"
            #include ^[pos.2]<test.h>
        )");

        checkSymbol("pos.1", SymbolDeclType::HeaderName, "\"test.h\"");
        checkSymbol("pos.2", SymbolDeclType::HeaderName, "<test.h>");
    }

    SECTION("Macro")
    {
        CompileLabelledSource(R"(
            #define ^[macro1.decl.pos]MACRO1
            #define ^[macro2.decl.pos]MACRO2 1
            #define ^[macro3.decl.pos]MACRO3(X) X

            #ifdef ^[macro1.use.pos]MACRO1
            #endif
            #if defined ^[macro2.use.pos]MACRO2 && ^[macro3.use.pos]MACRO3(0)
            #endif

            #undef ^[macro1.undef.pos]MACRO1
            #undef ^[macro4.undef.pos]MACRO4
        )");

        checkSymbol("macro1.decl.pos", SymbolDeclType::Macro, "MACRO1");
        checkSymbol("macro2.decl.pos", SymbolDeclType::Macro, "MACRO2");
        checkSymbol("macro3.decl.pos", SymbolDeclType::Macro, "MACRO3");
        checkSymbol("macro1.use.pos", SymbolDeclType::Macro, "MACRO1");
        checkSymbol("macro2.use.pos", SymbolDeclType::Macro, "MACRO2");
        checkSymbol("macro3.use.pos", SymbolDeclType::Macro, "MACRO3");
        checkSymbol("macro1.undef.pos", SymbolDeclType::Macro, "MACRO1");
        checkSymbol("macro4.undef.pos", SymbolDeclType::Macro, "MACRO4", true);
    }

    SECTION("LayoutQualifier")
    {
        CompileLabelledSource(R"(
            layout(^[pos.1]binding = 0, ^[pos.2]rgba) uniform image2D img;
        )");

        checkSymbol("pos.1", SymbolDeclType::LayoutQualifier, "binding");
        checkSymbol("pos.2", SymbolDeclType::LayoutQualifier, "rgba");
    }

    SECTION("StructName")
    {
        CompileLabelledSource(R"(
            struct ^[struct.decl.pos]S
            {
                int ^[member.decl.pos]member;
            };

            void foo()
            {
                ^[struct.use.pos]S s;
                s.^[member.use.pos]member;
                s.^[unknown.member.use.pos]unknown;

                ^[unknown.type.use.pos]Unknown u;
            }
        )");

        checkSymbol("struct.decl.pos", SymbolDeclType::Type, "S");
        checkSymbol("struct.use.pos", SymbolDeclType::Type, "S");
        checkSymbol("member.decl.pos", SymbolDeclType::StructMember, "member");
        checkSymbol("member.use.pos", SymbolDeclType::StructMember, "member");
        checkSymbol("unknown.member.use.pos", SymbolDeclType::StructMember, "unknown", true);
        checkSymbol("unknown.type.use.pos", SymbolDeclType::Type, "Unknown", true);
    }

    SECTION("VariableName")
    {
        CompileLabelledSource(R"(
            int ^[global.decl.pos]global;
            int foo(int ^[param.decl.pos]param)
            {
                int ^[local.decl.pos]local;

                return ^[local.use.pos]local
                    + ^[param.use.pos]param
                    + ^[global.use.pos]global
                    + ^[unknown.use.pos]unknown;
            }
        )");

        checkSymbol("global.decl.pos", SymbolDeclType::GlobalVariable, "global");
        checkSymbol("param.decl.pos", SymbolDeclType::Parameter, "param");
        checkSymbol("local.decl.pos", SymbolDeclType::LocalVariable, "local");
        checkSymbol("local.use.pos", SymbolDeclType::LocalVariable, "local");
        checkSymbol("param.use.pos", SymbolDeclType::Parameter, "param");
        checkSymbol("global.use.pos", SymbolDeclType::GlobalVariable, "global");
        checkSymbol("unknown.use.pos", SymbolDeclType::GlobalVariable, "unknown", true);
    }

    SECTION("BlockName")
    {
        CompileLabelledSource(R"(
            uniform ^[ubo.decl.pos]UBO
            {
                int ^[ubo.member.decl.pos]value;
            } ^[ubo.instance.decl.pos]block;
            buffer ^[ssbo.decl.pos]SSBO
            {
                int ^[ssbo.member.decl.pos]test;
            };
            void foo()
            {
                ^[ssbo.member.use.pos]test = ^[ubo.instance.use.pos]block.^[ubo.member.use.pos]value;
            }
        )");

        checkSymbol("ubo.decl.pos", SymbolDeclType::Block, "UBO");
        checkSymbol("ssbo.decl.pos", SymbolDeclType::Block, "SSBO");
        checkSymbol("ubo.member.decl.pos", SymbolDeclType::BlockMember, "value");
        checkSymbol("ubo.instance.decl.pos", SymbolDeclType::BlockInstance, "block");
        checkSymbol("ssbo.member.decl.pos", SymbolDeclType::BlockMember, "test");
        checkSymbol("ssbo.member.use.pos", SymbolDeclType::BlockMember, "test");
        checkSymbol("ubo.instance.use.pos", SymbolDeclType::BlockInstance, "block");
        checkSymbol("ubo.member.use.pos", SymbolDeclType::BlockMember, "value");
    }

    SECTION("FunctionName")
    {
        CompileLabelledSource(R"(
            void ^[func.decl.pos]foo()
            {
            }

            void bar()
            {
                ^[func.use.pos]foo();
                ^[unknown.use.pos]unknown();
            }
        )");

        checkSymbol("func.decl.pos", SymbolDeclType::Function, "foo");
        checkSymbol("func.use.pos", SymbolDeclType::Function, "foo");
        checkSymbol("unknown.use.pos", SymbolDeclType::Function, "unknown", true);
    }

    SECTION("SwizzleName")
    {
        CompileLabelledSource(R"(
            void foo()
            {
                vec4 v;
                v.^[swizzle1.use.pos]x;
                v.^[swizzle2.use.pos]xyzw;
            }
        )");

        checkSymbol("swizzle1.use.pos", SymbolDeclType::Swizzle, "x");
        checkSymbol("swizzle2.use.pos", SymbolDeclType::Swizzle, "xyzw");
    }
}
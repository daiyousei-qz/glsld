#include "ServerTestFixture.h"

#include "SymbolQuery.h"

using namespace glsld;

TEST_CASE_METHOD(ServerTestFixture, "SymbolQueryTest")
{
    auto checkSymbol = [this](const ServerTestContext& ctx, StringView label, SymbolDeclType type, StringView name,
                              bool unknown = false) {
        auto queryResult = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition(label));
        REQUIRE(queryResult.has_value());
        REQUIRE(queryResult->symbolType == type);
        REQUIRE(queryResult->spelledText == name);
    };

    SECTION("CursorOnPP")
    {
        auto sourceText = R"(
        #include ^[pos.1]"test^[pos.2].h"^[pos.3]
        #define ^[pos.4]MA^[pos.5]CRO^[pos.6] 1
    )";

        auto ctx = CompileLabelledSource(sourceText);

        auto result1 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.1"));
        REQUIRE(result1.has_value());
        REQUIRE(result1->symbolType == SymbolDeclType::HeaderName);
        REQUIRE(result1->spelledText == "\"test.h\"");

        auto result2 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.2"));
        REQUIRE(result2.has_value());
        REQUIRE(result2->symbolType == SymbolDeclType::HeaderName);
        REQUIRE(result2->spelledText == "\"test.h\"");

        auto result3 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.3"));
        REQUIRE(!result3.has_value());

        auto result4 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.4"));
        REQUIRE(result4.has_value());
        REQUIRE(result4->symbolType == SymbolDeclType::Macro);
        REQUIRE(result4->spelledText == "MACRO");

        auto result5 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.5"));
        REQUIRE(result5.has_value());
        REQUIRE(result5->symbolType == SymbolDeclType::Macro);
        REQUIRE(result5->spelledText == "MACRO");

        auto result6 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.6"));
        REQUIRE(!result6.has_value());
    }

    SECTION("CursorOnAST")
    {
        auto sourceText = R"(
        void ^[pos.1]ma^[pos.2]in^[pos.3](^[pos.4])
        ^[pos.5]{
        ^[pos.6]
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);

        auto result1 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.1"));
        REQUIRE(result1.has_value());
        REQUIRE(result1->spelledRange == ctx.GetRange("pos.1", "pos.3"));
        REQUIRE(result1->spelledText == "main");

        auto result2 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.2"));
        REQUIRE(result2.has_value());
        REQUIRE(result2->spelledRange == ctx.GetRange("pos.1", "pos.3"));
        REQUIRE(result2->spelledText == "main");

        auto result3 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.3"));
        REQUIRE(!result3.has_value());

        auto result4 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.4"));
        REQUIRE(!result4.has_value());

        auto result5 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.5"));
        REQUIRE(!result5.has_value());

        auto result6 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.6"));
        REQUIRE(!result6.has_value());
    }

    SECTION("CursorOnHeaderName")
    {
        auto sourceText = R"(
        #include ^[header.pos]"header.glsl"
    )";

        auto ctx = CompileLabelledSource(sourceText);

        auto queryResult = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("header.pos"));
        REQUIRE(queryResult.has_value());
        REQUIRE(queryResult->symbolType == SymbolDeclType::HeaderName);
        REQUIRE(queryResult->spelledText == "\"header.glsl\"");
    }

    SECTION("CursorOnTypeName")
    {
        auto sourceText = R"(
        struct ^[struct.decl.pos]S
        {
        };

        void foo()
        {
            ^[struct.access.pos]S s;
            ^[unknown.access.pos]unknown u;
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);

        checkSymbol(ctx, "struct.decl.pos", SymbolDeclType::Type, "S");
        checkSymbol(ctx, "struct.access.pos", SymbolDeclType::Type, "S");
        checkSymbol(ctx, "unknown.access.pos", SymbolDeclType::Type, "unknown", true);
    }

    SECTION("CursorOnVariableName")
    {
        auto sourceText = R"(
        int ^[global.decl.pos]global;
        int foo(int ^[param.decl.pos]param)
        {
            int ^[local.decl.pos]local;

            return ^[local.access.pos]local
                 + ^[param.access.pos]param
                 + ^[global.access.pos]global
                 + ^[unknown.access.pos]unknown;
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);

        checkSymbol(ctx, "global.decl.pos", SymbolDeclType::GlobalVariable, "global");
        checkSymbol(ctx, "param.decl.pos", SymbolDeclType::Parameter, "param");
        checkSymbol(ctx, "local.decl.pos", SymbolDeclType::LocalVariable, "local");
        checkSymbol(ctx, "local.access.pos", SymbolDeclType::LocalVariable, "local");
        checkSymbol(ctx, "param.access.pos", SymbolDeclType::Parameter, "param");
        checkSymbol(ctx, "global.access.pos", SymbolDeclType::GlobalVariable, "global");
        checkSymbol(ctx, "unknown.access.pos", SymbolDeclType::GlobalVariable, "unknown", true);
    }

    SECTION("CursorOnBlockName")
    {
        auto sourceText = R"(
        uniform ^[ubo.decl.pos]UBO
        {
            int ^[ubo.member.decl.pos]value;
        } ^[ubo.instance.decl.pos]block;
        buffer SSBO
        {
            int ^[ssbo.member.decl.pos]test;
        };
        void foo()
        {
            ^[ssbo.member.access.pos]test = ^[ubo.instance.access.pos]block.^[ubo.member.access.pos]value;
        }
        )";

        auto ctx = CompileLabelledSource(sourceText);

        checkSymbol(ctx, "ubo.decl.pos", SymbolDeclType::Block, "UBO");
        checkSymbol(ctx, "ubo.member.decl.pos", SymbolDeclType::BlockMember, "value");
        checkSymbol(ctx, "ubo.instance.decl.pos", SymbolDeclType::BlockInstance, "block");
        checkSymbol(ctx, "ssbo.member.decl.pos", SymbolDeclType::BlockMember, "test");
        checkSymbol(ctx, "ssbo.member.access.pos", SymbolDeclType::BlockMember, "test");
        checkSymbol(ctx, "ubo.instance.access.pos", SymbolDeclType::BlockInstance, "block");
        checkSymbol(ctx, "ubo.member.access.pos", SymbolDeclType::BlockMember, "value");
    }

    SECTION("CursorOnFunctionName")
    {
        auto sourceText = R"(
        void ^[func.decl.pos]foo()
        {
        }

        void bar()
        {
            ^[func.access.pos]foo();
            ^[unknown.access.pos]unknown();
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);

        checkSymbol(ctx, "func.decl.pos", SymbolDeclType::Function, "foo");
        checkSymbol(ctx, "func.access.pos", SymbolDeclType::Function, "foo");
        checkSymbol(ctx, "unknown.access.pos", SymbolDeclType::Function, "unknown", true);
    }

    SECTION("CursorOnStructFieldName")
    {
        auto sourceText = R"(
        struct S
        {
            int ^[member.decl.pos]member;
        };

        void foo()
        {
            S s;
            s.^[member.access.pos]member;
            s.^[unknown.access.pos]unknown;

            vec4 v;
            v.^[swizzle1.access.pos]x;
            v.^[swizzle2.access.pos]xyzw;
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);

        checkSymbol(ctx, "member.decl.pos", SymbolDeclType::StructMember, "member");
        checkSymbol(ctx, "member.access.pos", SymbolDeclType::StructMember, "member");
        checkSymbol(ctx, "unknown.access.pos", SymbolDeclType::StructMember, "unknown", true);
    }

    SECTION("CursorOnSwizzleName")
    {
        auto sourceText = R"(
        void foo()
        {
            vec4 v;
            v.^[swizzle1.access.pos]x;
            v.^[swizzle2.access.pos]xyzw;
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);

        checkSymbol(ctx, "swizzle1.access.pos", SymbolDeclType::Swizzle, "x");
        checkSymbol(ctx, "swizzle2.access.pos", SymbolDeclType::Swizzle, "xyzw");
    }
}
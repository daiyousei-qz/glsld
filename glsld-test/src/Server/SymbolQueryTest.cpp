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
        REQUIRE(queryResult->token.text == name);
    };

    SECTION("CursorNotHit")
    {
        auto sourceText = R"(
        void main(^[pos.1])^[pos.2]
        {^[pos.3]
         ^[pos.4]
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);

        auto result1 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.1"));
        auto result2 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.2"));
        auto result3 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.3"));
        auto result4 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.4"));
        REQUIRE(!result1.has_value());
        REQUIRE(!result2.has_value());
        REQUIRE(!result3.has_value());
        REQUIRE(!result4.has_value());
    }

    SECTION("CursorOnIdentifier")
    {
        auto sourceText = R"(
        void ^[pos.begin]ma^[pos.mid]in^[pos.end]()
        {
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);

        auto result1 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.begin"));
        REQUIRE(result1.has_value());
        REQUIRE(ctx.GetProvider().LookupExpandedTextRange(result1->token.id) == ctx.GetRange("pos.begin", "pos.end"));
        REQUIRE(result1->token.text == "main");

        auto result2 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.mid"));
        REQUIRE(result2.has_value());
        REQUIRE(ctx.GetProvider().LookupExpandedTextRange(result2->token.id) == ctx.GetRange("pos.begin", "pos.end"));
        REQUIRE(result2->token.text == "main");

        auto result3 = QuerySymbolByPosition(ctx.GetProvider(), ctx.GetPosition("pos.end"));
        REQUIRE(!result3.has_value());
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

    SECTION("CursorOnDotAccessName")
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
        checkSymbol(ctx, "swizzle1.access.pos", SymbolDeclType::Swizzle, "x");
        checkSymbol(ctx, "swizzle2.access.pos", SymbolDeclType::Swizzle, "xyzw");
    }
}
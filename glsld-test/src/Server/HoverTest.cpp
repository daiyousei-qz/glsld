#include "ServerTestFixture.h"

#include "SourceText.h"
#include "Hover.h"

using namespace glsld;

TEST_CASE_METHOD(ServerTestFixture, "Hover")
{
    auto checkHover = [this](const ServerTestContext& ctx, StringView label, SymbolAccessType type, StringView name,
                             bool unknown = false) {
        auto hover = ComputeHoverContent(ctx.GetProvider(), ctx.GetPosition(label));
        REQUIRE(hover.has_value());
        REQUIRE(hover->type == type);
        REQUIRE(hover->name == name);
        if (unknown) {
            REQUIRE(hover->unknown);
        }
    };

    SECTION("Range")
    {
        auto sourceText = R"(
        void ^[pos.begin]ma^[pos.mid]in^[pos.end]()
        {
        }
    )";

        auto ctx = CompileLabelledSource(sourceText);

        auto hover1 = ComputeHoverContent(ctx.GetProvider(), ctx.GetPosition("pos.begin"));
        REQUIRE(hover1.has_value());
        REQUIRE(hover1->range == ctx.GetRange("pos.begin", "pos.end"));

        auto hover2 = ComputeHoverContent(ctx.GetProvider(), ctx.GetPosition("pos.mid"));
        REQUIRE(hover2.has_value());
        REQUIRE(hover1->range == ctx.GetRange("pos.begin", "pos.end"));

        auto hover3 = ComputeHoverContent(ctx.GetProvider(), ctx.GetPosition("pos.end"));
        REQUIRE(!hover3.has_value());
    }

    SECTION("TypeName")
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

        checkHover(ctx, "struct.decl.pos", SymbolAccessType::Type, "S");
        checkHover(ctx, "struct.access.pos", SymbolAccessType::Type, "S");
        checkHover(ctx, "unknown.access.pos", SymbolAccessType::Type, "unknown", true);
    }

    SECTION("VariableName")
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

        checkHover(ctx, "global.decl.pos", SymbolAccessType::GlobalVariable, "global");
        checkHover(ctx, "param.decl.pos", SymbolAccessType::Parameter, "param");
        checkHover(ctx, "local.decl.pos", SymbolAccessType::LocalVariable, "local");
        checkHover(ctx, "local.access.pos", SymbolAccessType::LocalVariable, "local");
        checkHover(ctx, "param.access.pos", SymbolAccessType::Parameter, "param");
        checkHover(ctx, "global.access.pos", SymbolAccessType::GlobalVariable, "global");
        checkHover(ctx, "unknown.access.pos", SymbolAccessType::GlobalVariable, "unknown", true);
    }

    SECTION("FunctionName")
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

        checkHover(ctx, "func.decl.pos", SymbolAccessType::Function, "foo");
        checkHover(ctx, "func.access.pos", SymbolAccessType::Function, "foo");
        checkHover(ctx, "unknown.access.pos", SymbolAccessType::Function, "unknown", true);
    }

    SECTION("DotAccessName")
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

        checkHover(ctx, "member.decl.pos", SymbolAccessType::MemberVariable, "member");
        checkHover(ctx, "member.access.pos", SymbolAccessType::MemberVariable, "member");
        checkHover(ctx, "unknown.access.pos", SymbolAccessType::MemberVariable, "unknown", true);
        checkHover(ctx, "swizzle1.access.pos", SymbolAccessType::Swizzle, "x");
        checkHover(ctx, "swizzle2.access.pos", SymbolAccessType::Swizzle, "xyzw");
    }
}
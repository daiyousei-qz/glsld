#include "ServerTestFixture.h"

#include "Feature/SemanticTokens.h"

using namespace glsld;

static auto MockSemanticTokens(ServerTestContext& ctx, const SemanticTokenConfig& config = {})
    -> std::vector<SemanticTokenInfo>
{
    return CollectSemanticTokens(config, ctx.GetProvider());
}

TEST_CASE_METHOD(ServerTestFixture, "SemanticTokenTest")
{
    auto checkSemanticToken = [](ArrayView<SemanticTokenInfo> tokens, TextPosition begin, TextPosition end,
                                 SemanticTokenType type, SemanticTokenModifier modifier) {
        auto it = std::ranges::find_if(tokens, [=](const SemanticTokenInfo& token) {
            return token.line == begin.line && token.character == begin.character &&
                   token.length == end.character - begin.character;
        });
        REQUIRE(it != tokens.end());
        REQUIRE(it->type == type);
        REQUIRE(it->modifier == modifier);
    };

    SECTION("MacroName")
    {
        auto sourceText     = R"(
        #define ^[MACRO.def.begin]MACRO^[MACRO.def.end] 1
        #define ^[MACRO_FN.def.begin]MACRO_FN^[MACRO_FN.def.end]() 2
        ^[MACRO.use.begin]MACRO^[MACRO.use.end];
        ^[MACRO_FN.use.begin]MACRO_FN^[MACRO_FN.use.end]();
        #undef ^[MACRO.undef.begin]MACRO^[MACRO.undef.end]
        )";
        auto ctx            = CompileLabelledSource(sourceText);
        auto semanticTokens = MockSemanticTokens(ctx);

        checkSemanticToken(semanticTokens, ctx.GetPosition("MACRO.def.begin"), ctx.GetPosition("MACRO.def.end"),
                           SemanticTokenType::Macro, SemanticTokenModifier::None);
        checkSemanticToken(semanticTokens, ctx.GetPosition("MACRO_FN.def.begin"), ctx.GetPosition("MACRO_FN.def.end"),
                           SemanticTokenType::Macro, SemanticTokenModifier::None);
        checkSemanticToken(semanticTokens, ctx.GetPosition("MACRO.use.begin"), ctx.GetPosition("MACRO.use.end"),
                           SemanticTokenType::Macro, SemanticTokenModifier::None);
        checkSemanticToken(semanticTokens, ctx.GetPosition("MACRO_FN.use.begin"), ctx.GetPosition("MACRO_FN.use.end"),
                           SemanticTokenType::Macro, SemanticTokenModifier::None);
        checkSemanticToken(semanticTokens, ctx.GetPosition("MACRO.undef.begin"), ctx.GetPosition("MACRO.undef.end"),
                           SemanticTokenType::Macro, SemanticTokenModifier::None);
    }

    SECTION("TypeName")
    {
        auto sourceText = R"(
        struct ^[struct.decl.begin]S^[struct.decl.end]
        {
        };

        void foo()
        {
            ^[struct.access.begin]S^[struct.access.end] s;
            ^[unknown.access.begin]unknown^[unknown.access.end] u;
        }
    )";

        auto ctx            = CompileLabelledSource(sourceText);
        auto semanticTokens = MockSemanticTokens(ctx);

        checkSemanticToken(semanticTokens, ctx.GetPosition("struct.decl.begin"), ctx.GetPosition("struct.decl.end"),
                           SemanticTokenType::Struct, SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, ctx.GetPosition("struct.access.begin"), ctx.GetPosition("struct.access.end"),
                           SemanticTokenType::Type, SemanticTokenModifier::None);
        checkSemanticToken(semanticTokens, ctx.GetPosition("unknown.access.begin"),
                           ctx.GetPosition("unknown.access.end"), SemanticTokenType::Type, SemanticTokenModifier::None);
    }

    SECTION("FunctionName")
    {
        auto sourceText     = R"(
        void ^[foo.decl.begin]foo^[foo.decl.end](S s)
        {
        }

        void ^[main.begin]main^[main.end]()
        {
            ^[foo.use.begin]foo^[foo.use.end]();
        }
    )";
        auto ctx            = CompileLabelledSource(sourceText);
        auto semanticTokens = MockSemanticTokens(ctx);

        checkSemanticToken(semanticTokens, ctx.GetPosition("foo.decl.begin"), ctx.GetPosition("foo.decl.end"),
                           SemanticTokenType::Function, SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, ctx.GetPosition("main.begin"), ctx.GetPosition("main.end"),
                           SemanticTokenType::Function, SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, ctx.GetPosition("foo.use.begin"), ctx.GetPosition("foo.use.end"),
                           SemanticTokenType::Function, SemanticTokenModifier::None);
    }

    SECTION("BlockName")
    {
        auto sourceText = R"(
        uniform ^[ubo.decl.begin]UBO^[ubo.decl.end]
        {
            int ^[x.decl.begin]x^[x.decl.end];
        } ^[block.decl.begin]block^[block.decl.end];
        void foo() {
            ^[block.use.begin]block^[block.use.end].^[x.use.begin]x^[x.use.end];
        }
    )";

        auto ctx            = CompileLabelledSource(sourceText);
        auto semanticTokens = MockSemanticTokens(ctx);

        checkSemanticToken(semanticTokens, ctx.GetPosition("ubo.decl.begin"), ctx.GetPosition("ubo.decl.end"),
                           SemanticTokenType::Type, SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, ctx.GetPosition("x.decl.begin"), ctx.GetPosition("x.decl.end"),
                           SemanticTokenType::Variable, SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, ctx.GetPosition("block.decl.begin"), ctx.GetPosition("block.decl.end"),
                           SemanticTokenType::Variable, SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, ctx.GetPosition("block.use.begin"), ctx.GetPosition("block.use.end"),
                           SemanticTokenType::Variable, SemanticTokenModifier::None);
        checkSemanticToken(semanticTokens, ctx.GetPosition("x.use.begin"), ctx.GetPosition("x.use.end"),
                           SemanticTokenType::Variable, SemanticTokenModifier::None);
    }

    SECTION("VariableName")
    {
        auto sourceText = R"(
        int ^[global.decl.begin]global^[global.decl.end];
        int foo(int ^[param.decl.begin]param^[param.decl.end])
        {
            int ^[local.decl.begin]local^[local.decl.end];

            return ^[local.access.begin]local^[local.access.end]
                 + ^[param.access.begin]param^[param.access.end]
                 + ^[global.access.begin]global^[global.access.end]
                 + ^[unknown.access.begin]unknown^[unknown.access.end];
        }
    )";

        auto ctx            = CompileLabelledSource(sourceText);
        auto semanticTokens = MockSemanticTokens(ctx);

        checkSemanticToken(semanticTokens, ctx.GetPosition("global.decl.begin"), ctx.GetPosition("global.decl.end"),
                           SemanticTokenType::Variable, SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, ctx.GetPosition("param.decl.begin"), ctx.GetPosition("param.decl.end"),
                           SemanticTokenType::Parameter, SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, ctx.GetPosition("local.decl.begin"), ctx.GetPosition("local.decl.end"),
                           SemanticTokenType::Variable, SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, ctx.GetPosition("local.access.begin"), ctx.GetPosition("local.access.end"),
                           SemanticTokenType::Variable, SemanticTokenModifier::None);
        checkSemanticToken(semanticTokens, ctx.GetPosition("param.access.begin"), ctx.GetPosition("param.access.end"),
                           SemanticTokenType::Parameter, SemanticTokenModifier::None);
        checkSemanticToken(semanticTokens, ctx.GetPosition("global.access.begin"), ctx.GetPosition("global.access.end"),
                           SemanticTokenType::Variable, SemanticTokenModifier::None);
        checkSemanticToken(semanticTokens, ctx.GetPosition("unknown.access.begin"),
                           ctx.GetPosition("unknown.access.end"), SemanticTokenType::Variable,
                           SemanticTokenModifier::None);
    }
}
#include "ServerTestFixture.h"

#include "Feature/SemanticTokens.h"

using namespace glsld;

static auto MockSemanticTokens(const ServerTestFixture& fixture, const SemanticTokenConfig& config = {})
    -> std::vector<SemanticTokenInfo>
{
    return CollectSemanticTokens(config, fixture.GetLanguageQueryInfo());
}

TEST_CASE_METHOD(ServerTestFixture, "SemanticTokenTest")
{
    auto checkSemanticToken = [&](ArrayView<SemanticTokenInfo> tokens, StringView labelBegin, StringView labelEnd,
                                  SemanticTokenType type,
                                  SemanticTokenModifier modifier = SemanticTokenModifier::None) {
        auto posBegin = GetLabelledPosition(labelBegin);
        auto posEnd   = GetLabelledPosition(labelEnd);
        auto it       = std::ranges::find_if(tokens, [=](const SemanticTokenInfo& token) {
            return token.line == posBegin.line && token.character == posBegin.character &&
                   token.length == posEnd.character - posBegin.character;
        });
        REQUIRE(it != tokens.end());
        REQUIRE(it->type == type);
        REQUIRE(it->modifier == modifier);
    };

    SECTION("MacroName")
    {
        CompileLabelledSource(R"(
            #define ^[MACRO.def.begin]MACRO^[MACRO.def.end] 1
            #define ^[MACRO_FN.def.begin]MACRO_FN^[MACRO_FN.def.end]() 2
            ^[MACRO.use.begin]MACRO^[MACRO.use.end];
            ^[MACRO_FN.use.begin]MACRO_FN^[MACRO_FN.use.end]();
            #undef ^[MACRO.undef.begin]MACRO^[MACRO.undef.end]
        )");

        auto semanticTokens = MockSemanticTokens(*this);
        checkSemanticToken(semanticTokens, "MACRO.def.begin", "MACRO.def.end", SemanticTokenType::Macro);
        checkSemanticToken(semanticTokens, "MACRO_FN.def.begin", "MACRO_FN.def.end", SemanticTokenType::Macro);
        checkSemanticToken(semanticTokens, "MACRO.use.begin", "MACRO.use.end", SemanticTokenType::Macro);
        checkSemanticToken(semanticTokens, "MACRO_FN.use.begin", "MACRO_FN.use.end", SemanticTokenType::Macro);
        checkSemanticToken(semanticTokens, "MACRO.undef.begin", "MACRO.undef.end", SemanticTokenType::Macro);
    }

    SECTION("TypeName")
    {
        CompileLabelledSource(R"(
            struct ^[struct.decl.begin]S^[struct.decl.end]
            {
            };

            void foo()
            {
                ^[struct.access.begin]S^[struct.access.end] s;
                ^[unknown.access.begin]unknown^[unknown.access.end] u;
            }
        )");

        auto semanticTokens = MockSemanticTokens(*this);
        checkSemanticToken(semanticTokens, "struct.decl.begin", "struct.decl.end", SemanticTokenType::Struct,
                           SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, "struct.access.begin", "struct.access.end", SemanticTokenType::Type);
        checkSemanticToken(semanticTokens, "unknown.access.begin", "unknown.access.end", SemanticTokenType::Type);
    }

    SECTION("FunctionName")
    {
        CompileLabelledSource(R"(
            void ^[foo.decl.begin]foo^[foo.decl.end](S s)
            {
            }

            void ^[main.begin]main^[main.end]()
            {
                ^[foo.use.begin]foo^[foo.use.end]();
            }
        )");

        auto semanticTokens = MockSemanticTokens(*this);
        checkSemanticToken(semanticTokens, "foo.decl.begin", "foo.decl.end", SemanticTokenType::Function,
                           SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, "main.begin", "main.end", SemanticTokenType::Function,
                           SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, "foo.use.begin", "foo.use.end", SemanticTokenType::Function);
    }

    SECTION("BlockName")
    {

        CompileLabelledSource(R"(
            uniform ^[ubo.decl.begin]UBO^[ubo.decl.end]
            {
                int ^[x.decl.begin]x^[x.decl.end];
            } ^[block.decl.begin]block^[block.decl.end];
            void foo() {
                ^[block.use.begin]block^[block.use.end].^[x.use.begin]x^[x.use.end];
            }
        )");

        auto semanticTokens = MockSemanticTokens(*this);
        checkSemanticToken(semanticTokens, "ubo.decl.begin", "ubo.decl.end", SemanticTokenType::Type,
                           SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, "x.decl.begin", "x.decl.end", SemanticTokenType::Variable,
                           SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, "block.decl.begin", "block.decl.end", SemanticTokenType::Variable,
                           SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, "block.use.begin", "block.use.end", SemanticTokenType::Variable);
        checkSemanticToken(semanticTokens, "x.use.begin", "x.use.end", SemanticTokenType::Variable);
    }

    SECTION("VariableName")
    {
        CompileLabelledSource(R"(
            int ^[global.decl.begin]global^[global.decl.end];
            int foo(int ^[param.decl.begin]param^[param.decl.end])
            {
                int ^[local.decl.begin]local^[local.decl.end];

                return ^[local.access.begin]local^[local.access.end]
                    + ^[param.access.begin]param^[param.access.end]
                    + ^[global.access.begin]global^[global.access.end]
                    + ^[unknown.access.begin]unknown^[unknown.access.end];
            }
        )");

        auto semanticTokens = MockSemanticTokens(*this);
        checkSemanticToken(semanticTokens, "global.decl.begin", "global.decl.end", SemanticTokenType::Variable,
                           SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, "param.decl.begin", "param.decl.end", SemanticTokenType::Parameter,
                           SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, "local.decl.begin", "local.decl.end", SemanticTokenType::Variable,
                           SemanticTokenModifier::Declaration);
        checkSemanticToken(semanticTokens, "local.access.begin", "local.access.end", SemanticTokenType::Variable);
        checkSemanticToken(semanticTokens, "param.access.begin", "param.access.end", SemanticTokenType::Parameter);
        checkSemanticToken(semanticTokens, "global.access.begin", "global.access.end", SemanticTokenType::Variable);
        checkSemanticToken(semanticTokens, "unknown.access.begin", "unknown.access.end", SemanticTokenType::Variable);
    }
}
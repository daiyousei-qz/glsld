#include "ServerTestFixture.h"

#include "Feature/SemanticTokens.h"

#include <algorithm>

using namespace glsld;

static auto MockSemanticTokens(const ServerTestFixture& fixture, const SemanticTokenConfig& config = {.enable = true})
    -> std::vector<SemanticTokenInfo>
{
    return CollectSemanticTokens(config, fixture.GetLanguageQueryInfo());
}

static auto ApplySemanticTokenEdits(std::vector<lsp::uinteger> data,
                                    const std::vector<lsp::SemanticTokensEdit>& edits)
    -> std::vector<lsp::uinteger>
{
    std::vector<lsp::SemanticTokensEdit> sortedEdits(edits.begin(), edits.end());
    std::ranges::sort(sortedEdits, [](const auto& lhs, const auto& rhs) {
        return lhs.start > rhs.start;
    });

    for (const auto& edit : sortedEdits) {
        auto start = static_cast<std::size_t>(edit.start);
        auto end   = start + static_cast<std::size_t>(edit.deleteCount);
        auto startIt = data.begin() + static_cast<std::vector<lsp::uinteger>::difference_type>(start);
        auto endIt   = data.begin() + static_cast<std::vector<lsp::uinteger>::difference_type>(end);
        auto insertPos = data.erase(startIt, endIt);
        data.insert(insertPos, edit.data.begin(), edit.data.end());
    }
    return data;
}

TEST_CASE_METHOD(ServerTestFixture, "SemanticTokenTest")
{
    auto checkSemanticToken = [&](ArrayView<SemanticTokenInfo> tokens, StringView labelBegin, StringView labelEnd,
                                  SemanticTokenType type, SemanticTokenModifierBits modifiers = {}) {
        auto posBegin = GetLabelledPosition(labelBegin);
        auto posEnd   = GetLabelledPosition(labelEnd);
        auto it       = std::ranges::find_if(tokens, [=](const SemanticTokenInfo& token) {
            return token.line == posBegin.line && token.character == posBegin.character &&
                   token.length == posEnd.character - posBegin.character;
        });
        REQUIRE(it != tokens.end());
        REQUIRE(it->type == type);
        REQUIRE(it->modifiers == modifiers);
    };

    SECTION("Config")
    {
        CompileLabelledSource(R"(
            void foo()
            {
            }
        )");
        auto semanticTokens = MockSemanticTokens(*this, SemanticTokenConfig{.enable = false});
        REQUIRE(semanticTokens.empty());
    }

    SECTION("HeaderName")
    {
        CompileLabelledSource(R"(
            #include ^[header1.begin]<header1.h>^[header1.end]
            #include ^[header2.begin]"header2.h"^[header2.end]
        )");

        auto semanticTokens = MockSemanticTokens(*this);
        checkSemanticToken(semanticTokens, "header1.begin", "header1.end", SemanticTokenType::String);
        checkSemanticToken(semanticTokens, "header2.begin", "header2.end", SemanticTokenType::String);
    }

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

    SECTION("DeltaEdits")
    {
        SemanticTokensState state;
        lsp::SemanticTokensParams fullParams = {};
        CompileLabelledSource(R"(
            int ^[value.begin]value^[value.end] = 1;
        )");
        auto fullTokens = HandleSemanticTokens({.enable = true}, GetLanguageQueryInfo(), state, fullParams);
        REQUIRE(!fullTokens.resultId.empty());

        CompileLabelledSource(R"(
            int ^[value.begin]value^[value.end] = 2;
            int ^[other.begin]other^[other.end] = value;
        )");
        lsp::SemanticTokensDeltaParams deltaParams = {.previousResultId = fullTokens.resultId};
        auto deltaTokens = HandleSemanticTokensDelta({.enable = true}, GetLanguageQueryInfo(), state, deltaParams);
        REQUIRE(!deltaTokens.resultId.empty());
        REQUIRE(!deltaTokens.edits.empty());

        auto mergedData = ApplySemanticTokenEdits(fullTokens.data, deltaTokens.edits);
        SemanticTokensState expectedState;
        auto expectedTokens = HandleSemanticTokens({.enable = true}, GetLanguageQueryInfo(), expectedState, fullParams);
        REQUIRE(mergedData == expectedTokens.data);
    }
}

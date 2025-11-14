#include "ServerTestFixture.h"

#include "Feature/SemanticTokens.h"

using namespace glsld;

static auto MockSemanticTokens(const ServerTestFixture& fixture, const SemanticTokenConfig& config = {.enable = true})
    -> std::vector<SemanticTokenInfo>
{
    return CollectSemanticTokens(config, fixture.GetLanguageQueryInfo());
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
}

TEST_CASE_METHOD(ServerTestFixture, "SemanticTokenDeltaTest")
{
    SECTION("DeltaEnabled")
    {
        CompileLabelledSource(R"(
            void foo()
            {
            }
        )");
        
        auto options = GetSemanticTokensOptions(SemanticTokenConfig{.enable = true});
        REQUIRE(options.has_value());
        REQUIRE(options->full.delta == true);
    }

    SECTION("DeltaWithSameContent")
    {
        CompileLabelledSource(R"(
            int x;
        )");
        
        // Get initial tokens
        lsp::SemanticTokensParams params;
        params.textDocument.uri = "test://test.glsl";
        auto initialTokens = HandleSemanticTokens(SemanticTokenConfig{.enable = true}, 
                                                   GetLanguageQueryInfo(), params);
        REQUIRE(!initialTokens.resultId.empty());
        REQUIRE(!initialTokens.data.empty());
        
        // Get delta with same content (no changes)
        lsp::SemanticTokensDeltaParams deltaParams;
        deltaParams.textDocument.uri = "test://test.glsl";
        deltaParams.previousResultId = initialTokens.resultId;
        auto delta = HandleSemanticTokensDelta(SemanticTokenConfig{.enable = true}, 
                                               GetLanguageQueryInfo(), deltaParams);
        REQUIRE(!delta.resultId.empty());
        REQUIRE(delta.edits.empty());  // No changes, so no edits
    }

    SECTION("DeltaWithChangedContent")
    {
        // Get initial tokens with one variable
        CompileLabelledSource(R"(
            int x;
        )");
        
        lsp::SemanticTokensParams params;
        params.textDocument.uri = "test://test.glsl";
        auto initialTokens = HandleSemanticTokens(SemanticTokenConfig{.enable = true}, 
                                                   GetLanguageQueryInfo(), params);
        auto previousResultId = initialTokens.resultId;
        auto previousData = initialTokens.data;
        
        // Change content - add another variable
        CompileLabelledSource(R"(
            int x;
            int y;
        )");
        
        // Get delta
        lsp::SemanticTokensDeltaParams deltaParams;
        deltaParams.textDocument.uri = "test://test.glsl";
        deltaParams.previousResultId = previousResultId;
        auto delta = HandleSemanticTokensDelta(SemanticTokenConfig{.enable = true}, 
                                               GetLanguageQueryInfo(), deltaParams);
        
        REQUIRE(!delta.resultId.empty());
        REQUIRE(!delta.edits.empty());  // Should have edits due to changes
        
        // Apply the edits to verify correctness
        std::vector<lsp::uinteger> appliedData = previousData;
        for (const auto& edit : delta.edits) {
            // Remove deleteCount elements starting at start
            appliedData.erase(appliedData.begin() + edit.start, 
                            appliedData.begin() + edit.start + edit.deleteCount);
            // Insert new data at start
            appliedData.insert(appliedData.begin() + edit.start, 
                             edit.data.begin(), edit.data.end());
        }
        
        // Get fresh tokens for comparison
        auto newTokens = HandleSemanticTokens(SemanticTokenConfig{.enable = true}, 
                                             GetLanguageQueryInfo(), params);
        
        // The applied data should match the new tokens data
        REQUIRE(appliedData.size() == newTokens.data.size());
        REQUIRE(appliedData == newTokens.data);
    }

    SECTION("DeltaWithInvalidPreviousId")
    {
        CompileLabelledSource(R"(
            int x;
        )");
        
        // Request delta with invalid previous result ID
        lsp::SemanticTokensDeltaParams deltaParams;
        deltaParams.textDocument.uri = "test://test.glsl";
        deltaParams.previousResultId = "invalid-id-12345";
        auto delta = HandleSemanticTokensDelta(SemanticTokenConfig{.enable = true}, 
                                               GetLanguageQueryInfo(), deltaParams);
        
        REQUIRE(!delta.resultId.empty());
        REQUIRE(!delta.edits.empty());  // Should return full data as an edit
        // Should have one edit with all the data
        REQUIRE(delta.edits.size() == 1);
        REQUIRE(delta.edits[0].start == 0);
        REQUIRE(delta.edits[0].deleteCount == 0);
        REQUIRE(!delta.edits[0].data.empty());
    }
}
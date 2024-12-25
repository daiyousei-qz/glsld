#pragma once
#include "TokenMatcher.h"

#include "Basic/StringView.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/CompilerResult.h"
#include "Compiler/SyntaxToken.h"

#include <catch2/catch_all.hpp>

#include <vector>

namespace glsld
{
    class LexingTestFixture
    {
    private:
    public:
        LexingTestFixture()
        {
        }

        auto Compile(StringView sourceText) const -> std::unique_ptr<CompilerResult>
        {
            auto compiler = std::make_unique<CompilerInvocation>();
            compiler->SetNoStdlib(true);
            compiler->SetMainFileFromBuffer(sourceText);

            return compiler->CompileMainFile(nullptr, CompileMode::PreprocessOnly);
        }
    };

    class LexingTestCatchMatcher : public Catch::Matchers::MatcherBase<StringView>
    {
    private:
        const LexingTestFixture& fixture;
        std::vector<TokenMatcher> matchers;
        StringView matcherDesc;

    public:
        LexingTestCatchMatcher(const LexingTestFixture& fixture, std::vector<TokenMatcher> matchers,
                               StringView matcherDesc)
            : fixture(fixture), matchers(std::move(matchers)), matcherDesc(matcherDesc)
        {
        }

        auto match(const StringView& sourceText) const -> bool override
        {
            auto compilerResult = fixture.Compile(sourceText);

            auto tokens = compilerResult->GetUserFileTokens();
            if (tokens.size() != matchers.size()) {
                UNSCOPED_INFO("Token count mismatch");
                return false;
            }

            for (uint32_t i = 0; i < tokens.size(); ++i) {
                auto token = SyntaxToken{
                    .index = SyntaxTokenID{TranslationUnitID::UserFile, i},
                    .klass = tokens[i].klass,
                    .text  = tokens[i].text,
                };
                if (!matchers[i].Match(token)) {
                    UNSCOPED_INFO("Token mismatch at index " + std::to_string(i));
                    return false;
                }
            }

            return true;
        }

        auto describe() const -> std::string override
        {
            return matcherDesc.Str();
        }
    };
} // namespace glsld

#define GLSLD_CHECK_TOKENS(SRC, ...)                                                                                   \
    do {                                                                                                               \
        auto matchers =                                                                                                \
            ::glsld::LexingTestCatchMatcher{*this, ::std::vector<::glsld::TokenMatcher>{__VA_ARGS__}, #__VA_ARGS__};   \
        CHECK_THAT(SRC, matchers);                                                                                     \
    } while (false)
#pragma once

#include "AstMatcher.h"

#include "Basic/StringView.h"
#include "Compiler/CompilerInvocation.h"

#include <catch2/catch_all.hpp>

namespace glsld
{
    class AstTestFixture
    {
    private:
        std::string sourceTemplate;
        std::function<AstMatcher(AstMatcher)> matcherTemplate;

    public:
        auto SetTestTemplate(fmt::format_string<StringView> sourceTemplate,
                             std::function<AstMatcher(AstMatcher)> matcherTemplate) -> void
        {
            this->sourceTemplate  = std::string(sourceTemplate.get().begin(), sourceTemplate.get().end());
            this->matcherTemplate = std::move(matcherTemplate);
        }

        auto WrapSource(StringView sourceText) const -> std::string
        {
            return fmt::format(fmt::runtime(sourceTemplate), sourceText);
        }

        auto WrapMatcher(AstMatcher matcher) const -> AstMatcher
        {
            return matcherTemplate(std::move(matcher));
        }

        auto Compile(StringView sourceText) const -> std::unique_ptr<CompilerResult>
        {
            std::string realSourceText = WrapSource(sourceText);

            auto compiler = std::make_unique<CompilerInvocation>();
            compiler->SetNoStdlib(true);
            compiler->SetMainFileFromBuffer(realSourceText);
            return compiler->CompileMainFile(nullptr, CompileMode::ParseOnly);
        }
    };

    class AstTestCatchMatcher : public Catch::Matchers::MatcherBase<StringView>
    {
    private:
        const AstTestFixture& fixture;
        AstMatcher matcher;
        StringView matcherDesc;

    public:
        AstTestCatchMatcher(const AstTestFixture& fixture, AstMatcher matcher, StringView matcherDesc)
            : fixture(fixture), matcher(fixture.WrapMatcher(std::move(matcher))), matcherDesc(matcherDesc)
        {
        }

        auto match(const StringView& sourceText) const -> bool override
        {
            auto compilerResult = fixture.Compile(sourceText);

            auto matchResult = matcher.Match(*compilerResult->GetUserFileAst());
            if (matchResult.IsSuccess()) {
                return true;
            }
            else {
                UNSCOPED_INFO(matchResult.GetFailedNode()->Print());
                UNSCOPED_INFO(matchResult.GetErrorTrace());
                return false;
            }
        }

        auto describe() const -> std::string override
        {
            return matcherDesc.Str();
        }
    };

} // namespace glsld

#define GLSLD_CHECK_AST(SRC, ...)                                                                                      \
    do {                                                                                                               \
        auto matcher = ::glsld::AstTestCatchMatcher{*this, __VA_ARGS__, #__VA_ARGS__};                                 \
        CHECK_THAT(SRC, matcher);                                                                                      \
    } while (false)
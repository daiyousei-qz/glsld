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
        std::string templateText;
        std::function<AstMatcher(AstMatcher)> matcherWrapper;

    public:
        auto SetTemplate(fmt::format_string<StringView> templateText,
                         std::function<AstMatcher(AstMatcher)> matcherWrapper) -> void
        {
            this->templateText   = std::string(templateText.get().begin(), templateText.get().end());
            this->matcherWrapper = std::move(matcherWrapper);
        }

        auto WrapMatcher(AstMatcher matcher) const -> AstMatcher
        {
            return matcherWrapper(std::move(matcher));
        }

        auto Compile(StringView sourceText) const -> std::unique_ptr<CompilerResult>
        {
            std::string realSourceText = fmt::format(fmt::runtime(templateText), sourceText);

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
#pragma once
#include "Basic/StringView.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/CompilerResult.h"
#include "Compiler/SyntaxToken.h"

namespace glsld
{
    class LexingTestFixture
    {
    private:
        std::unique_ptr<CompilerResult> compilerResult;

    public:
        LexingTestFixture(StringView sourceText)
        {
            std::unique_ptr<CompilerInvocation> compiler = std::make_unique<CompilerInvocation>();
            compiler->SetNoStdlib(true);
            compiler->SetMainFileFromBuffer(sourceText);

            compilerResult = compiler->CompileMainFile(nullptr, CompileMode::PreprocessOnly);
        }

        auto GetTokens() const noexcept -> ArrayView<RawSyntaxTokenEntry>
        {
            return compilerResult->GetUserFileTokens();
        }
    };

    class GlsldTest
    {
    public:
        GlsldTest()          = default;
        virtual ~GlsldTest() = default;

        virtual auto Positive() -> bool = 0;

        auto Negative() -> bool
        {
            return !Positive();
        }
    };

    template <TokenKlass ExpectedKlass>
    class LexSingleTokenTest : public GlsldTest
    {
    private:
        StringView sourceText;

    public:
        LexSingleTokenTest(StringView sourceText) : sourceText(sourceText)
        {
        }

        auto Positive() -> bool override
        {
            auto fixture = LexingTestFixture{sourceText};
            auto tokens  = fixture.GetTokens();

            if (tokens.size() != 2) {
                return false;
            }

            if (tokens[0].klass != ExpectedKlass) {
                return false;
            }

            if (tokens[0].text != sourceText) {
                return false;
            }

            return true;
        }
    };
} // namespace glsld
#pragma once
#include "SyntaxToken.h"
#include "Compiler.h"
#include "LexContext.h"
#include "PreprocessContext.h"
#include "MacroExpansion.h"
#include "CompilerTrace.h"

namespace glsld
{
    class PPTokenScanner;

    enum class PreprocessorState
    {
        // The lexing should be performed normally.
        Default,

        // The lexing should be disabled because of conditional compilation.
        Inactive,

        // A # in the beginning of a line is encountered. Expecting a PP directive or EOF.
        ExpectDirective,

        // A regular PP directive has been parsed. Expecting following tokens for the directive.
        ExpectDefaultDirectiveTail,

        // An include directive has been parsed. Expecting a header name.
        ExpectIncludeDirectiveTail,
    };

    struct PPConditionalInfo
    {
        // If the current conditional directive active, this flag is set to true.
        bool active;

        // If an active branch has been encountered, this flag is set to true.
        bool seenActiveBranch;

        // If an #else directive has been encountered, this flag is set to true.
        bool seenElse;
    };

    // The preprocessor acts as a state machine which accepts PP tokens from the Tokenizer and
    // transforms them according to the current state. The fully preproccessed tokens are then
    // converted to RawSyntaxTokens and registered to the LexContext.
    class Preprocessor final
    {
    public:
        Preprocessor(CompilerObject& compilerObject, FileID fileId)
            : compilerObject(compilerObject),
              macroExpansionProcessor(compilerObject.GetPreprocessContext(), ExpandToLexContextCallback{*this})
        {
        }

        auto GetState() const noexcept -> PreprocessorState
        {
            return state;
        }

        // This flag instructs the tokenizer that we are lexing in an active region.
        // If this flag is set false, we are in a region disabled by conditional directives and thus
        // tokenizer should avoid expensive lexing. Instead it should scan for the next conditional
        // directive that may close the region.
        auto InActiveRegion() const noexcept -> bool
        {
            return state != PreprocessorState::Inactive;
        }

        // This flag instructs the tokenizer to lex header name.
        // This flag is only set true when we are working on #include directive. In typical context,
        // <headerName> and "headerName" are not valid tokens as glsl does not support string.
        auto ShouldLexHeaderName() const noexcept -> bool
        {
            return state == PreprocessorState::ExpectIncludeDirectiveTail;
        }

        auto IssueToken(const PPTokenData& token) -> void
        {
            if (token.klass == TokenKlass::Comment) {
                // FIXME: Handle comments.
                // We ignore comments tokens for now.
            }
            else {
                DispatchTokenToHandler(token);
            }
        }

    private:
        auto DispatchTokenToHandler(const PPTokenData& token) -> void
        {
            GLSLD_ASSERT(token.klass != TokenKlass::Comment);
            switch (state) {
            case PreprocessorState::Default:
                HandleDefaultState(token);
                break;

            case PreprocessorState::Inactive:
                HandleInactiveState(token);
                break;

            case PreprocessorState::ExpectDirective:
                HandleExpectDirectiveState(token);
                break;

            case PreprocessorState::ExpectDefaultDirectiveTail:
            case PreprocessorState::ExpectIncludeDirectiveTail:
                HandleExpectDirectiveTailState(token);
                break;
            }
        }

        auto TransitionTo(PreprocessorState newState) -> void
        {
            // TODO: Validate the transition.
            state = newState;
        }

        auto RedirectIncomingToken(PreprocessorState newState, const PPTokenData& token) -> void
        {
            TransitionTo(newState);
            DispatchTokenToHandler(token);
        }

        auto HandleDefaultState(const PPTokenData& token) -> void
        {
            if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
                state = PreprocessorState::ExpectDirective;
            }
            else if (token.klass == TokenKlass::Eof) {
                macroExpansionProcessor.Finalize();
                if (compilerObject.GetPreprocessContext().GetIncludeDepth() == 0) {
                    // We are done with the main file. Insert an EOF token.
                    compilerObject.GetLexContext().AddToken(token.klass, token.range, token.text);
                }
            }
            else {
                macroExpansionProcessor.Feed(token);
            }
        }

        auto HandleInactiveState(const PPTokenData& token) -> void
        {
            if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
                TransitionTo(PreprocessorState::ExpectDirective);
            }
            else if (token.klass == TokenKlass::Eof) {
                // FIXME: Unterminated inactive region. Report error.
            }
            else {
                // Ignore all other tokens since we are in an inactive region.
            }
        }

        auto HandleExpectDirectiveState(const PPTokenData& token) -> void
        {
            if (token.klass == TokenKlass::Eof || token.isFirstTokenOfLine) {
                // Empty directive.
                if (conditionalStack.empty() || conditionalStack.back().active) {
                    RedirectIncomingToken(PreprocessorState::Default, token);
                }
                else {
                    RedirectIncomingToken(PreprocessorState::Inactive, token);
                }
            }
            else if (token.klass == TokenKlass::Identifier) {
                // A PP directive parsed.
                directiveToken = token;
                if (conditionalStack.empty() || conditionalStack.back().active) {
                    if (directiveToken->text == "include") {
                        TransitionTo(PreprocessorState::ExpectIncludeDirectiveTail);
                    }
                    else {
                        TransitionTo(PreprocessorState::ExpectDefaultDirectiveTail);
                    }
                }
                else {
                    if (directiveToken->text == "elif" || directiveToken->text == "else" ||
                        directiveToken->text == "endif") {
                        // These directives may change the state of the conditional stack.
                        TransitionTo(PreprocessorState::ExpectDefaultDirectiveTail);
                    }
                    else {
                        // Other directives are not skipped in inactive regions.
                        TransitionTo(PreprocessorState::Inactive);
                        directiveToken = std::nullopt;
                    }
                }
            }
            else {
                // A bad directive.
                TransitionTo(PreprocessorState::ExpectDefaultDirectiveTail);
            }
        }

        auto HandleExpectDirectiveTailState(const PPTokenData& token) -> void
        {
            if (token.klass == TokenKlass::Eof || token.isFirstTokenOfLine) {
                // Finish processing the directive.
                HandleDirective(*directiveToken, directiveArgBuffer);
                directiveToken = std::nullopt;
                directiveArgBuffer.clear();

                // Redirect the token to the default state.
                if (conditionalStack.empty() || conditionalStack.back().active) {
                    RedirectIncomingToken(PreprocessorState::Default, token);
                }
                else {
                    RedirectIncomingToken(PreprocessorState::Inactive, token);
                }
            }
            else {
                directiveArgBuffer.push_back(token);
            }
        }

        auto HandleDirective(const PPTokenData& directiveToken, ArrayView<PPTokenData> restTokens) -> void;
        auto HandleIncludeDirective(PPTokenScanner& scanner) -> void;
        auto HandleDefineDirective(PPTokenScanner& scanner) -> void;
        auto HandleUndefDirective(PPTokenScanner& scanner) -> void;
        auto HandleIfDirective(PPTokenScanner& scanner) -> void;
        auto HandleIfdefDirective(PPTokenScanner& scanner, bool isNDef) -> void;
        auto HandleElseDirective(PPTokenScanner& scanner) -> void;
        auto HandleElifDirective(PPTokenScanner& scanner) -> void;
        auto HandleEndifDirective(PPTokenScanner& scanner) -> void;

        CompilerObject& compilerObject;

        PreprocessorState state = PreprocessorState::Default;

        struct ExpandToLexContextCallback
        {
            ExpandToLexContextCallback(Preprocessor& pp) : pp(pp)
            {
            }

            auto OnYieldToken(const PPTokenData& token) -> void
            {
                TraceLexTokenIssued(token);
                pp.compilerObject.GetLexContext().AddToken(token.klass, token.range, token.text);
            }

            Preprocessor& pp;
        };

        MacroExpansionProcessor<ExpandToLexContextCallback> macroExpansionProcessor;

        std::optional<PPTokenData> directiveToken;
        std::vector<PPTokenData> directiveArgBuffer;

        std::vector<PPConditionalInfo> conditionalStack;
    };

} // namespace glsld
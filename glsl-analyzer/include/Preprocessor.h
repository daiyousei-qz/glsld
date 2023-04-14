#pragma once
#include "PPCallback.h"
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
    //
    class Preprocessor final
    {
    public:
        Preprocessor(CompilerObject& compilerObject, PPCallback* callback,
                     std::optional<TextRange> includeExpansionRange)
            : compilerObject(compilerObject), callback(callback), state(PreprocessorState::Default),
              macroExpansionProcessor(compilerObject.GetPreprocessContext(), ExpandToLexContextCallback{*this}),
              includeExpansionRange(includeExpansionRange), directiveToken(), directiveArgBuffer(), conditionalStack()
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

        // Issue a PP token to the preprocessor. The token will be processed according to the current state.
        auto IssueToken(const PPToken& token) -> void
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
        // Possible transitions:
        // - Default -> ExpectDirective (See # from the beginning of a line, expect a PP directive)
        // - Inactive -> ExpectDirective (See # from the beginning of a line, expect a PP directive)
        // - ExpectDirective -> Default (Empty directive parsed)
        // - ExpectDirective -> Inactive (Current PP directive cannot exit the inactive region)
        // - ExpectDirective -> ExpectDefaultDirectiveTail (A PP directive parsed, expect following tokens)
        // - ExpectDirective -> ExpectIncludeDirectiveTail (A #include directive parsed, expect following tokens)
        // - ExpectDefaultDirectiveTail -> Default (A PP directive handled)
        // - ExpectIncludeDirectiveTail -> Default (A #include directive handled)
        // - ExpectDefaultDirectiveTail -> Inactive (A PP directive handled, entering an inactive region)
        auto TransitionTo(PreprocessorState newState) -> void
        {
            state = newState;
        }

        //
        auto RedirectIncomingToken(PreprocessorState newState, const PPToken& token) -> void
        {
            TransitionTo(newState);
            DispatchTokenToHandler(token);
        }

        // Dispatch the token to the appropriate handler based on the current state.
        // The handler (AcceptOnXXXState) will then update the state and act accordingly.
        auto DispatchTokenToHandler(const PPToken& token) -> void;

        auto AcceptOnDefaultState(const PPToken& token) -> void;
        auto AcceptOnInactiveState(const PPToken& token) -> void;
        auto AcceptOnExpectDirectiveState(const PPToken& token) -> void;
        auto AcceptOnExpectDirectiveTailState(const PPToken& token) -> void;

        auto HandleDirective(const PPToken& directiveToken, ArrayView<PPToken> restTokens) -> void;
        auto HandleIncludeDirective(PPTokenScanner& scanner) -> void;
        auto HandleDefineDirective(PPTokenScanner& scanner) -> void;
        auto HandleUndefDirective(PPTokenScanner& scanner) -> void;
        auto HandleIfDirective(PPTokenScanner& scanner) -> void;
        auto HandleIfdefDirective(PPTokenScanner& scanner, bool isNDef) -> void;
        auto HandleElseDirective(PPTokenScanner& scanner) -> void;
        auto HandleElifDirective(PPTokenScanner& scanner) -> void;
        auto HandleEndifDirective(PPTokenScanner& scanner) -> void;

        struct ExpandToLexContextCallback final : public EmptyMacroExpansionCallback
        {
            ExpandToLexContextCallback(Preprocessor& pp) : pp(pp)
            {
            }

            auto OnYieldToken(const PPToken& token) -> void
            {
                const TextRange* expandedRange = nullptr;
                if (pp.includeExpansionRange) {
                    expandedRange = &*pp.includeExpansionRange;
                }
                else if (expansionDepth > 0) {
                    expandedRange = &firstExpansionRange;
                }
                else {
                    expandedRange = &token.spelledRange;
                }

                GLSLD_TRACE_TOKEN_ISSUED(token, *expandedRange);
                pp.compilerObject.GetLexContext().AddToken(token, *expandedRange);
            }

            auto OnEnterMacroExpansion(const PPToken& macroUse) -> void
            {
                expansionDepth += 1;
                if (expansionDepth == 1) {
                    firstExpansionRange = TextRange{macroUse.spelledRange.start};
                }

                if (pp.callback) {
                    pp.callback->OnMacroExpansion(macroUse);
                }
            }

            auto OnExitMacroExpansion(const PPToken& macroUse) -> void
            {
                expansionDepth -= 1;
            }

            Preprocessor& pp;
            int expansionDepth = 0;
            TextRange firstExpansionRange;
        };

        // The compiler object that of the current compilation.
        CompilerObject& compilerObject;

        // The callback interface for the preprocessor.
        PPCallback* callback;

        // The current state of the preprocessor.
        PreprocessorState state;

        // The macro expansion processor.
        MacroExpansionProcessor<ExpandToLexContextCallback> macroExpansionProcessor;

        // The token expansion range in the main file.
        // - If we are at an included file, this is the expanded range of all tokens.
        // - If we are at the main file, this should always be std::nullopt.
        std::optional<TextRange> includeExpansionRange;

        // The current PP directive token being processed.
        std::optional<PPToken> directiveToken;

        // This buffer stores the tokens following the PP directive.
        std::vector<PPToken> directiveArgBuffer;

        // This stack stores all information about the conditional directives.
        std::vector<PPConditionalInfo> conditionalStack;
    };

} // namespace glsld
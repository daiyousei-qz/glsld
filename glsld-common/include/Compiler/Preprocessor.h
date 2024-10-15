#pragma once
#include "Compiler/PPCallback.h"
#include "Compiler/SourceContext.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/CompilerObject.h"
#include "Compiler/LexContext.h"
#include "Compiler/MacroExpansion.h"
#include "Compiler/CompilerTrace.h"

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

    // The preprocessor acts as a state machine which accepts PP tokens issued by the Tokenizer and
    // transforms them according to the current state. The fully preproccessed tokens are then
    // piped into the LexContext as a part of the token stream.
    class Preprocessor final
    {
    private:
        // The compiler object that of the current compilation.
        CompilerObject& compilerObject;

        // The callback interface for the preprocessor.
        PPCallback* callback;

        // The current state of the preprocessor.
        PreprocessorState state;

        struct ExpandToLexContextCallback final
        {
            Preprocessor& pp;
            int expansionDepth = 0;
            TextRange firstExpansionRange;

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
        };

        struct ExpandToVectorCallback final
        {
            Preprocessor& pp;
            std::vector<PPToken>& tokenBuffer;

            ExpandToVectorCallback(Preprocessor& pp, std::vector<PPToken>& tokens) : pp(pp), tokenBuffer(tokens)
            {
            }

            auto OnYieldToken(const PPToken& token) -> void
            {
                tokenBuffer.push_back(token);
            }

            auto OnEnterMacroExpansion(const PPToken& macroUse) -> void
            {
                if (pp.callback) {
                    pp.callback->OnMacroExpansion(macroUse);
                }
            }

            auto OnExitMacroExpansion(const PPToken& macroUse) -> void
            {
            }
        };

        // The macro expansion processor.
        MacroExpansionProcessor<ExpandToLexContextCallback> macroExpansionProcessor;

        // The token expansion range in the main file.
        // - If we are at an included file, this is the expanded range of all tokens.
        // - If we are at the main file, this should always be std::nullopt.
        std::optional<TextRange> includeExpansionRange;

        // The current include depth. Preprocessor should stop including headers when this value exceeds the limit.
        size_t includeDepth = 0;

        // The current PP directive token being processed.
        std::optional<PPToken> directiveToken;

        // This buffer stores the tokens following the PP directive.
        std::vector<PPToken> directiveArgBuffer;

        // This stack stores all information about the conditional directives.
        std::vector<PPConditionalInfo> conditionalStack;

    public:
        Preprocessor(CompilerObject& compilerObject, PPCallback* callback,
                     std::optional<TextRange> includeExpansionRange, size_t includeDepth)
            : compilerObject(compilerObject), callback(callback), state(PreprocessorState::Default),
              macroExpansionProcessor(compilerObject.GetLexContext(), ExpandToLexContextCallback{*this}),
              includeExpansionRange(includeExpansionRange), includeDepth(includeDepth), directiveToken(),
              directiveArgBuffer(), conditionalStack()
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
        auto IssuePPToken(const PPToken& token) -> void
        {
            DispatchTokenToHandler(token);
        }

        auto PreprocessSourceFile(const SourceFileEntry& fileEntry) -> void;

    private:
        // Possible transitions:
        // - Default -> ExpectDirective (See # from the beginning of a line, expect a PP directive next)
        // - Inactive -> ExpectDirective (See # from the beginning of a line, expect a PP directive next)
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
        auto HandleElifDirective(PPTokenScanner& scanner) -> void;
        auto HandleElseDirective(PPTokenScanner& scanner) -> void;
        auto HandleEndifDirective(PPTokenScanner& scanner) -> void;

        // Consumes all tokens and returns the result of the preprocessing expression.
        // If the expression is not valid, this function will return false.
        auto EvaluatePPExpression(PPTokenScanner& scanner) -> bool;
    };

} // namespace glsld
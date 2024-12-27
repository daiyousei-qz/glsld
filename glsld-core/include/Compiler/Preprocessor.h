#pragma once
#include "Basic/AtomTable.h"
#include "Basic/SourceInfo.h"
#include "Compiler/CompilerInvocationState.h"
#include "Compiler/DiagnosticStream.h"
#include "Compiler/MacroTable.h"
#include "Compiler/PPCallback.h"
#include "Compiler/SourceManager.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/MacroExpansion.h"
#include "Compiler/CompilerTrace.h"
#include "Compiler/PPTokenScanner.h"

#include <unordered_map>

namespace glsld
{
    class TokenStream
    {
    private:
        std::vector<RawSyntaxTokenEntry> tokens;
        std::vector<RawCommentTokenEntry> comments;

    public:
        // Add a new token to the token stream.
        auto AddToken(const PPToken& token, TextRange expandedRange) -> void;

        // Add an EOF token to the token stream. This indicates end of a translation unit.
        auto AddEofToken(const PPToken& token, TextRange expandedRange) -> void;

        auto Export() -> std::pair<std::vector<RawSyntaxTokenEntry>, std::vector<RawCommentTokenEntry>>
        {
            return {std::move(tokens), std::move(comments)};
        }
    };

    enum class PreprocessorState
    {
        // The lexing should be performed normally.
        Default,

        // The lexing should be halted immediately.
        Halt,

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
    // piped into the token stream.
    class PreprocessStateMachine
    {
    private:
        CompilerInvocationState& compiler;

        SourceManager& sourceManager;

        AtomTable& atomTable;

        MacroTable& macroTable;

        DiagnosticReportor diagReporter;

        TokenStream& outputStream;

        // The callback interface for the preprocessor.
        PPCallback* callback = nullptr;

        // The current state of the preprocessor.
        PreprocessorState state = PreprocessorState::Default;

        struct ExpandToTokenStreamCallback final
        {
            PreprocessStateMachine& pp;
            int expansionDepth = 0;
            TextRange firstExpansionRange;

            ExpandToTokenStreamCallback(PreprocessStateMachine& pp) : pp(pp)
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

#if defined(GLSLD_ENABLE_COMPILER_TRACE)
                pp.compiler.GetCompilerTrace().TraceLexTokenIssued(token, *expandedRange);
#endif
                pp.OutputToken(token, *expandedRange);
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
            PreprocessStateMachine& pp;
            std::vector<PPToken>& tokenBuffer;

            ExpandToVectorCallback(PreprocessStateMachine& pp, std::vector<PPToken>& tokens)
                : pp(pp), tokenBuffer(tokens)
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
        MacroExpansionProcessor<ExpandToTokenStreamCallback> macroExpansionProcessor;

        // Lookup map from keyword atom to keyword klass
        std::unordered_map<AtomString, TokenKlass> keywordLookup;

        // The token expansion range in the main file.
        // - If we are at an included file, this is the expanded range of all tokens.
        // - If we are at the main file, this should always be std::nullopt.
        std::optional<TextRange> includeExpansionRange = std::nullopt;

        // The current include depth. PreprocessStateMachine should stop including headers when this value exceeds the
        // limit.
        size_t includeDepth = 0;

        // If the preprocessor should do fast scanning for version/extension directive.
        // In this mode, the preprocessor will halt the lexing when it sees the first non-comment/preprocessor token.
        bool versionScanningMode = false;

        // The current PP directive token being processed.
        std::optional<PPToken> directiveToken = std::nullopt;

        // This buffer stores the tokens following the PP directive.
        std::vector<PPToken> directiveArgBuffer = {};

        // This stack stores all information about the conditional directives.
        std::vector<PPConditionalInfo> conditionalStack = {};

    public:
        PreprocessStateMachine(CompilerInvocationState& compiler, TokenStream& outputStream, PPCallback* callback,
                               std::optional<TextRange> includeExpansionRange, size_t includeDepth)
            : compiler(compiler), sourceManager(compiler.GetSourceManager()), atomTable(compiler.GetAtomTable()),
              macroTable(compiler.GetMacroTable()), diagReporter(compiler.GetDiagnosticStream()),
              outputStream(outputStream), callback(callback),
              macroExpansionProcessor(compiler.GetAtomTable(), compiler.GetMacroTable(),
                                      ExpandToTokenStreamCallback{*this}),
              includeExpansionRange(includeExpansionRange), includeDepth(includeDepth)
        {
        }

        auto GetState() const noexcept -> PreprocessorState
        {
            return state;
        }

        auto IsVersionScanningMode() const noexcept -> bool
        {
            return versionScanningMode;
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

        // This flag instructs the tokenizer to halt lexing even if the source file is not ended.
        auto ShouldHaltLexing() const noexcept -> bool
        {
            return state == PreprocessorState::Halt;
        }

        // Issue a PP token to the preprocessor. The token will be processed according to the current state.
        auto FeedPPToken(const PPToken& token) -> void
        {
            DispatchTokenToHandler(token);

            if (token.klass == TokenKlass::Eof) {
                macroExpansionProcessor.Finalize();
                if (includeDepth == 0) {
                    // We are done with the main file. Insert an EOF token.
                    outputStream.AddEofToken(token, token.spelledRange);
                }
            }
        }

    protected:
        auto InitializeForVersionScanning() -> void
        {
            versionScanningMode = true;
        }
        auto InitializeForPP() -> void
        {
            versionScanningMode = false;
            InitializeKeywordLookup();
        }

        auto PreprocessSourceFile(FileID sourceFile) -> void;

    private:
        auto InitializeKeywordLookup() -> void;
        auto FixupKeywordTokenKlass(TokenKlass klass, AtomString text) -> TokenKlass;

        auto OutputToken(PPToken token, TextRange expandedRange) -> void
        {
            if (token.klass == TokenKlass::Identifier) {
                token.klass = FixupKeywordTokenKlass(token.klass, token.text);
            }

            outputStream.AddToken(token, expandedRange);
        }

        // Possible transitions:
        // - Default -> ExpectDirective (See # from the beginning of a line, expect a PP directive next)
        // - Default -> Halt (See the first token that is not comment nor part of preprocessor in version scanning mode)
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
        auto HandleExtensionDirective(PPTokenScanner& scanner) -> void;
        auto HandleVersionDirective(PPTokenScanner& scanner) -> void;
        auto HandleLineDirective(PPTokenScanner& scanner) -> void;

        // Consumes all tokens and returns the result of the preprocessing expression.
        // If the expression is not valid, this function will return false.
        auto EvaluatePPExpression(PPTokenScanner& scanner) -> bool;
    };

    class Preprocessor final : PreprocessStateMachine
    {
    private:
        CompilerInvocationState& compiler;
        TokenStream outputStream;

    public:
        Preprocessor(CompilerInvocationState& compiler, PPCallback* callback, bool versionScanningMode)
            : PreprocessStateMachine(compiler, outputStream, callback, std::nullopt, 0), compiler(compiler)
        {
            if (versionScanningMode) {
                InitializeForVersionScanning();
            }
            else {
                InitializeForPP();
            }
        }

        auto DoPreprocess(FileID sourceFile) -> void
        {
            PreprocessSourceFile(sourceFile);

            if (!IsVersionScanningMode()) {
                TranslationUnitID id;
                if (sourceFile.IsSystemPreamble()) {
                    id = TranslationUnitID::SystemPreamble;
                }
                else if (sourceFile.IsUserPreamble()) {
                    id = TranslationUnitID::UserPreamble;
                }
                else {
                    id = TranslationUnitID::UserFile;
                }

                auto [tokens, comments] = outputStream.Export();
                compiler.UpdateTokenArtifact(id, std::move(tokens), std::move(comments));
            }
        }
    };

} // namespace glsld
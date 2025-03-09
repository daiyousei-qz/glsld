#pragma once
#include "Basic/AtomTable.h"
#include "Basic/SourceInfo.h"
#include "Compiler/CompilerInvocationState.h"
#include "Compiler/DiagnosticStream.h"
#include "Compiler/MacroTable.h"
#include "Compiler/PPCallback.h"
#include "Compiler/SourceManager.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/CompilerTrace.h"
#include "Compiler/PPTokenScanner.h"

#include <unordered_map>
#include <unordered_set>

namespace glsld
{
    class TokenStream
    {
    private:
        std::vector<RawSyntaxToken> tokens;
        std::vector<RawCommentToken> comments;

    public:
        auto GetNextTokenIndex() const noexcept -> uint32_t
        {
            return tokens.size();
        }

        // Add a new token to the token stream.
        auto AddToken(const PPToken& token, TextRange expandedRange) -> void;

        // Add an EOF token to the token stream. This indicates end of a translation unit.
        auto AddEofToken(const PPToken& token, TextRange expandedRange) -> void;

        auto Export() -> std::pair<std::vector<RawSyntaxToken>, std::vector<RawCommentToken>>
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

    auto TokenizeOnce(StringView text) -> std::tuple<TokenKlass, StringView, StringView>;

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

        // The current translation unit ID.
        TranslationUnitID tuId = TranslationUnitID::UserFile;

        // The callback interface for the preprocessor.
        PPCallback* callback = nullptr;

        // The current state of the preprocessor.
        PreprocessorState state = PreprocessorState::Default;

        struct MacroExpansionInfo
        {
            SyntaxTokenID startTokenId;
        };

        // This callback is used to expand tokens in regular context.
        struct ExpandToTokenStreamCallback final
        {
            PreprocessStateMachine& pp;
            std::vector<MacroExpansionInfo> expansionStack;
            TextRange firstExpansionRange;

            ExpandToTokenStreamCallback(PreprocessStateMachine& pp) : pp(pp)
            {
            }

            auto OnYieldToken(const PPToken& token) -> void
            {
                const TextRange* expandedRange = nullptr;
                if (pp.includeExpansionRange) {
                    // If the token is from an included file, use the expanded range for that header.
                    expandedRange = &*pp.includeExpansionRange;
                }
                else if (!expansionStack.empty()) {
                    // If the token is from a macro expansion, use the expanded range for that macro.
                    expandedRange = &firstExpansionRange;
                }
                else {
                    // Otherwise, use the spelled range.
                    expandedRange = &token.spelledRange;
                }

#if defined(GLSLD_ENABLE_COMPILER_TRACE)
                pp.compiler.GetCompilerTrace().TraceLexTokenIssued(token, *expandedRange);
#endif
                pp.OutputToken(token, *expandedRange);
            }

            auto OnEnterMacroExpansion(const PPToken& macroUse) -> void
            {
                if (expansionStack.empty()) {
                    firstExpansionRange = TextRange{macroUse.spelledRange.start};
                }

                expansionStack.push_back(MacroExpansionInfo{pp.GetNextTokenId()});
            }

            auto OnExitMacroExpansion(const PPToken& macroUse) -> void
            {
                if (pp.callback) {
                    pp.callback->OnMacroExpansion(
                        macroUse, AstSyntaxRange{expansionStack.back().startTokenId, pp.GetNextTokenId()});
                }
                expansionStack.pop_back();
            }
        };

        // This callback is used to expand tokens in preprocessing directives.
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
            }

            auto OnExitMacroExpansion(const PPToken& macroUse) -> void
            {
                if (pp.callback) {
                    pp.callback->OnMacroExpansion(macroUse, {});
                }
            }
        };

        std::unordered_set<const MacroDefinition*> disabledMacros = {};

        class MacroExpansionProcessor
        {
        private:
            PreprocessStateMachine& pp;

            std::vector<PPToken>* outputBuffer = nullptr;

            // Tokens witheld for potential function-like macro expansion.
            std::vector<PPToken> witheldTokens;

            struct InvocationArgumentInfo
            {
                // The number of tokens originally provided as the macro invocation argument.
                size_t numArgumentToken;

                // The token that is being pasted, aka. the first token of the argument.
                PPToken pasteToken;

                // The index range of the expanded argument in tokens witheld.
                size_t indexBegin;
                size_t indexEnd;
            };

            std::vector<InvocationArgumentInfo> invocationArguments;

            // The pending function-like macro that is being witheld for expansion.
            const MacroDefinition* pendingInvokedMacro = nullptr;

            // The first token ID of the pending macro expansion, if any.
            SyntaxTokenID pendingExpansionTokenId = {};

            // Tracks unclosed '(' in the tokens witheld.
            int argLParenCounter = 0;

            std::unique_ptr<MacroExpansionProcessor> argProcessor = nullptr;

        public:
            MacroExpansionProcessor(PreprocessStateMachine& pp, std::vector<PPToken>* outputBuffer = nullptr)
                : pp(pp), outputBuffer(outputBuffer)
            {
            }

            // Feed a token to the macro expansion processor, which will be either:
            // 1. Witheld for potential macro expansion later.
            // 2. Trigger macro expansion.
            // 3. Yielded to the output.
            auto Feed(const PPToken& token) -> void;

            // Release all witheld tokens and finalize the macro expansion.
            auto Finalize() -> void;

        private:
            // Feed a token while collecting arguments for a function-like macro.
            auto FeedTokenWithMacroContext(const PPToken& token) -> void;
            auto FeedTokenWithoutMacroContext(const PPToken& token) -> void;
            auto FeedMacroExpansion(const PPToken& macroNameTok, const MacroDefinition& macroDefinition,
                                    ArrayView<PPToken> invocationTokens, ArrayView<InvocationArgumentInfo> args,
                                    SyntaxTokenID expansionStartId) -> void;

            auto NewPendingInvocationArgument() -> void
            {
                invocationArguments.push_back(InvocationArgumentInfo{
                    .numArgumentToken = 0,
                    .pasteToken       = {},
                    .indexBegin       = witheldTokens.size(),
                    .indexEnd         = witheldTokens.size(),
                });
                argProcessor = std::make_unique<MacroExpansionProcessor>(pp, &witheldTokens);
            }
            auto FinishPendingInvocationArgument() -> void
            {
                argProcessor->Finalize();
                argProcessor = nullptr;

                invocationArguments.back().indexEnd = witheldTokens.size();
            }
            auto RevokePendingMacroInvocation() -> void
            {
                GLSLD_ASSERT(pendingInvokedMacro);
                pendingInvokedMacro     = nullptr;
                pendingExpansionTokenId = {};
                YieldToken(witheldTokens[0]);
                witheldTokens.clear();
            }
            auto FinishPendingMacroInvocation() -> void
            {
                pendingInvokedMacro     = nullptr;
                pendingExpansionTokenId = {};
                witheldTokens.clear();
            }

            auto YieldToken(const PPToken& token) -> void
            {
                if (outputBuffer) {
                    outputBuffer->push_back(token);
                }
                else {
                    pp.OutputToken(token, token.spelledRange);
                }
            }
            auto EnterMacroExpansion(const PPToken& macroNameTok, const MacroDefinition& macroDefinition) -> void
            {
                // Disable this macro to avoid recursive expansion during rescan.
                pp.DisableMacro(macroDefinition);
            }
            auto ExitMacroExpansion(const PPToken& macroNameTok, const MacroDefinition& macroDefinition,
                                    AstSyntaxRange expansionRange) -> void
            {
                pp.EnableMacro(macroDefinition);

                if (pp.callback) {
                    pp.callback->OnMacroExpansion(macroNameTok, expansionRange);
                }
            }
        };

        // The macro expansion processor.
        MacroExpansionProcessor macroExpansionProcessor;

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
        PreprocessStateMachine(CompilerInvocationState& compiler, TokenStream& outputStream, TranslationUnitID tuId,
                               PPCallback* callback, std::optional<TextRange> includeExpansionRange,
                               size_t includeDepth)
            : compiler(compiler), sourceManager(compiler.GetSourceManager()), atomTable(compiler.GetAtomTable()),
              macroTable(compiler.GetMacroTable()), diagReporter(compiler.GetDiagnosticStream()),
              outputStream(outputStream), tuId(tuId), callback(callback), macroExpansionProcessor(*this),
              includeExpansionRange(includeExpansionRange), includeDepth(includeDepth)
        {
        }

        auto GetState() const noexcept -> PreprocessorState
        {
            return state;
        }

        auto GetNextTokenId() const noexcept -> SyntaxTokenID
        {
            return SyntaxTokenID{tuId, outputStream.GetNextTokenIndex()};
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

        auto DisableMacro(const MacroDefinition& macroDefinition) -> void
        {
            disabledMacros.insert(&macroDefinition);
        }

        auto EnableMacro(const MacroDefinition& macroDefinition) -> void
        {
            disabledMacros.erase(&macroDefinition);
        }

        auto FindEnabledMacroDefinition(AtomString name) const -> const MacroDefinition*
        {
            auto result = macroTable.FindMacroDefinition(name);
            if (result && !disabledMacros.contains(result)) {
                return result;
            }
            return nullptr;
        }

        auto OutputToken(PPToken token, TextRange expandedRange) -> void
        {
            if (token.klass == TokenKlass::Identifier) {
                token.klass = FixupKeywordTokenKlass(token.klass, token.text);
            }

#if defined(GLSLD_ENABLE_COMPILER_TRACE)
            compiler.GetCompilerTrace().TraceLexTokenIssued(token, expandedRange);
#endif
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
        FileID sourceFile;
        TokenStream outputStream;

        static auto GetTUId(FileID sourceFile) -> TranslationUnitID
        {
            if (sourceFile.IsSystemPreamble()) {
                return TranslationUnitID::SystemPreamble;
            }
            else if (sourceFile.IsUserPreamble()) {
                return TranslationUnitID::UserPreamble;
            }
            else {
                return TranslationUnitID::UserFile;
            }
        }

    public:
        Preprocessor(CompilerInvocationState& compiler, FileID sourceFile, PPCallback* callback,
                     bool versionScanningMode)
            : PreprocessStateMachine(compiler, outputStream, GetTUId(sourceFile), callback, std::nullopt, 0),
              sourceFile(sourceFile), compiler(compiler)
        {
            if (versionScanningMode) {
                InitializeForVersionScanning();
            }
            else {
                InitializeForPP();
            }
        }

        auto DoPreprocess() -> void
        {
            PreprocessSourceFile(sourceFile);

            if (!IsVersionScanningMode()) {
                auto [tokens, comments] = outputStream.Export();
                compiler.UpdateTokenArtifact(GetTUId(sourceFile), std::move(tokens), std::move(comments));
            }
        }
    };

} // namespace glsld
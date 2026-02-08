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
    struct PreprocessedTokens
    {
        std::vector<RawSyntaxToken> tokens;
        std::vector<RawCommentToken> comments;
        std::vector<PreprocessedFile> files;
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

        PreprocessedTokens& outputStream;

        // The current translation unit ID.
        TranslationUnitID tuId = TranslationUnitID::UserFile;

        // The callback interface for the preprocessor.
        PPCallback* callback = nullptr;

        // The current state of the preprocessor.
        PreprocessorState state = PreprocessorState::Default;

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
                invocationArguments.clear();
            }
            auto FinishPendingMacroInvocation() -> void
            {
                GLSLD_ASSERT(pendingInvokedMacro && argLParenCounter == 0);
                pendingInvokedMacro     = nullptr;
                pendingExpansionTokenId = {};
                witheldTokens.clear();
                invocationArguments.clear();
            }

            auto YieldToken(const PPToken& token) -> void
            {
                if (outputBuffer) {
                    outputBuffer->push_back(token);
                }
                else {
                    // FIXME: Use correct expanded range.
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

        // This buffer stores the tokens that form a PP directive. The first token is always '#'.
        std::vector<PPToken> directiveTokBuffer = {};

        // This stack stores all information about the effective conditional directives.
        std::vector<PPConditionalInfo> conditionalStack = {};

        // This tracks the number of parsed #if/#ifdef/#ifndef directives in inactive regions.
        // We need ignore their pairing #else/#elif/#endif directives.
        uint32_t skippedInactiveConditionalCount = 0;

        // Buffer for pending trivia tokens (comments). They are appended to output stream when next non-comment token
        // is preprocessed.
        std::vector<PPToken> pendingTriviaBuffer = {};

        // This tracks the (zero-based) line number trivia front attachment location.
        uint32_t triviaAttachmentLine = 0;

        AtomString atomDirectiveInclude   = {};
        AtomString atomDirectiveDefine    = {};
        AtomString atomDirectiveUndef     = {};
        AtomString atomDirectiveIf        = {};
        AtomString atomDirectiveIfdef     = {};
        AtomString atomDirectiveIfndef    = {};
        AtomString atomDirectiveElse      = {};
        AtomString atomDirectiveElif      = {};
        AtomString atomDirectiveEndif     = {};
        AtomString atomDirectiveError     = {};
        AtomString atomDirectiveExtension = {};
        AtomString atomDirectiveVersion   = {};
        AtomString atomDirectivePragma    = {};
        AtomString atomDirectiveLine      = {};

        AtomString atomBuiltinLineMacro     = {};
        AtomString atomBuiltinFileMacro     = {};
        AtomString atomBuiltinVersionMacro  = {};
        AtomString atomMacroOperatorDefined = {};

        AtomString atomExtensionBehaviorEnable  = {};
        AtomString atomExtensionBehaviorRequire = {};
        AtomString atomExtensionBehaviorWarn    = {};
        AtomString atomExtensionBehaviorDisable = {};

        AtomString atomGlslVersion110 = {};
        AtomString atomGlslVersion120 = {};
        AtomString atomGlslVersion130 = {};
        AtomString atomGlslVersion140 = {};
        AtomString atomGlslVersion150 = {};
        AtomString atomGlslVersion330 = {};
        AtomString atomGlslVersion400 = {};
        AtomString atomGlslVersion410 = {};
        AtomString atomGlslVersion420 = {};
        AtomString atomGlslVersion430 = {};
        AtomString atomGlslVersion440 = {};
        AtomString atomGlslVersion450 = {};
        AtomString atomGlslVersion460 = {};

        AtomString atomGlslProfileCore          = {};
        AtomString atomGlslProfileCompatibility = {};
        AtomString atomGlslProfileEs            = {};

    public:
        PreprocessStateMachine(CompilerInvocationState& compiler, PreprocessedTokens& outputStream,
                               TranslationUnitID tuId, PPCallback* callback,
                               std::optional<TextRange> includeExpansionRange, size_t includeDepth)
            : compiler(compiler), sourceManager(compiler.GetSourceManager()), atomTable(compiler.GetAtomTable()),
              macroTable(compiler.GetMacroTable()), diagReporter(compiler.GetDiagnosticStream()),
              outputStream(outputStream), tuId(tuId), callback(callback), macroExpansionProcessor(*this),
              includeExpansionRange(includeExpansionRange), includeDepth(includeDepth)
        {
            atomDirectiveInclude   = atomTable.GetAtom("include");
            atomDirectiveDefine    = atomTable.GetAtom("define");
            atomDirectiveUndef     = atomTable.GetAtom("undef");
            atomDirectiveIf        = atomTable.GetAtom("if");
            atomDirectiveIfdef     = atomTable.GetAtom("ifdef");
            atomDirectiveIfndef    = atomTable.GetAtom("ifndef");
            atomDirectiveElse      = atomTable.GetAtom("else");
            atomDirectiveElif      = atomTable.GetAtom("elif");
            atomDirectiveEndif     = atomTable.GetAtom("endif");
            atomDirectiveError     = atomTable.GetAtom("error");
            atomDirectiveExtension = atomTable.GetAtom("extension");
            atomDirectiveVersion   = atomTable.GetAtom("version");
            atomDirectivePragma    = atomTable.GetAtom("pragma");
            atomDirectiveLine      = atomTable.GetAtom("line");

            atomBuiltinLineMacro     = atomTable.GetAtom("__LINE__");
            atomBuiltinFileMacro     = atomTable.GetAtom("__FILE__");
            atomBuiltinVersionMacro  = atomTable.GetAtom("__VERSION__");
            atomMacroOperatorDefined = atomTable.GetAtom("defined");

            atomExtensionBehaviorEnable  = atomTable.GetAtom("enable");
            atomExtensionBehaviorRequire = atomTable.GetAtom("require");
            atomExtensionBehaviorWarn    = atomTable.GetAtom("warn");
            atomExtensionBehaviorDisable = atomTable.GetAtom("disable");

            atomGlslVersion110 = atomTable.GetAtom("110");
            atomGlslVersion120 = atomTable.GetAtom("120");
            atomGlslVersion130 = atomTable.GetAtom("130");
            atomGlslVersion140 = atomTable.GetAtom("140");
            atomGlslVersion150 = atomTable.GetAtom("150");
            atomGlslVersion330 = atomTable.GetAtom("330");
            atomGlslVersion400 = atomTable.GetAtom("400");
            atomGlslVersion410 = atomTable.GetAtom("410");
            atomGlslVersion420 = atomTable.GetAtom("420");
            atomGlslVersion430 = atomTable.GetAtom("430");
            atomGlslVersion440 = atomTable.GetAtom("440");
            atomGlslVersion450 = atomTable.GetAtom("450");
            atomGlslVersion460 = atomTable.GetAtom("460");

            atomGlslProfileCore          = atomTable.GetAtom("core");
            atomGlslProfileCompatibility = atomTable.GetAtom("compatibility");
            atomGlslProfileEs            = atomTable.GetAtom("es");
        }

        auto GetState() const noexcept -> PreprocessorState
        {
            return state;
        }

        auto GetNextTokenId() const noexcept -> SyntaxTokenID
        {
            return SyntaxTokenID{tuId, GetNextTokenIndex()};
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
            if (token.klass == TokenKlass::Comment) {
                pendingTriviaBuffer.push_back(token);
            }
            else {
                uint32_t frontAttachmentLine = triviaAttachmentLine;
                triviaAttachmentLine         = token.spelledRange.start.line;
                for (const auto& commentToken : pendingTriviaBuffer) {
                    OutputCommentToken(commentToken, frontAttachmentLine);
                }
                pendingTriviaBuffer.clear();

                DispatchTokenToHandler(token);

                if (token.klass == TokenKlass::Eof) {
                    macroExpansionProcessor.Finalize();
                    if (includeDepth == 0) {
                        // We are done with the main file. Insert an EOF token.
                        OutputToken(token, token.spelledRange);
                        TransitionToHaltState();
                    }
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

        auto GetNextTokenIndex() const noexcept -> uint32_t
        {
            return static_cast<uint32_t>(outputStream.tokens.size());
        }

        auto OutputToken(const PPToken& token, TextRange expandedRange) -> void
        {
            GLSLD_ASSERT(token.klass != TokenKlass::Comment && "Comment is handled separately");

            TokenKlass klass = token.klass;
            if (token.klass == TokenKlass::Identifier) {
                klass = FixupKeywordTokenKlass(token.klass, token.text);
            }

#if defined(GLSLD_ENABLE_COMPILER_TRACE)
            compiler.GetCompilerTrace().TraceLexTokenIssued(token, expandedRange);
#endif

            outputStream.tokens.push_back(RawSyntaxToken{
                .klass         = klass,
                .spelledFile   = token.spelledFile,
                .spelledRange  = token.spelledRange,
                .expandedRange = expandedRange,
                .text          = token.text,
            });
        }

        auto OutputCommentToken(const PPToken& token, uint32_t frontAttachmentLine) -> void
        {
            GLSLD_ASSERT(token.klass == TokenKlass::Comment);
            outputStream.comments.push_back(RawCommentToken{
                .spelledFile         = token.spelledFile,
                .spelledRange        = token.spelledRange,
                .text                = token.text,
                .frontAttachmentLine = frontAttachmentLine,
                .backAttachmentLine  = triviaAttachmentLine,
                .nextTokenIndex      = GetNextTokenIndex(),
            });
        }

        // Possible transitions:
        // - ExpectDirective -> Default (Empty directive parsed)
        // - ExpectDefaultDirectiveTail -> Default (A PP directive handled)
        // - ExpectIncludeDirectiveTail -> Default (A #include directive handled)
        auto TransitionToDefaultState(const PPToken* redirectedToken) -> void
        {
            GLSLD_ASSERT(state == PreprocessorState::ExpectDirective ||
                         state == PreprocessorState::ExpectDefaultDirectiveTail ||
                         state == PreprocessorState::ExpectIncludeDirectiveTail);
            state = PreprocessorState::Default;
            if (redirectedToken) {
                DispatchTokenToHandler(*redirectedToken);
            }
        }

        // Possible transitions:
        // - ExpectDirective -> Inactive (Current PP directive cannot exit the inactive region)
        // - ExpectDefaultDirectiveTail -> Inactive (A PP directive handled, entering an inactive region)
        auto TransitionToInactiveState(const PPToken* redirectedToken) -> void
        {
            GLSLD_ASSERT(state == PreprocessorState::ExpectDirective ||
                         state == PreprocessorState::ExpectDefaultDirectiveTail);
            state = PreprocessorState::Inactive;
            if (redirectedToken) {
                DispatchTokenToHandler(*redirectedToken);
            }
        }

        // Possible transitions:
        // - Default -> Halt (See the first token that is not comment nor part of preprocessor in version scanning mode)
        auto TransitionToHaltState() -> void
        {
            GLSLD_ASSERT(state == PreprocessorState::Default);
            state = PreprocessorState::Halt;
        }

        // Possible transitions:
        // - Default -> ExpectDirective (See # from the beginning of a line, expect a PP directive next)
        // - Inactive -> ExpectDirective (See # from the beginning of a line, expect a PP directive next)
        auto TransitionToExpectDirectiveState(const PPToken& hashToken) -> void
        {
            GLSLD_ASSERT(state == PreprocessorState::Default || state == PreprocessorState::Inactive);
            GLSLD_ASSERT(hashToken.klass == TokenKlass::Hash && hashToken.isFirstTokenOfLine);
            state = PreprocessorState::ExpectDirective;
            directiveTokBuffer.clear();
            directiveTokBuffer.push_back(hashToken);
        }

        // Possible transitions:
        // - ExpectDirective -> ExpectDefaultDirectiveTail (A PP directive parsed, expect following tokens)
        auto TransitionToExpectDefaultDirectiveTailState() -> void
        {
            GLSLD_ASSERT(state == PreprocessorState::ExpectDirective);
            state = PreprocessorState::ExpectDefaultDirectiveTail;
        }

        // Possible transitions:
        // - ExpectDirective -> ExpectIncludeDirectiveTail (A #include directive parsed, expect following tokens)
        auto TransitionToExpectIncludeDirectiveTailState() -> void
        {
            GLSLD_ASSERT(state == PreprocessorState::ExpectDirective);
            state = PreprocessorState::ExpectIncludeDirectiveTail;
        }

        // Dispatch the token to the appropriate handler based on the current state.
        // The handler (AcceptOnXXXState) will then update the state and act accordingly.
        auto DispatchTokenToHandler(const PPToken& token) -> void;

        auto AcceptOnDefaultState(const PPToken& token) -> void;
        auto AcceptOnInactiveState(const PPToken& token) -> void;
        auto AcceptOnExpectDirectiveState(const PPToken& token) -> void;
        auto AcceptOnExpectDirectiveTailState(const PPToken& token) -> void;

        auto ParseExtensionBehavior(const PPToken& toggle) -> std::optional<ExtensionBehavior>;
        auto ParseGlslVersion(const PPToken& versionNumber) -> std::optional<GlslVersion>;
        auto ParseGlslProfile(const PPToken& profile) -> std::optional<GlslProfile>;

        auto DispatchPPDirectiveToHandler() -> void;
        auto HandleBadDirective(PPTokenScanner& scanner) -> void;
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
        auto HandlePragmaDirective(PPTokenScanner& scanner) -> void;
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
        PreprocessedTokens outputStream;

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
                compiler.UpdatePreprocessingArtifact(GetTUId(sourceFile), std::move(outputStream.tokens),
                                                     std::move(outputStream.comments), std::move(outputStream.files));
            }
        }
    };

} // namespace glsld
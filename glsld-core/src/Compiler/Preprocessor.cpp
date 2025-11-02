#include "Compiler/Preprocessor.h"
#include "Compiler/Tokenizer.h"
#include "Compiler/SyntaxToken.h"
#include "Language/ShaderTarget.h"
#include "Language/ConstValue.h"
#include "Support/ScopeExit.h"

#include <string>

namespace glsld
{
#pragma region TokenStream
    auto TokenStream::AddToken(const PPToken& token, TextRange expandedRange) -> void
    {
        GLSLD_ASSERT(token.klass != TokenKlass::Eof && "EOF is handled separately");
        // FIXME: why this fails?
        // if (!tokens.empty()) {
        //     GLSLD_ASSERT(token.spelledFile != tokens.back().spelledFile ||
        //                  token.spelledRange.start >= tokens.back().spelledRange.start);
        //     GLSLD_ASSERT(expandedRange.start >= tokens.back().expandedRange.start);
        // }

        if (token.klass != TokenKlass::Comment) {
            tokens.push_back(RawSyntaxToken{
                .klass         = token.klass,
                .spelledFile   = token.spelledFile,
                .spelledRange  = token.spelledRange,
                .expandedRange = expandedRange,
                .text          = token.text,
            });
        }
        else {
            // FIXME: make a toggle for this
            comments.push_back(RawCommentToken{
                .spelledFile    = token.spelledFile,
                .spelledRange   = token.spelledRange,
                .text           = token.text,
                .nextTokenIndex = static_cast<uint32_t>(tokens.size()),
            });
        }
    }

    auto TokenStream::AddEofToken(const PPToken& token, TextRange expandedRange) -> void
    {
        GLSLD_ASSERT(token.klass == TokenKlass::Eof);
        tokens.push_back(RawSyntaxToken{
            .klass         = token.klass,
            .spelledFile   = token.spelledFile,
            .spelledRange  = token.spelledRange,
            .expandedRange = expandedRange,
            .text          = token.text,
        });
    }
#pragma endregion

#pragma region MacroExpansionProcessor
    auto PreprocessStateMachine::MacroExpansionProcessor::Feed(const PPToken& token) -> void
    {
        GLSLD_ASSERT(token.klass != TokenKlass::Eof);

        if (witheldTokens.empty()) {
            FeedTokenWithoutMacroContext(token);
        }
        else {
            FeedTokenWithMacroContext(token);
        }
    }
    auto PreprocessStateMachine::MacroExpansionProcessor::Finalize() -> void
    {
        if (pendingInvokedMacro) {
            if (argLParenCounter > 0) {
                // FIXME: unterminated argument list
            }

            RevokePendingMacroInvocation();
        }
    }
    auto PreprocessStateMachine::MacroExpansionProcessor::FeedTokenWithMacroContext(const PPToken& token) -> void
    {
        GLSLD_ASSERT(!witheldTokens.empty());

        if (invocationArguments.empty()) {
            // We have witheld a single token, which is the macro name.
            // We are looking for a '(' to start the argument list of a function-like macro.
            GLSLD_ASSERT(argLParenCounter == 0);

            if (token.klass == TokenKlass::LParen) {
                // We are seeing `MACRO (`. This is likely a function-like macro invocation.
                NewPendingInvocationArgument();
                argLParenCounter += 1;
            }
            else {
                // This is not a function-like macro invocation.
                RevokePendingMacroInvocation();
                FeedTokenWithoutMacroContext(token);
            }
        }
        else {
            // We are collecting the argument list of a function-like macro.
            // So try to close the list with ')' or continue collecting arguments.
            GLSLD_ASSERT(argLParenCounter >= 1);

            if (token.klass == TokenKlass::LParen) {
                // NOTE if we see '(' inside the argument list, we have to increment the paranthesis
                // counter for balancing.
                argLParenCounter += 1;
            }
            else if (token.klass == TokenKlass::RParen) {
                argLParenCounter -= 1;
            }

            if (argLParenCounter == 0) {
                // The argument list has been fully collected. Now we can finalize the macro invocation.
                GLSLD_ASSERT(token.klass == TokenKlass::RParen);
                FinishPendingInvocationArgument();
                if (pendingInvokedMacro->GetParamTokens().empty() && invocationArguments.size() == 1 &&
                    invocationArguments.front().numArgumentToken == 0) {
                    // If the macro has no parameter and no token in the argument list, we should fix-up the argument
                    // list so that it's also empty.
                    invocationArguments.clear();
                }
                if (invocationArguments.size() != pendingInvokedMacro->GetParamTokens().size()) {
                    // FIXME: Not exact number of arguments provided. Report error.
                }

                FeedMacroExpansion(witheldTokens.front(), *pendingInvokedMacro, witheldTokens, invocationArguments,
                                   pendingExpansionTokenId);
                FinishPendingMacroInvocation();
            }
            else if (argLParenCounter == 1 && token.klass == TokenKlass::Comma) {
                // This is the delimiting comma of the argument list.
                FinishPendingInvocationArgument();
                NewPendingInvocationArgument();
                witheldTokens.push_back(token);
            }
            else {
                // This is a regular token in the argument list.
                auto& currentArg = invocationArguments.back();
                if (currentArg.numArgumentToken == 0) {
                    currentArg.pasteToken = token;
                }
                currentArg.numArgumentToken += 1;

                argProcessor->Feed(token);
            }
        }
    }
    auto PreprocessStateMachine::MacroExpansionProcessor::FeedTokenWithoutMacroContext(const PPToken& token) -> void
    {
        // No context, we are either looking for a macro invocation or just yield the token.
        GLSLD_ASSERT(witheldTokens.empty() && argLParenCounter == 0);

        if (token.klass == TokenKlass::Identifier) {
            if (token.text == pp.atomBuiltinLineMacro) {
                // Handle __LINE__ builtin macro
                const auto nextTokenId = pp.GetNextTokenId();
                YieldToken(PPToken{
                    .klass                = TokenKlass::IntegerConstant,
                    .spelledFile          = token.spelledFile,
                    .spelledRange         = TextRange{token.spelledRange.start},
                    .text                 = pp.atomTable.GetAtom(std::to_string(token.spelledRange.start.line + 1)),
                    .isFirstTokenOfLine   = false,
                    .hasLeadingWhitespace = false,
                });

                if (pp.callback) {
                    pp.callback->OnMacroExpansion(token, AstSyntaxRange{nextTokenId});
                }
            }
            else if (token.text == pp.atomBuiltinFileMacro) {
                // Handle __FILE__ builtin macro
                // FIXME: Need clarification of what value to use here.
                const auto nextTokenId = pp.GetNextTokenId();
                YieldToken(PPToken{
                    .klass                = TokenKlass::IntegerConstant,
                    .spelledFile          = token.spelledFile,
                    .spelledRange         = TextRange{token.spelledRange.start},
                    .text                 = pp.atomTable.GetAtom(std::to_string(token.spelledFile.GetValue())),
                    .isFirstTokenOfLine   = false,
                    .hasLeadingWhitespace = false,
                });

                if (pp.callback) {
                    pp.callback->OnMacroExpansion(token, AstSyntaxRange{nextTokenId});
                }
            }
            else if (token.text == pp.atomBuiltinVersionMacro) {
                // Handle __VERSION__ builtin macro
                const auto nextTokenId = pp.GetNextTokenId();
                YieldToken(PPToken{
                    .klass        = TokenKlass::IntegerConstant,
                    .spelledFile  = token.spelledFile,
                    .spelledRange = TextRange{token.spelledRange.start},
                    .text         = pp.atomTable.GetAtom(
                        std::to_string(static_cast<uint32_t>(pp.compiler.GetLanguageConfig().version))),
                    .isFirstTokenOfLine   = false,
                    .hasLeadingWhitespace = false,
                });

                if (pp.callback) {
                    pp.callback->OnMacroExpansion(token, AstSyntaxRange{nextTokenId});
                }
            }
            else if (auto macroDefinition = pp.FindEnabledMacroDefinition(token.text); macroDefinition) {
                if (macroDefinition->IsFunctionLike()) {
                    // Could be a function-like macro invocation. We withold the token for potential expansion.
                    witheldTokens.push_back(token);
                    pendingInvokedMacro     = macroDefinition;
                    pendingExpansionTokenId = pp.GetNextTokenId();
                }
                else {
                    // An object-like macro invocation.
                    FeedMacroExpansion(token, *macroDefinition, {}, {}, pp.GetNextTokenId());
                }
            }
            else {
                YieldToken(token);
            }
        }
        else {
            YieldToken(token);
        }
    }
    auto PreprocessStateMachine::MacroExpansionProcessor::FeedMacroExpansion(const PPToken& macroNameTok,
                                                                             const MacroDefinition& macroDefinition,
                                                                             ArrayView<PPToken> invocationTokens,
                                                                             ArrayView<InvocationArgumentInfo> args,
                                                                             SyntaxTokenID expansionStartId) -> void
    {
        // Disable this macro to avoid recursive expansion during rescan.
        EnterMacroExpansion(macroNameTok, macroDefinition);
        auto _ = ScopeExit{[this, &macroNameTok, &macroDefinition, &expansionStartId] {
            ExitMacroExpansion(macroNameTok, macroDefinition, AstSyntaxRange{expansionStartId, pp.GetNextTokenId()});
        }};

        ArrayView<PPToken> paramTokens = macroDefinition.GetParamTokens();

        PPTokenScanner macroScanner{macroDefinition.GetExpansionTokens()};
        MacroExpansionProcessor nextProcessor{pp, outputBuffer};
        while (!macroScanner.CursorAtEnd()) {
            // NOTE we assume that all tokens are expanded into the beginning of the macro use token.
            PPToken token              = macroScanner.ConsumeToken();
            token.spelledFile          = macroNameTok.spelledFile;
            token.spelledRange         = TextRange{macroNameTok.spelledRange.start};
            token.isFirstTokenOfLine   = false;
            token.hasLeadingWhitespace = false;

            // FIXME: properly handle comments
            if (token.klass == TokenKlass::Hash) {
                if (macroScanner.TryTestToken(TokenKlass::Identifier, 1)) {
                    // #identifier, aka. stringification
                    // However, as GLSL doesn't have string literal, we just discard two tokens.
                    // FIXME: report error, stringification is not supported.
                    macroScanner.ConsumeToken();
                    continue;
                }

                // Fallthrough to be handled as regular token.
            }
            else if (macroScanner.TryTestToken(TokenKlass::HashHash)) {
                // token##token, aka. token pasting
                bool pastingFailure = false;
                std::string pastedText;
                auto pasteTokenText = [&](const PPToken& tok) {
                    if (tok.klass == TokenKlass::Identifier) {
                        // Try substitute the parameter names with the unexpanded argument.
                        for (size_t i = 0; i < paramTokens.size(); ++i) {
                            if (tok.text == paramTokens[i].text) {
                                if (args[i].numArgumentToken == 1) {
                                    pastedText += args[i].pasteToken.text.StrView();
                                }
                                else if (args[i].numArgumentToken > 1) {
                                    pastingFailure = true;
                                }
                                return;
                            }
                        }
                    }

                    pastedText += tok.text.StrView();
                };

                // Paste all token text into a single string buffer.
                pasteTokenText(token);
                while (macroScanner.TryConsumeToken(TokenKlass::HashHash)) {
                    if (!macroScanner.CursorAtEnd()) {
                        pasteTokenText(macroScanner.ConsumeToken());
                    }
                    else {
                        pastingFailure = true;
                    }
                }

                // Try to tokenize the pasted text.
                auto [klass, tokText, remText] = TokenizeOnce(pastedText);
                if (!pastingFailure && remText.Empty()) {
                    token.klass = klass;
                    token.text  = pp.atomTable.GetAtom(tokText);
                    nextProcessor.Feed(token);
                }
                else {
                    // FIXME: report error, bad token pasting
                }

                continue;
            }
            else if (token.klass == TokenKlass::Identifier) {
                // Try substitute the parameter names with the expanded argument.
                bool substituted = false;
                for (size_t i = 0; i < paramTokens.size(); ++i) {
                    if (token.text == paramTokens[i].text) {
                        if (args.size() > i) {
                            auto expandedArgs = ArrayView<PPToken>{invocationTokens.begin() + args[i].indexBegin,
                                                                   invocationTokens.begin() + args[i].indexEnd};
                            for (const PPToken& argToken : expandedArgs) {
                                nextProcessor.Feed(argToken);
                            }
                        }

                        substituted = true;
                        break;
                    }
                }

                if (substituted) {
                    continue;
                }

                // Fallthrough to be handled as regular token.
            }

            nextProcessor.Feed(token);
        }

        nextProcessor.Finalize();
    }
#pragma endregion

#pragma region PreprocessStateMachine
    auto PreprocessStateMachine::PreprocessSourceFile(FileID sourceFile) -> void
    {
        if (!sourceFile.IsValid()) {
            // FIXME: report error, invalid source file
            return;
        }

        auto sourceText = sourceManager.GetSourceText(sourceFile);
        Tokenizer{compiler, *this, atomTable, sourceFile, sourceText}.DoTokenize();
    }

    auto PreprocessStateMachine::InitializeKeywordLookup() -> void
    {
#define DECL_KEYWORD(KEYWORD) keywordLookup[atomTable.GetAtom(#KEYWORD)] = TokenKlass::K_##KEYWORD;
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
    }

    auto PreprocessStateMachine::FixupKeywordTokenKlass(TokenKlass klass, AtomString text) -> TokenKlass
    {
        if (klass == TokenKlass::Identifier) {
            if (auto it = keywordLookup.find(text); it != keywordLookup.end()) {
                return it->second;
            }
        }

        return klass;
    }

    auto PreprocessStateMachine::DispatchTokenToHandler(const PPToken& token) -> void
    {
        switch (state) {
        case PreprocessorState::Default:
            AcceptOnDefaultState(token);
            break;

        case glsld::PreprocessorState::Halt:
            GLSLD_ASSERT(false);
            break;

        case PreprocessorState::Inactive:
            AcceptOnInactiveState(token);
            break;

        case PreprocessorState::ExpectDirective:
            AcceptOnExpectDirectiveState(token);
            break;

        case PreprocessorState::ExpectDefaultDirectiveTail:
        case PreprocessorState::ExpectIncludeDirectiveTail:
            AcceptOnExpectDirectiveTailState(token);
            break;
        }
    }

    auto PreprocessStateMachine::AcceptOnDefaultState(const PPToken& token) -> void
    {
        if (token.klass == TokenKlass::Eof) {
            // EOF is handled by caller
        }
        else if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
            TransitionTo(PreprocessorState::ExpectDirective);
        }
        else {
            // TODO: skip tokenization for version scanning mode
            macroExpansionProcessor.Feed(token);
        }
    }

    auto PreprocessStateMachine::AcceptOnInactiveState(const PPToken& token) -> void
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

    auto PreprocessStateMachine::AcceptOnExpectDirectiveState(const PPToken& token) -> void
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
                if (directiveToken->text == atomDirectiveInclude) {
                    TransitionTo(PreprocessorState::ExpectIncludeDirectiveTail);
                }
                else {
                    TransitionTo(PreprocessorState::ExpectDefaultDirectiveTail);
                }
            }
            else {
                // We are in an inactive region.
                if (directiveToken->text == atomDirectiveIf || directiveToken->text == atomDirectiveIfdef ||
                    directiveToken->text == atomDirectiveIfndef || directiveToken->text == atomDirectiveElif ||
                    directiveToken->text == atomDirectiveElse || directiveToken->text == atomDirectiveEndif) {
                    // These directives may change the state of the conditional stack.
                    TransitionTo(PreprocessorState::ExpectDefaultDirectiveTail);
                }
                else {
                    // Other directives are skipped in inactive regions.
                    TransitionTo(PreprocessorState::Inactive);
                    directiveToken = std::nullopt;
                }
            }
        }
        else if (token.klass == TokenKlass::Comment) {
            // FIXME: we ignore comment for now.
            return;
        }
        else {
            // A bad directive.
            TransitionTo(PreprocessorState::ExpectDefaultDirectiveTail);
        }
    }

    auto PreprocessStateMachine::AcceptOnExpectDirectiveTailState(const PPToken& token) -> void
    {
        if (token.klass == TokenKlass::Eof || token.isFirstTokenOfLine) {
            // Finish processing the directive.
            if (directiveToken) {
                // FIXME: what if the directive is bad? we should raise error
                HandleDirective(*directiveToken, directiveArgBuffer);
            }
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
            if (token.klass != TokenKlass::Comment) {
                // FIXME: we ignore comment for now.
                directiveArgBuffer.push_back(token);
            }
        }
    }

    auto PreprocessStateMachine::HandleDirective(const PPToken& directiveToken, ArrayView<PPToken> restTokens) -> void
    {
        PPTokenScanner scanner{restTokens};
        if (directiveToken.text == atomDirectiveInclude) {
            HandleIncludeDirective(scanner);
        }
        else if (directiveToken.text == atomDirectiveDefine) {
            HandleDefineDirective(scanner);
        }
        else if (directiveToken.text == atomDirectiveUndef) {
            HandleUndefDirective(scanner);
        }
        else if (directiveToken.text == atomDirectiveIf) {
            HandleIfDirective(scanner);
        }
        else if (directiveToken.text == atomDirectiveIfdef) {
            HandleIfdefDirective(scanner, false);
        }
        else if (directiveToken.text == atomDirectiveIfndef) {
            HandleIfdefDirective(scanner, true);
        }
        else if (directiveToken.text == atomDirectiveElse) {
            HandleElseDirective(scanner);
        }
        else if (directiveToken.text == atomDirectiveElif) {
            HandleElifDirective(scanner);
        }
        else if (directiveToken.text == atomDirectiveEndif) {
            HandleEndifDirective(scanner);
        }
        else if (directiveToken.text == atomDirectiveError) {
            // FIXME: report error
        }
        else if (directiveToken.text == atomDirectiveExtension) {
            HandleExtensionDirective(scanner);
        }
        else if (directiveToken.text == atomDirectiveVersion) {
            HandleVersionDirective(scanner);
        }
        else if (directiveToken.text == atomDirectivePragma) {
            HandlePragmaDirective(scanner);
        }
        else if (directiveToken.text == atomDirectiveLine) {
            HandleLineDirective(scanner);
        }
        else {
            // FIXME: warn about unknown directives
        }

        if (versionScanningMode) {
            // FIXME: should we also halt PP in version scanning mode for directives that are not version/extension?
        }
    }

    auto PreprocessStateMachine::HandleIncludeDirective(PPTokenScanner& scanner) -> void
    {
        const auto& compilerConfig = compiler.GetCompilerConfig();
        if (includeDepth >= compilerConfig.maxIncludeDepth) {
            // FIXME: report error, too many nested include files
            return;
        }

        if (auto headerNameToken = scanner.TryConsumeToken(TokenKlass::UserHeaderName, TokenKlass::SystemHeaderName)) {
            if (!scanner.CursorAtEnd()) {
                // FIXME: report warning, extra tokens after the header file name
            }

            // Search for the header file in the include paths and load the source text.
            StringView headerName = headerNameToken->text.StrView().Drop(1).DropBack(1);
            FileID includeFile;
            for (const auto& includePath : compilerConfig.includePaths) {
                // TODO: distinguish between system include and user include
                includeFile = sourceManager.OpenFromFile(includePath / headerName.StdStrView());
                if (includeFile.IsValid()) {
                    break;
                }
            }

            // Run PP callback event if any
            if (callback) {
                callback->OnIncludeDirective(*headerNameToken, sourceManager.GetAbsolutePath(includeFile));
            }

            if (!includeFile.IsValid()) {
                // FIXME: report error, cannot find the header file
                return;
            }

            // We create a new preprocessor to process the included file.
#if defined(GLSLD_DEBUG)
            compiler.GetCompilerTrace().TraceEnterIncludeFile(headerName);
#endif
            if (callback) {
                callback->OnEnterIncludedFile();
            }

            PreprocessStateMachine nextPP{
                compiler,
                outputStream,
                tuId,
                callback,
                includeExpansionRange ? includeExpansionRange : TextRange{headerNameToken->spelledRange.start},
                includeDepth + 1,
            };
            nextPP.PreprocessSourceFile(includeFile);

            if (callback) {
                callback->OnExitIncludedFile();
            }
#if defined(GLSLD_DEBUG)
            compiler.GetCompilerTrace().TraceExitIncludeFile(headerName);
#endif
        }
        else {
            // FIXME: report error, expected a header file name
        }
    }

    auto PreprocessStateMachine::HandleDefineDirective(PPTokenScanner& scanner) -> void
    {
        // Parse the macro name
        PPToken macroName;
        if (auto tok = scanner.TryConsumeToken(TokenKlass::Identifier); tok) {
            macroName = *tok;
        }
        else {
            // FIXME: report error, expected a macro name
            return;
        }

        if (scanner.CursorAtEnd()) {
            // Fast path for empty macro definitions.
            macroTable.DefineObjectLikeMacro(macroName, {});

            // Run PP callback event if any
            if (callback) {
                callback->OnDefineDirective(macroName, {}, {}, false);
            }
            return;
        }

        // Parse the macro parameters, if any
        bool isFunctionLike = false;
        std::vector<PPToken> paramTokens;
        if (scanner.PeekToken().klass == TokenKlass::LParen && !scanner.PeekToken().hasLeadingWhitespace) {
            isFunctionLike = true;
            scanner.ConsumeToken();

            if (scanner.TryConsumeToken(TokenKlass::RParen)) {
                // Fast path for empty parameter list
            }
            else {
                bool expectComma = false;
                while (!scanner.CursorAtEnd()) {
                    if (expectComma) {
                        if (scanner.TryConsumeToken(TokenKlass::Comma)) {
                            expectComma = false;
                        }
                        else if (scanner.TryConsumeToken(TokenKlass::RParen)) {
                            break;
                        }
                        else {
                            // FIXME: report error, expected a comma or a terminating ')'
                            return;
                        }
                    }
                    else {
                        if (auto tok = scanner.TryConsumeToken(TokenKlass::Identifier); tok) {
                            paramTokens.push_back(*tok);
                            expectComma = true;
                        }
                        else if (scanner.TryTestToken(TokenKlass::Dot)) {
                            auto tok1 = scanner.TryConsumeToken(TokenKlass::Dot);
                            auto tok2 = scanner.TryConsumeToken(TokenKlass::Dot);
                            auto tok3 = scanner.TryConsumeToken(TokenKlass::Dot);
                            if (tok1 && tok2 && tok3 && !tok2->hasLeadingWhitespace && !tok3->hasLeadingWhitespace) {
                                // FIXME: report error, variadic macro is not supported yet
                            }
                            else {
                                // FIXME: report error, expected a macro parameter name
                            }

                            return;
                        }
                        else {
                            // FIXME: report error, expected a macro parameter name
                            return;
                        }
                    }
                }
            }
        }

        // Parse the macro body
        std::vector<PPToken> expansionTokens;
        while (!scanner.CursorAtEnd()) {
            expansionTokens.push_back(scanner.ConsumeToken());
        }

        // Run PP callback event if any
        if (callback) {
            callback->OnDefineDirective(macroName, paramTokens, expansionTokens, isFunctionLike);
        }

        // Register the macro
        if (isFunctionLike) {
            macroTable.DefineFunctionLikeMacro(macroName, std::move(paramTokens), std::move(expansionTokens));
        }
        else {
            macroTable.DefineObjectLikeMacro(macroName, std::move(expansionTokens));
        }
    }

    auto PreprocessStateMachine::HandleUndefDirective(PPTokenScanner& scanner) -> void
    {
        // Parse the macro name
        PPToken macroName;
        if (auto tok = scanner.TryConsumeToken(TokenKlass::Identifier); tok) {
            macroName = *tok;
        }
        else {
            // FIXME: report error, expected a macro name
            return;
        }

        if (!scanner.CursorAtEnd()) {
            // FIXME: report warning, expected no more tokens after the macro name.
        }

        // Run PP callback event if any
        if (callback) {
            callback->OnUndefDirective(macroName);
        }

        // Undefine the macro
        // FIXME: report error if the macro is not defined. Where do we want this check to be placed?
        // FIXME: report error to undefine a builtin macro
        macroTable.UndefineMacro(macroName.text);
    }

    auto PreprocessStateMachine::HandleIfDirective(PPTokenScanner& scanner) -> void
    {
        if (!conditionalStack.empty() && !conditionalStack.back().active) {
            skippedInactiveConditionalCount += 1;
            return;
        }

        bool evalToTrue = EvaluatePPExpression(scanner);

        // Run PP callback event if any
        if (callback) {
            callback->OnIfDirective(evalToTrue);
        }

        conditionalStack.push_back(PPConditionalInfo{
            .active           = evalToTrue,
            .seenActiveBranch = evalToTrue,
            .seenElse         = false,
        });

#if defined(GLSLD_DEBUG)
        compiler.GetCompilerTrace().TracePPConditionalInfo(evalToTrue);
#endif
    }

    auto PreprocessStateMachine::HandleIfdefDirective(PPTokenScanner& scanner, bool isNDef) -> void
    {
        if (!conditionalStack.empty() && !conditionalStack.back().active) {
            skippedInactiveConditionalCount += 1;
            return;
        }

        // Parse the macro name
        PPToken macroName;
        if (auto tok = scanner.TryConsumeToken(TokenKlass::Identifier); tok) {
            macroName = *tok;
        }
        else {
            // FIXME: report error, expected a macro name
            return;
        }

        if (!scanner.CursorAtEnd()) {
            // FIXME: report warning, expected no more tokens after the macro name.
        }

        // Run PP callback event if any
        if (callback) {
            callback->OnIfDefDirective(macroName, isNDef);
        }

        bool active = macroTable.IsMacroDefined(macroName.text) != isNDef;
        conditionalStack.push_back(PPConditionalInfo{
            .active           = active,
            .seenActiveBranch = active,
            .seenElse         = false,
        });

#if defined(GLSLD_DEBUG)
        compiler.GetCompilerTrace().TracePPConditionalInfo(active);
#endif
    }

    auto PreprocessStateMachine::HandleElifDirective(PPTokenScanner& scanner) -> void
    {
        if (skippedInactiveConditionalCount > 0) {
            return;
        }

        bool evalToTrue = EvaluatePPExpression(scanner);

        if (conditionalStack.empty()) {
            // FIXME: report warning, standalone #elif directive
            return;
        }

        auto& conditionalInfo = conditionalStack.back();
        if (conditionalInfo.seenElse) {
            // FIXME: report error, #elif directive after #else directive
            return;
        }

        // Run PP callback event if any
        if (callback) {
            callback->OnElifDirective(evalToTrue);
        }

        conditionalInfo.active           = !conditionalInfo.seenActiveBranch && evalToTrue;
        conditionalInfo.seenActiveBranch = conditionalInfo.active;

#if defined(GLSLD_DEBUG)
        compiler.GetCompilerTrace().TracePPConditionalInfo(conditionalInfo.active);
#endif
    }

    auto PreprocessStateMachine::HandleElseDirective(PPTokenScanner& scanner) -> void
    {
        if (skippedInactiveConditionalCount > 0) {
            return;
        }

        if (!scanner.CursorAtEnd()) {
            // FIXME: report warning, expected no more tokens after the directive.
        }

        if (conditionalStack.empty()) {
            // FIXME: report warning, standalone #else directive
            return;
        }

        auto& conditionalInfo = conditionalStack.back();
        if (conditionalInfo.seenElse) {
            // FIXME: report error, an extraous #else directive
            return;
        }

        // Run PP callback event if any
        if (callback) {
            callback->OnElseDirective();
        }

        conditionalInfo.active           = !conditionalInfo.seenActiveBranch;
        conditionalInfo.seenActiveBranch = true;
        conditionalInfo.seenElse         = true;

#if defined(GLSLD_DEBUG)
        compiler.GetCompilerTrace().TracePPConditionalInfo(conditionalInfo.active);
#endif
    }

    auto PreprocessStateMachine::HandleEndifDirective(PPTokenScanner& scanner) -> void
    {
        if (skippedInactiveConditionalCount > 0) {
            skippedInactiveConditionalCount -= 1;
            return;
        }

        if (!scanner.CursorAtEnd()) {
            // FIXME: report warning, expected no more tokens after the directive.
        }

        if (conditionalStack.empty()) {
            // FIXME: report warning, unmatched #endif directive
            return;
        }

        // Run PP callback event if any
        if (callback) {
            callback->OnEndifDirective();
        }

        conditionalStack.pop_back();
    }

    auto PreprocessStateMachine::ParseExtensionBehavior(const PPToken& toggle) -> std::optional<ExtensionBehavior>
    {
        GLSLD_ASSERT(toggle.klass == TokenKlass::Identifier);
        if (toggle.text == atomExtensionBehaviorEnable) {
            return ExtensionBehavior::Enable;
        }
        else if (toggle.text == atomExtensionBehaviorRequire) {
            return ExtensionBehavior::Require;
        }
        else if (toggle.text == atomExtensionBehaviorWarn) {
            return ExtensionBehavior::Warn;
        }
        else if (toggle.text == atomExtensionBehaviorDisable) {
            return ExtensionBehavior::Disable;
        }
        else {
            return std::nullopt;
        }
    }

    auto PreprocessStateMachine::HandleExtensionDirective(PPTokenScanner& scanner) -> void
    {
        if (!scanner.TryTestToken(TokenKlass::Identifier)) {
            // FIXME: report error, expected an extension name
            return;
        }
        auto extensionNameTok = scanner.ConsumeToken();
        auto extension        = ParseExtensionName(extensionNameTok.text.StrView());
        if (!extension) {
            // FIXME: report error, invalid extension name
            return;
        }

        if (!scanner.TryConsumeToken(TokenKlass::Colon)) {
            // FIXME: report error, expected a colon
            return;
        }
        if (!scanner.TryTestToken(TokenKlass::Identifier)) {
            // FIXME: report error, expected an extension toggle
            return;
        }
        auto extensionBehaviorTok = scanner.ConsumeToken();
        auto behavior             = ParseExtensionBehavior(extensionBehaviorTok);
        if (!behavior) {
            // FIXME: report error, invalid extension toggle
            return;
        }

        // TODO: validate the extension name and toggle
        if (callback) {
            callback->OnExtensionDirective(extensionNameTok.spelledFile, extensionNameTok.spelledRange, *extension,
                                           *behavior);
        }
    }

    auto PreprocessStateMachine::ParseGlslVersion(const PPToken& versionNumber) -> std::optional<GlslVersion>
    {
        if (versionNumber.klass != TokenKlass::IntegerConstant) {
            return std::nullopt;
        }

        if (versionNumber.text == atomGlslVersion110) {
            return GlslVersion::Ver110;
        }
        else if (versionNumber.text == atomGlslVersion120) {
            return GlslVersion::Ver120;
        }
        else if (versionNumber.text == atomGlslVersion130) {
            return GlslVersion::Ver130;
        }
        else if (versionNumber.text == atomGlslVersion140) {
            return GlslVersion::Ver140;
        }
        else if (versionNumber.text == atomGlslVersion150) {
            return GlslVersion::Ver150;
        }
        else if (versionNumber.text == atomGlslVersion330) {
            return GlslVersion::Ver330;
        }
        else if (versionNumber.text == atomGlslVersion400) {
            return GlslVersion::Ver400;
        }
        else if (versionNumber.text == atomGlslVersion410) {
            return GlslVersion::Ver410;
        }
        else if (versionNumber.text == atomGlslVersion420) {
            return GlslVersion::Ver420;
        }
        else if (versionNumber.text == atomGlslVersion430) {
            return GlslVersion::Ver430;
        }
        else if (versionNumber.text == atomGlslVersion440) {
            return GlslVersion::Ver440;
        }
        else if (versionNumber.text == atomGlslVersion450) {
            return GlslVersion::Ver450;
        }
        else if (versionNumber.text == atomGlslVersion460) {
            return GlslVersion::Ver460;
        }
        else {
            return std::nullopt;
        }
    }

    auto PreprocessStateMachine::ParseGlslProfile(const PPToken& profile) -> std::optional<GlslProfile>
    {
        GLSLD_ASSERT(profile.klass == TokenKlass::Identifier);
        if (profile.text == atomGlslProfileCore) {
            return GlslProfile::Core;
        }
        else if (profile.text == atomGlslProfileCompatibility) {
            return GlslProfile::Compatibility;
        }
        else if (profile.text == atomGlslProfileEs) {
            return GlslProfile::Es;
        }
        else {
            return std::nullopt;
        }
    }

    auto GetDefaultProfile(GlslVersion version) -> GlslProfile
    {
        switch (version) {
        case GlslVersion::Ver110:
        case GlslVersion::Ver120:
        case GlslVersion::Ver130:
        case GlslVersion::Ver140:
        case GlslVersion::Ver150:
            return GlslProfile::Compatibility;

        case GlslVersion::Ver300:
        case GlslVersion::Ver310:
            return GlslProfile::Compatibility;

        case GlslVersion::Ver330:
        case GlslVersion::Ver400:
        case GlslVersion::Ver410:
        case GlslVersion::Ver420:
        case GlslVersion::Ver430:
        case GlslVersion::Ver440:
        case GlslVersion::Ver450:
        case GlslVersion::Ver460:
            return GlslProfile::Core;
        }

        GLSLD_UNREACHABLE();
    }

    auto PreprocessStateMachine::HandleVersionDirective(PPTokenScanner& scanner) -> void
    {
        if (scanner.CursorAtEnd()) {
            // FIXME: report error, expect version number
            return;
        }
        PPToken versionTok                 = scanner.ConsumeToken();
        std::optional<GlslVersion> version = ParseGlslVersion(versionTok);
        if (!version) {
            // FIXME: report error
            return;
        }

        PPToken profileTok;
        std::optional<GlslProfile> profile;
        if (!scanner.CursorAtEnd()) {
            profileTok = scanner.ConsumeToken();
            profile    = ParseGlslProfile(profileTok);
            if (!profile) {
                // FIXME: report error
                return;
            }
        }

        if (profile) {
            // FIXME: check version and profile are correct
        }
        else {
            profile = GetDefaultProfile(*version);
        }

        GLSLD_ASSERT(version && profile);
        // TODO: validate the version number and profile
        if (callback) {
            callback->OnVersionDirective(versionTok.spelledFile, versionTok.spelledRange, *version, *profile);
        }
    }

    auto PreprocessStateMachine::HandlePragmaDirective(PPTokenScanner& scanner) -> void
    {
        if (scanner.CursorAtEnd()) {
            // FIXME: report warning, expected pragma arguments
            return;
        }

        PPToken pragmaTok = scanner.ConsumeToken();
        // FIXME: handle known pragmas

        if (callback) {
            std::vector<PPToken> pragmaArgs;
            pragmaArgs.push_back(pragmaTok);
            while (!scanner.CursorAtEnd()) {
                pragmaArgs.push_back(scanner.ConsumeToken());
            }

            callback->OnUnknownPragmaDirective(pragmaArgs);
        }
    }

    auto PreprocessStateMachine::HandleLineDirective(PPTokenScanner& scanner) -> void
    {
        // FIXME: support line directive
    }

    // See https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.pdf, section 3.3
    enum class PPOperator
    {
        // Precedence 1 (highest)
        LParen,

        // Precedence 2
        Identity,
        Negate,
        BitNot,
        LogicalNot,

        // Precedence 3
        Multiply,
        Divide,
        Modulo,

        // Precedence 4
        Add,
        Subtract,

        // Precedence 5
        LShift,
        RShift,

        // Precedence 6
        Less,
        LessEq,
        Greater,
        GreaterEq,

        // Precedence 7
        Equal,
        NotEqual,

        // Precedence 8
        BitAnd,

        // Precedence 9
        BitXor,

        // Precedence 10
        BitOr,

        // Precedence 11
        LogicalAnd,

        // Precedence 12
        LogicalOr,
    };

    struct PPOperatorInfo
    {
        PPOperator op;
        int precedence;
        bool isUnary;
    };

    static auto GetParenPPOperator() -> PPOperatorInfo
    {
        return PPOperatorInfo{.op = PPOperator::LParen, .precedence = 1, .isUnary = false};
    }

    static auto ParseUnaryPPOperator(const PPToken& token) -> std::optional<PPOperatorInfo>
    {
        switch (token.klass) {
        case TokenKlass::Plus:
            return PPOperatorInfo{.op = PPOperator::Identity, .precedence = 2, .isUnary = true};
        case TokenKlass::Dash:
            return PPOperatorInfo{.op = PPOperator::Negate, .precedence = 2, .isUnary = true};
        case TokenKlass::Tilde:
            return PPOperatorInfo{.op = PPOperator::BitNot, .precedence = 2, .isUnary = true};
        case TokenKlass::Bang:
            return PPOperatorInfo{.op = PPOperator::LogicalNot, .precedence = 2, .isUnary = true};
        default:
            return std::nullopt;
        }
    }

    static auto ParseBinaryPPOperator(const PPToken& token) -> std::optional<PPOperatorInfo>
    {
        switch (token.klass) {
        case TokenKlass::Star:
            return PPOperatorInfo{.op = PPOperator::Multiply, .precedence = 3, .isUnary = false};
        case TokenKlass::Slash:
            return PPOperatorInfo{.op = PPOperator::Divide, .precedence = 3, .isUnary = false};
        case TokenKlass::Percent:
            return PPOperatorInfo{.op = PPOperator::Modulo, .precedence = 3, .isUnary = false};
        case TokenKlass::Plus:
            return PPOperatorInfo{.op = PPOperator::Add, .precedence = 4, .isUnary = false};
        case TokenKlass::Dash:
            return PPOperatorInfo{.op = PPOperator::Subtract, .precedence = 4, .isUnary = false};
        case TokenKlass::LShift:
            return PPOperatorInfo{.op = PPOperator::LShift, .precedence = 5, .isUnary = false};
        case TokenKlass::RShift:
            return PPOperatorInfo{.op = PPOperator::RShift, .precedence = 5, .isUnary = false};
        case TokenKlass::LAngle:
            return PPOperatorInfo{.op = PPOperator::Less, .precedence = 6, .isUnary = false};
        case TokenKlass::LessEq:
            return PPOperatorInfo{.op = PPOperator::LessEq, .precedence = 6, .isUnary = false};
        case TokenKlass::RAngle:
            return PPOperatorInfo{.op = PPOperator::Greater, .precedence = 6, .isUnary = false};
        case TokenKlass::GreaterEq:
            return PPOperatorInfo{.op = PPOperator::GreaterEq, .precedence = 6, .isUnary = false};
        case TokenKlass::Equal:
            return PPOperatorInfo{.op = PPOperator::Equal, .precedence = 7, .isUnary = false};
        case TokenKlass::NotEqual:
            return PPOperatorInfo{.op = PPOperator::NotEqual, .precedence = 7, .isUnary = false};
        case TokenKlass::Ampersand:
            return PPOperatorInfo{.op = PPOperator::BitAnd, .precedence = 8, .isUnary = false};
        case TokenKlass::Caret:
            return PPOperatorInfo{.op = PPOperator::BitXor, .precedence = 9, .isUnary = false};
        case TokenKlass::VerticalBar:
            return PPOperatorInfo{.op = PPOperator::BitOr, .precedence = 10, .isUnary = false};
        case TokenKlass::And:
            return PPOperatorInfo{.op = PPOperator::LogicalAnd, .precedence = 11, .isUnary = false};
        case TokenKlass::Or:
            return PPOperatorInfo{.op = PPOperator::LogicalOr, .precedence = 12, .isUnary = false};
        default:
            return std::nullopt;
        }
    }

    // For parentheses, this function returns 0.
    // For unary operators, the right-hand side is NOT used.
    // This function returns 0 for invalid operations such as division by zero.
    static auto EvaluatePPOperator(PPOperator op, int64_t lhs, int64_t rhs = 0) -> int64_t
    {
        switch (op) {
        case PPOperator::LParen:
            return 0;
        case PPOperator::Identity:
            return lhs;
        case PPOperator::Negate:
            return -lhs;
        case PPOperator::BitNot:
            return ~lhs;
        case PPOperator::LogicalNot:
            return !lhs;
        case PPOperator::Multiply:
            return lhs * rhs;
        case PPOperator::Divide:
            return rhs != 0 ? lhs / rhs : 0;
        case PPOperator::Modulo:
            return rhs != 0 ? lhs % rhs : 0;
        case PPOperator::Add:
            return lhs + rhs;
        case PPOperator::Subtract:
            return lhs - rhs;
        case PPOperator::LShift:
            return lhs << rhs;
        case PPOperator::RShift:
            return lhs >> rhs;
        case PPOperator::Less:
            return lhs < rhs;
        case PPOperator::LessEq:
            return lhs <= rhs;
        case PPOperator::Greater:
            return lhs > rhs;
        case PPOperator::GreaterEq:
            return lhs >= rhs;
        case PPOperator::Equal:
            return lhs == rhs;
        case PPOperator::NotEqual:
            return lhs != rhs;
        case PPOperator::BitAnd:
            return lhs & rhs;
        case PPOperator::BitXor:
            return lhs ^ rhs;
        case PPOperator::BitOr:
            return lhs | rhs;
        case PPOperator::LogicalAnd:
            return lhs && rhs;
        case PPOperator::LogicalOr:
            return lhs || rhs;
        }

        return 0;
    }

    static auto EvaluatePPExpressionAux(ArrayView<PPToken> tokens) -> bool
    {
        bool expectBinaryOperator = false;
        std::vector<int64_t> evalStack;
        std::vector<PPOperatorInfo> operatorStack;

        auto evalOperatorOnTop = [&]() -> bool {
            if (operatorStack.empty()) {
                return false;
            }

            auto& op = operatorStack.back();
            if (op.op == PPOperator::LParen) {
                return false;
            }

            uint64_t lhs = 0;
            uint64_t rhs = 0;
            if (op.isUnary) {
                if (evalStack.empty()) {
                    return false;
                }
                lhs = evalStack.back();
                evalStack.pop_back();
            }
            else {
                if (evalStack.size() < 2) {
                    return false;
                }
                rhs = evalStack.back();
                evalStack.pop_back();
                lhs = evalStack.back();
                evalStack.pop_back();
            }

            evalStack.push_back(EvaluatePPOperator(op.op, lhs, rhs));
            operatorStack.pop_back();
            return true;
        };

        auto pushOperator = [&](PPOperatorInfo op) {
            // All unary operators are right-associative and binary operators are left-associative for now.
            bool leftAssoc = !op.isUnary;
            while (!operatorStack.empty()) {
                if ((leftAssoc && operatorStack.back().precedence >= op.precedence) ||
                    (!leftAssoc && operatorStack.back().precedence > op.precedence)) {
                    if (!evalOperatorOnTop()) {
                        return false;
                    }
                }
                else {
                    break;
                }
            }
            operatorStack.push_back(op);
            return true;
        };

        PPTokenScanner scanner{tokens};
        while (!scanner.CursorAtEnd()) {
            auto token = scanner.ConsumeToken();

            if (expectBinaryOperator) {
                if (auto ppOpInfo = ParseBinaryPPOperator(token)) {
                    if (!pushOperator(*ppOpInfo)) {
                        // FIXME: report error, failed to evaluate the expression somehow
                        return false;
                    }
                    expectBinaryOperator = false;
                }
                else {
                    // FIXME: report error, expected a binary operator
                    return false;
                }
            }
            else {
                if (token.klass == TokenKlass::IntegerConstant) {
                    auto value = ParseNumberLiteral(token.text.StrView());
                    if (value.IsScalarInt32()) {
                        evalStack.push_back(value.GetInt32Value());
                    }
                    else if (value.IsScalarUInt32()) {
                        evalStack.push_back(value.GetUInt32Value());
                    }
                    else {
                        // FIXME: report error, bad integer literal
                        return false;
                    }
                    expectBinaryOperator = true;
                }
                else if (token.klass == TokenKlass::DefinedYes) {
                    evalStack.push_back(1);
                    expectBinaryOperator = true;
                }
                else if (token.klass == TokenKlass::DefinedNo || token.klass == TokenKlass::Identifier) {
                    // NOTE macros are already expanded at this point. Unknown identifier is treated as 0.
                    evalStack.push_back(0);
                    expectBinaryOperator = true;
                }
                else if (token.klass == TokenKlass::LParen) {
                    operatorStack.push_back(GetParenPPOperator());
                    expectBinaryOperator = false;
                }
                else if (token.klass == TokenKlass::RParen) {
                    while (!operatorStack.empty() && operatorStack.back().op != PPOperator::LParen) {
                        if (!evalOperatorOnTop()) {
                            // FIXME: report error, failed to evaluate the expression somehow
                            return false;
                        }
                    }

                    if (!operatorStack.empty()) {
                        GLSLD_ASSERT(operatorStack.back().op == PPOperator::LParen);
                        operatorStack.pop_back();
                    }
                    else {
                        // FIXME: report error, mismatched parenthesis
                        return false;
                    }
                    expectBinaryOperator = true;
                }
                else if (auto ppOpInfo = ParseUnaryPPOperator(token)) {
                    if (!pushOperator(*ppOpInfo)) {
                        // FIXME: report error, failed to evaluate the expression somehow
                        return false;
                    }
                    expectBinaryOperator = false;
                }
                else {
                    // FIXME: report error, expected a unary operator or literal
                    return false;
                }
            }
        }

        while (!operatorStack.empty()) {
            if (!evalOperatorOnTop()) {
                // FIXME: report error, failed to evaluate the expression somehow
                return false;
            }
        }

        return evalStack.size() == 1 && evalStack.back() != 0;
    }

    auto PreprocessStateMachine::EvaluatePPExpression(PPTokenScanner& scanner) -> bool
    {
        // Expand macros and evaluate the expression.
        std::vector<PPToken> tokenBuffer;
        MacroExpansionProcessor processor{*this, &tokenBuffer};

        // Our approach handling defined(X) is going to conflict with the macro expansion.
        // However, it is not a problem since it's a UB.
        while (!scanner.CursorAtEnd()) {
            auto token = scanner.ConsumeToken();
            if (token.klass == TokenKlass::Identifier && token.text == atomMacroOperatorDefined) {
                PPToken macroName;
                if (scanner.TryTestToken(TokenKlass::Identifier)) {
                    macroName = scanner.ConsumeToken();
                }
                else if (scanner.TryTestToken(TokenKlass::LParen) && scanner.TryTestToken(TokenKlass::Identifier, 1) &&
                         scanner.TryTestToken(TokenKlass::RParen, 2)) {
                    scanner.ConsumeToken();
                    macroName = scanner.ConsumeToken();
                    scanner.ConsumeToken();
                }
                else {
                    // FIXME: report error, expected a macro name
                    return false;
                }

                bool isDefined = macroTable.IsMacroDefined(macroName.text);

                if (callback) {
                    callback->OnDefinedOperator(macroName, isDefined);
                }

                processor.Feed(PPToken{
                    .klass                = isDefined ? TokenKlass::DefinedYes : TokenKlass::DefinedNo,
                    .spelledFile          = token.spelledFile,
                    .spelledRange         = token.spelledRange,
                    .text                 = {},
                    .isFirstTokenOfLine   = token.isFirstTokenOfLine,
                    .hasLeadingWhitespace = token.hasLeadingWhitespace,
                });
            }
            else {
                processor.Feed(token);
            }
        }
        processor.Finalize();

        return EvaluatePPExpressionAux(tokenBuffer);
    }
#pragma endregion

} // namespace glsld
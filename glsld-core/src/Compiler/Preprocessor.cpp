#include "Compiler/Preprocessor.h"
#include "Compiler/PPEval.h"
#include "Compiler/Tokenizer.h"
#include "Compiler/SyntaxToken.h"
#include "Language/ShaderTarget.h"
#include "Support/ScopeExit.h"

#include <string>

namespace glsld
{
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

        auto beginTokenIndex   = outputStream.tokens.size();
        auto beginCommentIndex = outputStream.comments.size();
        auto sourceText        = sourceManager.GetSourceText(sourceFile);
        Tokenizer{compiler, *this, atomTable, sourceFile, sourceText}.DoTokenize();

        outputStream.files.push_back(PreprocessedFile{
            .fileID            = sourceFile,
            .beginTokenIndex   = beginTokenIndex,
            .endTokenIndex     = outputStream.tokens.size(),
            .beginCommentIndex = beginCommentIndex,
            .endCommentIndex   = outputStream.comments.size(),
        });
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
        GLSLD_ASSERT(token.klass != TokenKlass::Comment);

        switch (state) {
        case PreprocessorState::Default:
            AcceptOnDefaultState(token);
            break;

        case PreprocessorState::Halt:
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
            directiveTokBuffer.push_back(token);
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
            directiveTokBuffer.push_back(token);
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
            ParsePPDirective();
            if (conditionalStack.empty() || conditionalStack.back().active) {
                RedirectIncomingToken(PreprocessorState::Default, token);
            }
            else {
                RedirectIncomingToken(PreprocessorState::Inactive, token);
            }
        }
        else if (token.klass == TokenKlass::Identifier) {
            // A PP directive parsed.
            directiveTokBuffer.push_back(token);
            if (conditionalStack.empty() || conditionalStack.back().active) {
                // We are in an active region, so we expect to process all directives.
                if (token.text == atomDirectiveInclude) {
                    TransitionTo(PreprocessorState::ExpectIncludeDirectiveTail);
                }
                else {
                    TransitionTo(PreprocessorState::ExpectDefaultDirectiveTail);
                }
            }
            else {
                // We are in an inactive region.
                if (token.text == atomDirectiveElif || token.text == atomDirectiveElse ||
                    token.text == atomDirectiveEndif) {
                    // These directives may change the state of the conditional stack.
                    TransitionTo(PreprocessorState::ExpectDefaultDirectiveTail);
                }
                else {
                    // Other directives are skipped in inactive regions.
                    TransitionTo(PreprocessorState::Inactive);
                }
            }
        }
        else {
            // A bad directive.
            directiveTokBuffer.push_back(token);
            TransitionTo(PreprocessorState::ExpectDefaultDirectiveTail);
        }
    }

    auto PreprocessStateMachine::AcceptOnExpectDirectiveTailState(const PPToken& token) -> void
    {
        if (token.klass == TokenKlass::Eof || token.isFirstTokenOfLine) {
            // Finish processing the directive.
            ParsePPDirective();
            directiveTokBuffer.clear();

            // Redirect the token to the default state.
            if (conditionalStack.empty() || conditionalStack.back().active) {
                RedirectIncomingToken(PreprocessorState::Default, token);
            }
            else {
                RedirectIncomingToken(PreprocessorState::Inactive, token);
            }
        }
        else {
            directiveTokBuffer.push_back(token);
        }
    }

    auto PreprocessStateMachine::ParsePPDirective() -> void
    {
        PPTokenScanner scanner{directiveTokBuffer};
        scanner.ConsumeToken(); // Consume '#'
        if (scanner.CursorAtEnd()) {
            // Empty directive, nothing to do
            return;
        }

        PPToken directiveToken = scanner.ConsumeToken();
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

    auto PreprocessStateMachine::HandleBadDirective(PPTokenScanner& scanner) -> void
    {
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
                callback->OnIncludeDirective(scanner.AllTokens(), *headerNameToken,
                                             sourceManager.GetAbsolutePath(includeFile));
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
                callback->OnDefineDirective(scanner.AllTokens(), macroName, {}, {}, false);
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
        std::vector<PPToken> replacementTokens;
        while (!scanner.CursorAtEnd()) {
            replacementTokens.push_back(scanner.ConsumeToken());
        }

        // Run PP callback event if any
        if (callback) {
            callback->OnDefineDirective(scanner.AllTokens(), macroName, paramTokens, replacementTokens, isFunctionLike);
        }

        // Register the macro
        if (isFunctionLike) {
            macroTable.DefineFunctionLikeMacro(macroName, std::move(paramTokens), std::move(replacementTokens));
        }
        else {
            macroTable.DefineObjectLikeMacro(macroName, std::move(replacementTokens));
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
            callback->OnUndefDirective(scanner.AllTokens(), macroName);
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
            callback->OnIfDirective(scanner.AllTokens(), evalToTrue);
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

        bool isActive = macroTable.IsMacroDefined(macroName.text) != isNDef;

        // Run PP callback event if any
        if (callback) {
            callback->OnIfDefDirective(scanner.AllTokens(), macroName, isNDef, isActive);
        }

        conditionalStack.push_back(PPConditionalInfo{
            .active           = isActive,
            .seenActiveBranch = isActive,
            .seenElse         = false,
        });

#if defined(GLSLD_DEBUG)
        compiler.GetCompilerTrace().TracePPConditionalInfo(isActive);
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
            callback->OnElifDirective(scanner.AllTokens(), evalToTrue);
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
            callback->OnElseDirective(scanner.AllTokens());
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
            callback->OnEndifDirective(scanner.AllTokens());
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
            callback->OnExtensionDirective(scanner.AllTokens(), *extension, *behavior);
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
            callback->OnVersionDirective(scanner.AllTokens(), *version, *profile);
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

        auto result = PPExprEvaluator{}.Evaluate(tokenBuffer);
        return result.has_value() && *result != 0;
    }
#pragma endregion

} // namespace glsld
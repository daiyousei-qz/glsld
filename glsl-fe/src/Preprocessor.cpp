#include "Preprocessor.h"
#include "Tokenizer.h"
#include "SourceContext.h"
#include "SyntaxToken.h"
#include "LexContext.h"

namespace glsld
{
    // A simple scanner for a preprosessing token stream. It is used to parse preprocessor directives.
    class PPTokenScanner final
    {
    public:
        PPTokenScanner() = default;
        PPTokenScanner(const PPToken* tokBegin, const PPToken* tokEnd)
            : tokBegin(tokBegin), tokEnd(tokEnd), tokCursor(tokBegin)
        {
        }

        auto CursorAtBegin() const noexcept -> bool
        {
            return tokCursor == tokBegin;
        }

        auto CursorAtEnd() const noexcept -> bool
        {
            return tokCursor == tokEnd;
        }

        auto PeekToken() const noexcept -> const PPToken&
        {
            return *tokCursor;
        }

        auto TryConsumeToken(TokenKlass klass) -> std::optional<PPToken>
        {
            if (!CursorAtEnd() && tokCursor->klass == klass) {
                return ConsumeToken();
            }

            return std::nullopt;
        }

        auto TryConsumeToken(TokenKlass klass1, TokenKlass klass2) -> std::optional<PPToken>
        {
            if (!CursorAtEnd() && (tokCursor->klass == klass1 || tokCursor->klass == klass2)) {
                return ConsumeToken();
            }

            return std::nullopt;
        }

        auto ConsumeToken() -> PPToken
        {
            return *tokCursor++;
        }

    private:
        const PPToken* tokBegin  = nullptr;
        const PPToken* tokEnd    = nullptr;
        const PPToken* tokCursor = nullptr;
    };

    auto Preprocessor::DispatchTokenToHandler(const PPToken& token) -> void
    {
        GLSLD_ASSERT(token.klass != TokenKlass::Comment);
        switch (state) {
        case PreprocessorState::Default:
            AcceptOnDefaultState(token);
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

    auto Preprocessor::AcceptOnDefaultState(const PPToken& token) -> void
    {
        if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
            state = PreprocessorState::ExpectDirective;
        }
        else if (token.klass == TokenKlass::Eof) {
            macroExpansionProcessor.Finalize();
            if (compilerObject.GetPreprocessContext().GetIncludeDepth() == 0) {
                // We are done with the main file. Insert an EOF token.
                compilerObject.GetLexContext().AddToken(token, token.spelledRange);
            }
        }
        else {
            macroExpansionProcessor.Feed(token);
        }
    }

    auto Preprocessor::AcceptOnInactiveState(const PPToken& token) -> void
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

    auto Preprocessor::AcceptOnExpectDirectiveState(const PPToken& token) -> void
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

    auto Preprocessor::AcceptOnExpectDirectiveTailState(const PPToken& token) -> void
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

    auto Preprocessor::HandleDirective(const PPToken& directiveToken, ArrayView<PPToken> restTokens) -> void
    {
        PPTokenScanner scanner{restTokens.data(), restTokens.data() + restTokens.size()};
        if (directiveToken.text == "include") {
            HandleIncludeDirective(scanner);
        }
        else if (directiveToken.text == "define") {
            HandleDefineDirective(scanner);
        }
        else if (directiveToken.text == "undef") {
            HandleUndefDirective(scanner);
        }
        else if (directiveToken.text == "if") {
            HandleIfDirective(scanner);
        }
        else if (directiveToken.text == "ifdef") {
            HandleIfdefDirective(scanner, false);
        }
        else if (directiveToken.text == "ifndef") {
            HandleIfdefDirective(scanner, true);
        }
        else if (directiveToken.text == "else") {
            HandleElseDirective(scanner);
        }
        else if (directiveToken.text == "elif") {
            HandleElifDirective(scanner);
        }
        else if (directiveToken.text == "endif") {
            HandleEndifDirective(scanner);
        }
        else {
            // We ignore other directives for now
            // FIXME: support other directives
        }
    }

    auto Preprocessor::HandleIncludeDirective(PPTokenScanner& scanner) -> void
    {
        const auto& compilerConfig = compilerObject.GetConfig();
        if (compilerObject.GetPreprocessContext().GetIncludeDepth() >= compilerConfig.maxIncludeDepth) {
            // FIXME: report error, too many nested include files
            return;
        }

        if (auto headerNameToken = scanner.TryConsumeToken(TokenKlass::QuotedString, TokenKlass::AngleString)) {
            if (!scanner.CursorAtEnd()) {
                // FIXME: report warning, extra tokens after the header file name
            }

            // Run PP callback event if any
            if (callback) {
                callback->OnIncludeDirective(*headerNameToken);
            }

            // Search for the header file in the include paths and load the source text.
            StringView headerName                  = headerNameToken->text.StrView().Drop(1).DropBack(1);
            const SourceFileEntry* sourceFileEntry = nullptr;
            for (const auto& includePath : compilerConfig.includePaths) {
                sourceFileEntry = compilerObject.GetSourceContext().OpenFromFile(includePath / headerName.StdStrView());
                if (sourceFileEntry) {
                    break;
                }
            }

            if (sourceFileEntry) {
                GLSLD_ASSERT(sourceFileEntry->GetSourceText().has_value());

                // We create a new preprocessor and lexer to process the included file.
                GLSLD_TRACE_ENTER_INCLUDE_FILE(headerName);
                compilerObject.GetPreprocessContext().EnterIncludeFile();
                if (callback) {
                    callback->OnEnterIncludedFile();
                }
                Preprocessor nextPP{compilerObject, callback,
                                    includeExpansionRange ? includeExpansionRange
                                                          : TextRange{headerNameToken->spelledRange.start}};
                Tokenizer{compilerObject, nextPP, sourceFileEntry->GetID(), *sourceFileEntry->GetSourceText()}
                    .DoTokenize();
                if (callback) {
                    callback->OnExitIncludedFile();
                }
                compilerObject.GetPreprocessContext().ExitIncludeFile();
                GLSLD_TRACE_EXIT_INCLUDE_FILE(headerName);
            }
            else {
                // FIXME: report error, cannot find the header file
            }
        }
        else {
            // FIXME: report error, expected a header file name
        }
    }

    auto Preprocessor::HandleDefineDirective(PPTokenScanner& scanner) -> void
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
            compilerObject.GetPreprocessContext().DefineObjectLikeMacro(macroName, {});
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
                while (true) {
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
            callback->OnDefineDirective(macroName, paramTokens);
        }

        // Register the macro
        if (isFunctionLike) {
            compilerObject.GetPreprocessContext().DefineFunctionLikeMacro(macroName, std::move(paramTokens),
                                                                          std::move(expansionTokens));
        }
        else {
            compilerObject.GetPreprocessContext().DefineObjectLikeMacro(macroName, std::move(expansionTokens));
        }
    }

    auto Preprocessor::HandleUndefDirective(PPTokenScanner& scanner) -> void
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
        compilerObject.GetPreprocessContext().UndefineMacro(macroName.text);
    }

    auto Preprocessor::HandleIfDirective(PPTokenScanner& scanner) -> void
    {
    }

    auto Preprocessor::HandleIfdefDirective(PPTokenScanner& scanner, bool isNDef) -> void
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

        auto macroDef = compilerObject.GetPreprocessContext().FindMacroDefinition(macroName.text);
        bool active   = (macroDef != nullptr) != isNDef;
        conditionalStack.push_back(PPConditionalInfo{
            .active           = active,
            .seenActiveBranch = active,
            .seenElse         = false,
        });
    }

    auto Preprocessor::HandleElseDirective(PPTokenScanner& scanner) -> void
    {
        if (!scanner.CursorAtEnd()) {
            // FIXME: report warning, expected no more tokens after the directive.
        }

        if (conditionalStack.empty()) {
            // FIXME: report warning, standalone #else directive
            return;
        }

        auto& conditionalInfo = conditionalStack.back();
        if (!conditionalInfo.seenElse) {
            conditionalInfo.active   = !conditionalInfo.seenActiveBranch;
            conditionalInfo.seenElse = true;
        }
        else {
            // FIXME: report error, an extraous #else directive
            conditionalInfo.active = false;
        }
    }

    auto Preprocessor::HandleElifDirective(PPTokenScanner& scanner) -> void
    {
    }

    auto Preprocessor::HandleEndifDirective(PPTokenScanner& scanner) -> void
    {
        if (!scanner.CursorAtEnd()) {
            // FIXME: report warning, expected no more tokens after the directive.
        }

        if (!conditionalStack.empty()) {
            conditionalStack.pop_back();
        }
        else {
            // FIXME: report warning, unmatched #endif directive
        }
    }

} // namespace glsld
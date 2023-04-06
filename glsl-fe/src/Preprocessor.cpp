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
        PPTokenScanner(const PPTokenData* tokBegin, const PPTokenData* tokEnd)
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

        auto PeekToken() const noexcept -> const PPTokenData&
        {
            return *tokCursor;
        }

        auto TryConsumeToken(TokenKlass klass) -> std::optional<PPTokenData>
        {
            if (!CursorAtEnd() && tokCursor->klass == klass) {
                return ConsumeToken();
            }

            return std::nullopt;
        }

        auto TryConsumeToken(TokenKlass klass1, TokenKlass klass2) -> std::optional<PPTokenData>
        {
            if (!CursorAtEnd() && (tokCursor->klass == klass1 || tokCursor->klass == klass2)) {
                return ConsumeToken();
            }

            return std::nullopt;
        }

        auto ConsumeToken() -> PPTokenData
        {
            return *tokCursor++;
        }

    private:
        const PPTokenData* tokBegin  = nullptr;
        const PPTokenData* tokEnd    = nullptr;
        const PPTokenData* tokCursor = nullptr;
    };

    auto Preprocessor::HandleDirective(const PPTokenData& directiveToken, ArrayView<PPTokenData> restTokens) -> void
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
        if (compilerObject.GetPreprocessContext().GetIncludeDepth() >= 16) {
            // FIXME: report error, too many nested include files
        }

        if (auto headerNameToken = scanner.TryConsumeToken(TokenKlass::QuotedString, TokenKlass::AngleString)) {
            if (!scanner.CursorAtEnd()) {
                // FIXME: report warning, extra tokens after the header file name
            }

            std::optional<StringView> sourceText;

            const auto& compilerConfig = compilerObject.GetConfig();
            StringView headerName      = headerNameToken->text.StrView().Drop(1).DropBack(1);
            for (const auto& includePath : compilerConfig.includePaths) {
                auto headerPath = includePath / headerName.StdStrView();
                sourceText      = compilerObject.GetSourceContext().GetSourceView(headerPath.string());
                if (sourceText) {
                    break;
                }
            }

            if (sourceText) {
                // We create a new preprocessor and lexer to process the included file.
                TraceEnterIncludeFile(headerName);
                compilerObject.GetPreprocessContext().EnterIncludeFile();
                Preprocessor nextPP{compilerObject, 1};
                Tokenizer{compilerObject, nextPP, *sourceText}.DoTokenize();
                compilerObject.GetPreprocessContext().ExitIncludeFile();
                TraceExitIncludeFile(headerName);
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
        PPTokenData macroName;
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
        std::vector<PPTokenData> paramTokens;
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
        std::vector<PPTokenData> expansionTokens;
        while (!scanner.CursorAtEnd()) {
            expansionTokens.push_back(scanner.ConsumeToken());
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
        PPTokenData macroName;
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
        PPTokenData macroName;
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
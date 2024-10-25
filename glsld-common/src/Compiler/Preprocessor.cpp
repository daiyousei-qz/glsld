#include "Compiler/Preprocessor.h"
#include "Compiler/Tokenizer.h"
#include "Compiler/SourceContext.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/LexContext.h"
#include "Language/ShaderTarget.h"
#include "Language/ConstValue.h"

namespace glsld
{
    auto Preprocessor::PreprocessSourceFile(FileID sourceFile) -> void
    {
        if (!sourceFile.IsValid()) {
            // FIXME: report error, invalid source file
            return;
        }

        if (versionScanningMode) {
            AtomTable atomTable;
            Tokenizer{compilerObject, atomTable, *this, sourceFile}.DoTokenize();
        }
        else {
            Tokenizer{compilerObject, compilerObject.GetLexContext().GetAtomTable(), *this, sourceFile}.DoTokenize();
        }
    }

    auto Preprocessor::DispatchTokenToHandler(const PPToken& token) -> void
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

    auto Preprocessor::AcceptOnDefaultState(const PPToken& token) -> void
    {
        if (token.klass == TokenKlass::Hash && token.isFirstTokenOfLine) {
            TransitionTo(PreprocessorState::ExpectDirective);
        }
        else {
            if (token.klass != TokenKlass::Comment) {
                if (versionScanningMode) {
                    TransitionTo(PreprocessorState::Halt);
                }
                else {
                    // FIXME: we ignore comment for now.
                    macroExpansionProcessor.Feed(token);
                }
            }
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
        else if (token.klass == TokenKlass::Comment) {
            // FIXME: we ignore comment for now.
            return;
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

    auto Preprocessor::HandleDirective(const PPToken& directiveToken, ArrayView<PPToken> restTokens) -> void
    {
        PPTokenScanner scanner{restTokens};
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
        else if (directiveToken.text == "error") {
            // FIXME: report error
        }
        else if (directiveToken.text == "pragma") {
            // Ignore pragma directives
        }
        else if (directiveToken.text == "extension") {
            HandleExtensionDirective(scanner);
        }
        else if (directiveToken.text == "version") {
            HandleVersionDirective(scanner);
        }
        else if (directiveToken.text == "line") {
            HandleLineDirective(scanner);
        }
        else {
            // FIXME: warn about unknown directives
        }

        if (versionScanningMode) {
            // FIXME: should we also halt PP in version scanning mode for directives that are not version/extension?
        }
    }

    auto Preprocessor::HandleIncludeDirective(PPTokenScanner& scanner) -> void
    {
        const auto& compilerConfig = compilerObject.GetConfig();
        if (includeDepth >= compilerConfig.maxIncludeDepth) {
            // FIXME: report error, too many nested include files
            return;
        }

        if (auto headerNameToken = scanner.TryConsumeToken(TokenKlass::UserHeaderName, TokenKlass::SystemHeaderName)) {
            if (!scanner.CursorAtEnd()) {
                // FIXME: report warning, extra tokens after the header file name
            }

            // Run PP callback event if any
            if (callback) {
                callback->OnIncludeDirective(*headerNameToken);
            }

            // Search for the header file in the include paths and load the source text.
            StringView headerName = headerNameToken->text.StrView().Drop(1).DropBack(1);
            FileID includeFile;
            for (const auto& includePath : compilerConfig.includePaths) {
                // TODO: distinguish between system include and user include
                includeFile = compilerObject.GetSourceContext().OpenFromFile(includePath / headerName.StdStrView());
                if (includeFile.IsValid()) {
                    break;
                }
            }

            if (!includeFile.IsValid()) {
                // FIXME: report error, cannot find the header file
                return;
            }

            // We create a new preprocessor to process the included file.
            GLSLD_TRACE_ENTER_INCLUDE_FILE(headerName);
            if (callback) {
                callback->OnEnterIncludedFile();
            }
            Preprocessor nextPP{compilerObject, callback,
                                includeExpansionRange ? includeExpansionRange
                                                      : TextRange{headerNameToken->spelledRange.start},
                                includeDepth + 1};
            nextPP.PreprocessSourceFile(includeFile);
            if (callback) {
                callback->OnExitIncludedFile();
            }
            GLSLD_TRACE_EXIT_INCLUDE_FILE(headerName);
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
            compilerObject.GetLexContext().DefineObjectLikeMacro(macroName, {});
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
            callback->OnDefineDirective(macroName, paramTokens);
        }

        // Register the macro
        if (isFunctionLike) {
            compilerObject.GetLexContext().DefineFunctionLikeMacro(macroName, std::move(paramTokens),
                                                                   std::move(expansionTokens));
        }
        else {
            compilerObject.GetLexContext().DefineObjectLikeMacro(macroName, std::move(expansionTokens));
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
        // FIXME: report error to undefine a builtin macro
        compilerObject.GetLexContext().UndefineMacro(macroName.text);
    }

    auto Preprocessor::HandleIfDirective(PPTokenScanner& scanner) -> void
    {
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

        // Run PP callback event if any
        if (callback) {
            callback->OnIfDefDirective(macroName, isNDef);
        }

        bool active = compilerObject.GetLexContext().IsMacroDefined(macroName.text) != isNDef;
        conditionalStack.push_back(PPConditionalInfo{
            .active           = active,
            .seenActiveBranch = active,
            .seenElse         = false,
        });
    }

    auto Preprocessor::HandleElifDirective(PPTokenScanner& scanner) -> void
    {
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
    }

    auto Preprocessor::HandleEndifDirective(PPTokenScanner& scanner) -> void
    {
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

    auto ParseExtensionBehavior(const PPToken& toggle) -> std::optional<ExtensionBehavior>
    {
        GLSLD_ASSERT(toggle.klass == TokenKlass::Identifier);
        if (toggle.text == "enable") {
            return ExtensionBehavior::Enable;
        }
        else if (toggle.text == "require") {
            return ExtensionBehavior::Require;
        }
        else if (toggle.text == "warn") {
            return ExtensionBehavior::Warn;
        }
        else if (toggle.text == "disable") {
            return ExtensionBehavior::Disable;
        }
        else {
            return std::nullopt;
        }
    }

    auto Preprocessor::HandleExtensionDirective(PPTokenScanner& scanner) -> void
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

    auto ParseGlslVersion(const PPToken& versionNumber) -> std::optional<GlslVersion>
    {
        if (versionNumber.klass != TokenKlass::IntegerConstant) {
            return std::nullopt;
        }

        if (versionNumber.text == "110") {
            return GlslVersion::Ver110;
        }
        else if (versionNumber.text == "120") {
            return GlslVersion::Ver120;
        }
        else if (versionNumber.text == "130") {
            return GlslVersion::Ver130;
        }
        else if (versionNumber.text == "140") {
            return GlslVersion::Ver140;
        }
        else if (versionNumber.text == "150") {
            return GlslVersion::Ver150;
        }
        else if (versionNumber.text == "330") {
            return GlslVersion::Ver330;
        }
        else if (versionNumber.text == "400") {
            return GlslVersion::Ver400;
        }
        else if (versionNumber.text == "410") {
            return GlslVersion::Ver410;
        }
        else if (versionNumber.text == "420") {
            return GlslVersion::Ver420;
        }
        else if (versionNumber.text == "430") {
            return GlslVersion::Ver430;
        }
        else if (versionNumber.text == "440") {
            return GlslVersion::Ver440;
        }
        else if (versionNumber.text == "450") {
            return GlslVersion::Ver450;
        }
        else if (versionNumber.text == "460") {
            return GlslVersion::Ver460;
        }
        else {
            return std::nullopt;
        }
    }

    auto ParseGlslProfile(const PPToken& profile) -> std::optional<GlslProfile>
    {
        GLSLD_ASSERT(profile.klass == TokenKlass::Identifier);
        if (profile.text == "core") {
            return GlslProfile::Core;
        }
        else if (profile.text == "compatibility") {
            return GlslProfile::Compatibility;
        }
        else if (profile.text == "es") {
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

    auto Preprocessor::HandleVersionDirective(PPTokenScanner& scanner) -> void
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

    auto Preprocessor::HandleLineDirective(PPTokenScanner& scanner) -> void
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

    static auto EvaluatePPExpressionAux(const LexContext& lexContext, ArrayView<PPToken> tokens) -> bool
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

    auto Preprocessor::EvaluatePPExpression(PPTokenScanner& scanner) -> bool
    {
        // Expand macros and evaluate the expression.
        std::vector<PPToken> tokenBuffer;
        MacroExpansionProcessor<ExpandToVectorCallback> processor{compilerObject.GetLexContext(),
                                                                  ExpandToVectorCallback{*this, tokenBuffer}};

        // Our approach handling defined(X) is going to conflict with the macro expansion.
        // However, it is not a problem since it's a UB.
        while (!scanner.CursorAtEnd()) {
            auto token = scanner.ConsumeToken();
            if (token.klass == TokenKlass::Identifier && token.text == "defined") {
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

                bool isDefined = compilerObject.GetLexContext().IsMacroDefined(macroName.text);
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

        return EvaluatePPExpressionAux(compilerObject.GetLexContext(), tokenBuffer);
    }
} // namespace glsld
#include "Compiler/Parser.h"
#include "Ast/Misc.h"
#include "Basic/AtomTable.h"
#include "Compiler/CompilerTrace.h"
#include "Compiler/SyntaxToken.h"

#if defined(GLSLD_ENABLE_COMPILER_TRACE)
#define GLSLD_TRACE_PARSER() ::glsld::ParserTrace glsldParserTraceObject{compiler.GetCompilerTrace(), __func__};
#else
#define GLSLD_TRACE_PARSER()
#endif

namespace glsld
{
    auto Parser::DoParse() -> void
    {
        auto ast = ParseTranslationUnit();
        GLSLD_ASSERT(ast->GetSyntaxRange().GetEndID().GetTokenIndex() == tokens.size() - 1);
        compiler.UpdateAstArtifact(tuID, ast);
    }

    auto Parser::ParseTranslationUnit() -> const AstTranslationUnit*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        std::vector<AstDecl*> decls;
        while (true) {
            while (!Eof()) {
                decls.push_back(ParseDeclAndTryRecover(nullptr, true));
            }

            if (Eof()) {
                break;
            }

            // Consumes EOF
            // TODO: should we parses preambles into a different translation unit?
            parsingInitializerList = false;
            ilistBraceDepth        = 0;

            parenDepth   = 0;
            bracketDepth = 0;
            braceDepth   = 0;
            ++currentTok;
        }

        return astBuilder.BuildTranslationUnit(CreateAstSyntaxRange(beginTokID), std::move(decls));
    }

#pragma region Parsing Misc

    auto Parser::ParseOrInferSemicolonHelper() -> void
    {
        if (InParsingMode()) {
            if (!TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expecting ';'");
                // However, we don't do error recovery as if the ';' is inferred by the parser.
            }
        }
    }

    auto Parser::ParseCommaInParenHelper(size_t leftParenDepth) -> bool
    {
        if (parenDepth < leftParenDepth) {
            // The closing ')' is already consumed, probably by another recovery process.
            return false;
        }

        if (TryTestToken(TokenKlass::RParen)) {
            return false;
        }
        else if (!TryConsumeToken(TokenKlass::Comma)) {
            ReportError("expect ',' or ')'");

            // We try to search for a ',' to recover first.
            RecoverFromError(RecoveryMode::Comma);
            if (!TryConsumeToken(TokenKlass::Comma)) {
                return false;
            }
        }

        return true;
    }

    auto Parser::ParseClosingParenHelper(size_t leftParenDepth) -> void
    {
        if (parenDepth < leftParenDepth) {
            // The closing ')' is already consumed, probably by another recovery process.
            return;
        }

        if (TryConsumeToken(TokenKlass::RParen)) {
            if (InRecoveryMode()) {
                ExitRecoveryMode();
            }
        }
        else {
            ReportError("expect ')'");

            RecoverFromError(RecoveryMode::Paren);
            if (TryConsumeToken(TokenKlass::RParen)) {
                if (InRecoveryMode()) {
                    ExitRecoveryMode();
                }
            }
            else {
                // we cannot find the closing ')' to continue parsing
                EnterRecoveryMode();
            }
        }
    }

    auto Parser::ParseClosingBracketHelper(size_t leftBracketDepth) -> void
    {
        if (bracketDepth < leftBracketDepth) {
            // The closing ']' is already consumed, probably by another recovery process.
            return;
        }

        if (TryConsumeToken(TokenKlass::RBracket)) {
            if (InRecoveryMode()) {
                ExitRecoveryMode();
            }
        }
        else {
            ReportError("expect ']'");

            RecoverFromError(RecoveryMode::Bracket);
            if (TryConsumeToken(TokenKlass::RBracket)) {
                if (InRecoveryMode()) {
                    ExitRecoveryMode();
                }
            }
            else {
                // we cannot find the closing ']' to continue parsing
                EnterRecoveryMode();
            }
        }
    }

    auto Parser::ParseOptionalDeclIdHelper() -> AstSyntaxToken
    {
        if (TryTestToken(TokenKlass::Identifier)) {
            return ParseDeclIdHelper();
        }
        else {
            ReportError("Expect identifier");
            return AstSyntaxToken{};
        }
    }

    auto Parser::ParseDeclIdHelper() -> AstSyntaxToken
    {
        GLSLD_ASSERT(PeekToken().klass == TokenKlass::Identifier);
        AstSyntaxToken result = GetCurrentToken();
        ConsumeToken();
        return result;
    }

    auto Parser::ParseParenWrappedExprOrErrorHelper() -> AstExpr*
    {
        if (TryTestToken(TokenKlass::LParen)) {
            return ParseParenWrappedExpr();
        }
        else {
            EnterRecoveryMode();
            return CreateErrorExpr();
        }
    }

    auto Parser::ParseDeclAndTryRecover(AstQualType* typeSpec, bool atGlobalScope) -> AstDecl*
    {
        auto declResult = typeSpec ? ParseDeclarationWithTypeSpec(typeSpec) : ParseDeclaration(atGlobalScope);
        if (InRecoveryMode()) {
            RecoverFromError(RecoveryMode::Semi);
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                ExitRecoveryMode();
            }
        }

        return declResult;
    }

    auto Parser::ParseStmtAndTryRecover() -> AstStmt*
    {
        auto stmtResult = ParseStmt();
        if (InRecoveryMode()) {
            RecoverFromError(RecoveryMode::Semi);
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                ExitRecoveryMode();
            }
        }

        return stmtResult;
    }

#pragma endregion

#pragma region Parsing QualType
    auto Parser::ParseLayoutQualifier(std::vector<LayoutItem>& items) -> void
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::K_layout));

        auto beginTokID = GetCurrentTokenID();
        ConsumeToken();

        if (!TryTestToken(TokenKlass::LParen)) {
            EnterRecoveryMode();
            return;
        }

        ParsingBalancedParenGuard parenGuard(*this);

        while (!Eof()) {
            auto idToken = ParseOptionalDeclIdHelper();

            AstExpr* value = nullptr;
            if (TryConsumeToken(TokenKlass::Assign)) {
                value = ParseExprNoComma();
            }

            items.push_back(LayoutItem{idToken, value});

            if (!ParseCommaInParenHelper(parenGuard.GetLeftParenDepth())) {
                break;
            }
        }
    }

    auto Parser::ParseTypeQualifierSeq() -> AstTypeQualifierSeq*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();
        QualifierGroup qualifiers;
        std::vector<LayoutItem> layoutQuals;

        bool inQualSeq = true;
        while (InParsingMode() && inQualSeq) {
            switch (PeekToken().klass) {
                // precision qualifier
            case TokenKlass::K_highp:
                qualifiers.qHighp = true;
                break;
            case TokenKlass::K_mediump:
                qualifiers.qMediump = true;
                break;
            case TokenKlass::K_lowp:
                qualifiers.qLowp = true;
                break;

                // invariance qual?
            case TokenKlass::K_invariant:
                qualifiers.qInvariant = true;
                break;

                // precise qual?
            case TokenKlass::K_precise:
                qualifiers.qPrecise = true;
                break;

                // memory qualifier
            case TokenKlass::K_coherent:
                qualifiers.qCoherent = true;
                break;
            case TokenKlass::K_volatile:
                qualifiers.qVolatile = true;
                break;
            case TokenKlass::K_restrict:
                qualifiers.qRestrict = true;
                break;
            case TokenKlass::K_readonly:
                qualifiers.qReadonly = true;
                break;
            case TokenKlass::K_writeonly:
                qualifiers.qWriteonly = true;
                break;

                // storage qual (also, parameter qual)
            case TokenKlass::K_const:
                qualifiers.qConst = true;
                break;
            case TokenKlass::K_in:
                qualifiers.qIn = true;
                break;
            case TokenKlass::K_out:
                qualifiers.qOut = true;
                break;
            case TokenKlass::K_inout:
                qualifiers.qInout = true;
                break;
            case TokenKlass::K_attribute:
                qualifiers.qAttribute = true;
                break;
            case TokenKlass::K_uniform:
                qualifiers.qUniform = true;
                break;
            case TokenKlass::K_varying:
                qualifiers.qVarying = true;
                break;
            case TokenKlass::K_buffer:
                qualifiers.qBuffer = true;
                break;
            case TokenKlass::K_shared:
                qualifiers.qShared = true;
                break;

                // auxiliary storage qual
            case TokenKlass::K_centroid:
                qualifiers.qCentroid = true;
                break;
            case TokenKlass::K_sample:
                qualifiers.qSample = true;
                break;
            case TokenKlass::K_patch:
                qualifiers.qPatch = true;
                break;

                // Interpolation qual
            case TokenKlass::K_smooth:
                qualifiers.qSmooth = true;
                break;
            case TokenKlass::K_flat:
                qualifiers.qFlat = true;
                break;
            case TokenKlass::K_noperspective:
                qualifiers.qNoperspective = true;
                break;

                // Extension: ray tracing
            case TokenKlass::K_rayPayloadNV:
            case TokenKlass::K_rayPayloadEXT:
                qualifiers.qRayPayloadEXT = true;
                break;
            case TokenKlass::K_rayPayloadInNV:
            case TokenKlass::K_rayPayloadInEXT:
                qualifiers.qRayPayloadInEXT = true;
                break;
            case TokenKlass::K_hitAttributeNV:
            case TokenKlass::K_hitAttributeEXT:
                qualifiers.qHitAttributeEXT = true;
                break;
            case TokenKlass::K_callableDataNV:
            case TokenKlass::K_callableDataEXT:
                qualifiers.qCallableDataEXT = true;
                break;
            case TokenKlass::K_callableDataInNV:
            case TokenKlass::K_callableDataInEXT:
                qualifiers.qCallableDataInEXT = true;
                break;

                // Extension: mesh shader
            case TokenKlass::K_perprimitiveNV:
            case TokenKlass::K_perprimitiveEXT:
                qualifiers.qPerprimitiveNV = true;
                break;
            case TokenKlass::K_perviewNV:
                qualifiers.qPerviewNV = true;
                break;
            case TokenKlass::K_taskNV:
                qualifiers.qTaskNV = true;
                break;
            case TokenKlass::K_taskPayloadSharedEXT:
                qualifiers.qTaskPayloadSharedEXT = true;
                break;

                // Not a qualifier
            default:
                inQualSeq = false;
                break;
            }

            if (inQualSeq) {
                ConsumeToken();
            }

            // Handle layout qualifier
            if (TryTestToken(TokenKlass::K_layout)) {
                ParseLayoutQualifier(layoutQuals);
                inQualSeq = true;
            }
        }

        if (GetCurrentTokenID() != beginTokID) {
            // We've parsed a qualifier sequence if we've consumed at least one token.
            return astBuilder.BuildTypeQualifierSeq(CreateAstSyntaxRange(beginTokID), qualifiers,
                                                    std::move(layoutQuals));
        }
        else {
            // No token consumed, aka. no qualifiers.
            return nullptr;
        }
    }

    auto Parser::ParseStructBody() -> std::vector<AstStructFieldDecl*>
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        ConsumeToken();

        if (TryConsumeToken(TokenKlass::RBrace)) {
            // Empty struct body
            return {};
        }

        std::vector<AstStructFieldDecl*> result;
        while (!Eof()) {
            // Fast path. Parse an empty decl
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                // TODO: report warning
                continue;
            }

            // Parse a member decl
            auto beginTokID  = GetCurrentTokenID();
            auto typeResult  = ParseQualType();
            auto declarators = ParseDeclaratorListNoInit();

            if (!InRecoveryMode()) {
                result.push_back(astBuilder.BuildStructFieldDecl(CreateAstSyntaxRange(beginTokID), typeResult,
                                                                 std::move(declarators)));
                ParseOrInferSemicolonHelper();
            }
            else {
                // Try to resume if we see a ';'
                if (TryConsumeToken(TokenKlass::Semicolon)) {
                    ExitRecoveryMode();
                }
                else {
                    break;
                }
            }

            if (TryTestToken(TokenKlass::RBrace)) {
                break;
            }
        }

        if (!TryConsumeToken(TokenKlass::RBrace)) {
            RecoverFromError(RecoveryMode::Brace);
            if (TryConsumeToken(TokenKlass::RBrace) && InRecoveryMode()) {
                ExitRecoveryMode();
            }
        }

        return result;
    }

    auto Parser::ParseStructDefinition() -> AstStructDecl*
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::K_struct));
        auto beginTokID = GetCurrentTokenID();
        ConsumeToken();

        // Parse the declared struct type name
        std::optional<AstSyntaxToken> declTok = std::nullopt;
        if (TryTestToken(TokenKlass::Identifier)) {
            declTok = GetCurrentToken();
            ConsumeToken();
        }

        // Parse the struct body
        if (TryTestToken(TokenKlass::LBrace)) {
            auto structBodyResult = ParseStructBody();
            return astBuilder.BuildStructDecl(CreateAstSyntaxRange(beginTokID), declTok, std::move(structBodyResult));
        }
        else {
            // FIXME: could we do better on recovery?
            EnterRecoveryMode();
            return astBuilder.BuildStructDecl(CreateAstSyntaxRange(beginTokID), declTok, {});
        }
    }

    auto Parser::ParseArraySpec() -> AstArraySpec*
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(PeekToken().klass == TokenKlass::LBracket);
        auto beginTokID = GetCurrentTokenID();

        std::vector<AstExpr*> sizes;
        while (TryTestToken(TokenKlass::LBracket)) {
            sizes.push_back(ParseBracketWrappedExpr(false));
        }

        return astBuilder.BuildArraySpec(CreateAstSyntaxRange(beginTokID), std::move(sizes));
    }

    auto Parser::ParseTypeSpec(AstTypeQualifierSeq* quals) -> AstQualType*
    {
        GLSLD_TRACE_PARSER();

        // The range of AstQualType should be at the start of AstTypeQualifierSeq
        auto beginTokID = quals ? quals->GetSyntaxRange().GetBeginID() : GetCurrentTokenID();

        if (TryTestToken(TokenKlass::K_struct)) {
            // Parse struct definition
            auto structDefinition   = ParseStructDefinition();
            AstArraySpec* arraySpec = nullptr;
            if (TryTestToken(TokenKlass::LBracket)) {
                arraySpec = ParseArraySpec();
            }
            return astBuilder.BuildQualType(CreateAstSyntaxRange(beginTokID), quals, structDefinition, arraySpec);
        }
        else if (TryTestToken(TokenKlass::Identifier) || GetGlslBuiltinType(PeekToken().klass).has_value()) {
            // Parse type name, which is either an identifier or a keyword
            auto typeNameTok = GetCurrentToken();
            ConsumeToken();

            AstArraySpec* arraySpec = nullptr;
            if (TryTestToken(TokenKlass::LBracket)) {
                arraySpec = ParseArraySpec();
            }
            return astBuilder.BuildQualType(CreateAstSyntaxRange(beginTokID), quals, typeNameTok, arraySpec);
        }
        else {
            EnterRecoveryMode();
            return astBuilder.BuildQualType(CreateAstSyntaxRange(beginTokID), quals, AstSyntaxToken{}, nullptr);
        }
    }

    auto Parser::ParseQualType() -> AstQualType*
    {
        GLSLD_TRACE_PARSER();

        auto qualifiers = ParseTypeQualifierSeq();
        return ParseTypeSpec(qualifiers);
    }

#pragma endregion

#pragma region Parsing Decl

    auto Parser::ParseDeclaration(bool atGlobalScope) -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        if (TryConsumeToken(TokenKlass::Semicolon)) {
            // empty decl
            return astBuilder.BuildEmptyDecl(CreateAstSyntaxRange(beginTokID));
        }

        if (atGlobalScope && TryTestToken(TokenKlass::K_precision)) {
            // precision decl
            return ParsePrecisionDecl();
        }

        auto quals = ParseTypeQualifierSeq();
        if (atGlobalScope && quals) {
            if (TryTestToken(TokenKlass::Semicolon)) {
                // default qualifier decl
                // e.g. `layout(local_size_x = X​, local_size_y = Y​, local_size_z = Z​) in;`
                // FIXME: implement this
                ReportError("default qualifier decl not supported yet");
                EnterRecoveryMode();
                return astBuilder.BuildErrorDecl(CreateAstSyntaxRange(beginTokID));
            }

            if (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::Semicolon, TokenKlass::Comma, 1)) {
                // qualifier overwrite decl
                // e.g. `invariant gl_Position;`
                // FIXME: implement this
                ReportError("qualifier overwrite decl not supported yet");
                EnterRecoveryMode();
                return astBuilder.BuildErrorDecl(CreateAstSyntaxRange(beginTokID));
            }

            if (quals->GetQualGroup().CanDeclareInterfaceBlock()) {
                if (TryTestToken(TokenKlass::LBrace) ||
                    (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::LBrace, 1))) {
                    // interface block decl
                    // e.g. `uniform UBO { ... }`
                    return ParseInterfaceBlockDecl(beginTokID, quals);
                }
            }
        }

        // function/variable decl
        auto typeSpec = ParseTypeSpec(quals);
        if (InRecoveryMode()) {
            // Parser is currently in a unknown state.
            ReportError(beginTokID, "expect a qualified type but failed to parse one");
            return astBuilder.BuildErrorDecl(CreateAstSyntaxRange(beginTokID));
        }

        return ParseDeclarationWithTypeSpec(typeSpec);
    }

    auto Parser::ParseDeclarationWithTypeSpec(AstQualType* typeSpec) -> AstDecl*
    {
        GLSLD_TRACE_PARSER();
        GLSLD_ASSERT(typeSpec);

        auto beginTokID = typeSpec->GetSyntaxRange().GetBeginID();

        // FIXME: should we parse function decl in function body? This is bad in language server.
        if (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::LParen, 1)) {
            // function decl
            return ParseFunctionDecl(beginTokID, typeSpec);
        }
        else if (TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier)) {
            // type/variable decl
            return ParseTypeOrVariableDecl(beginTokID, typeSpec);
        }
        else {
            // This would always fail. We are just trying to infer the ';' for better diagnostic.
            ParseOrInferSemicolonHelper();

            // We just see this declaration as a variable decl without declarator.
            return astBuilder.BuildVariableDecl(CreateAstSyntaxRange(beginTokID), typeSpec, {});
        }
    }

    auto Parser::ParseInitializer(const Type* type) -> AstInitializer*
    {
        GLSLD_TRACE_PARSER();

        if (TryTestToken(TokenKlass::LBrace)) {
            return ParseInitializerList(type);
        }
        else {
            return ParseExprNoComma();
        }
    }

    auto Parser::ParseInitializerList(const Type* type) -> AstInitializerList*
    {
        GLSLD_TRACE_PARSER();

        struct InitializerListBraceDepthTracker
        {
            Parser* parser_ = nullptr;

            InitializerListBraceDepthTracker(Parser* parser)
            {
                // Only update when we are at the outermost call to ParseInitializerList
                if (!parser->parsingInitializerList) {
                    parser->parsingInitializerList = true;
                    parser->ilistBraceDepth        = parser->braceDepth;
                    parser_                        = parser;
                }
            }
            ~InitializerListBraceDepthTracker()
            {
                if (parser_) {
                    parser_->parsingInitializerList = false;
                }
            }
        };

        InitializerListBraceDepthTracker braceDepthTracker(this);

        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        auto beginTokID = GetCurrentTokenID();
        ConsumeToken();

        std::vector<AstInitializer*> initializers;
        for (size_t index = 0; !Eof(); ++index) {
            if (TryTestToken(TokenKlass::RBrace)) {
                break;
            }
            else if (TryTestToken(TokenKlass::LBrace)) {
                initializers.push_back(ParseInitializerList(type->GetComponentType(index)));

                if (!TryConsumeToken(TokenKlass::Comma)) {
                    break;
                }
            }
            else {
                initializers.push_back(ParseExprNoComma());

                if (!TryConsumeToken(TokenKlass::Comma)) {
                    break;
                }
            }
        }

        if (!TryConsumeToken(TokenKlass::RBrace)) {
            EnterRecoveryMode();
        }
        if (InRecoveryMode()) {
            RecoverFromError(RecoveryMode::IListBrace);

            if (TryConsumeToken(TokenKlass::RBrace)) {
                ExitRecoveryMode();
            }
        }

        return astBuilder.BuildInitializerList(CreateAstSyntaxRange(beginTokID), std::move(initializers), type);
    }

    auto Parser::ParseDeclarator(const Type* type) -> Declarator
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::Identifier));

        Declarator result;

        // Parse declaration identifier
        result.nameToken = ParseOptionalDeclIdHelper();

        // Parse array specifier
        auto declType = type;
        if (TryTestToken(TokenKlass::LBracket)) {
            result.arraySpec = ParseArraySpec();
            declType         = astBuilder.GetAstContext().GetArrayType(type, result.arraySpec);
        }

        if (TryConsumeToken(TokenKlass::Assign)) {
            result.initializer = ParseInitializer(declType);
        }

        return result;
    }

    auto Parser::ParseDeclaratorList(const Type* type) -> std::vector<Declarator>
    {
        GLSLD_TRACE_PARSER();

        std::vector<Declarator> result;
        while (TryTestToken(TokenKlass::Identifier)) {
            result.push_back(ParseDeclarator(type));

            if (!TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        return std::move(result);
    }

    auto Parser::ParseDeclaratorNoInit() -> Declarator
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::Identifier));

        Declarator result;

        // Parse declaration identifier
        result.nameToken = ParseOptionalDeclIdHelper();

        // Parse array specifier
        if (TryTestToken(TokenKlass::LBracket)) {
            result.arraySpec = ParseArraySpec();
        }

        return result;
    }

    auto Parser::ParseDeclaratorListNoInit() -> std::vector<Declarator>
    {
        GLSLD_TRACE_PARSER();

        std::vector<Declarator> result;
        while (TryTestToken(TokenKlass::Identifier)) {
            result.push_back(ParseDeclaratorNoInit());

            if (!TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        return std::move(result);
    }

    auto Parser::ParseFunctionParamList() -> std::vector<AstParamDecl*>
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::LParen));
        ParsingBalancedParenGuard parenGuard(*this);

        // Empty parameter list
        if (TryTestToken(TokenKlass::RParen)) {
            return {};
        }

        // TODO: Needed? spec doesn't include this grammar.
        if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
            ConsumeToken();
            return {};
        }

        // Parse parameters
        std::vector<AstParamDecl*> result;
        while (!Eof()) {
            auto beginTokID = GetCurrentTokenID();

            auto type = ParseQualType();
            if (!InRecoveryMode()) {
                std::optional<Declarator> declarator;
                if (TryTestToken(TokenKlass::Identifier)) {
                    declarator = ParseDeclaratorNoInit();
                }

                result.push_back(astBuilder.BuildParamDecl(CreateAstSyntaxRange(beginTokID), type, declarator));
            }

            if (!ParseCommaInParenHelper(parenGuard.GetLeftParenDepth())) {
                break;
            }
        }

        return std::move(result);
    }

    auto Parser::ParseFunctionDecl(SyntaxTokenID beginTokID, AstQualType* returnType) -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        // Parse function name
        auto declTok = ParseOptionalDeclIdHelper();

        astBuilder.EnterFunctionScope(returnType->GetResolvedType());

        // Parse function parameter list
        auto params = ParseFunctionParamList();

        // Parse function body
        AstStmt* body = nullptr;
        if (TryTestToken(TokenKlass::LBrace)) {
            // This is a definition
            GLSLD_ASSERT(InParsingMode());
            body = ParseCompoundStmt();
        }
        else {
            // This is a declaration

            // Parse trailing ';'
            // NOTE we will keep the ';' for recovery if parsing failed
            if (InParsingMode() && !TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expect ';' or function body");
            }
        }

        astBuilder.LeaveFunctionScope();
        return astBuilder.BuildFunctionDecl(CreateAstSyntaxRange(beginTokID), returnType, declTok, std::move(params),
                                            body);
    }

    auto Parser::ParseTypeOrVariableDecl(SyntaxTokenID beginTokID, AstQualType* variableType) -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier));

        // Parse type decl
        if (TryConsumeToken(TokenKlass::Semicolon)) {
            // TODO: should we just return a type decl?
            return astBuilder.BuildVariableDecl(CreateAstSyntaxRange(beginTokID), variableType, {});
        }

        // Parse variable decl
        auto declarators = ParseDeclaratorList(variableType->GetResolvedType());

        // Parse ';'
        ParseOrInferSemicolonHelper();

        return astBuilder.BuildVariableDecl(CreateAstSyntaxRange(beginTokID), variableType, std::move(declarators));
    }

    auto Parser::ParseBlockBody() -> std::vector<AstBlockFieldDecl*>
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        ConsumeToken();

        if (TryConsumeToken(TokenKlass::RBrace)) {
            // Empty block body
            return {};
        }

        std::vector<AstBlockFieldDecl*> result;
        while (!Eof()) {
            // Fast path. Parse an empty decl
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                // TODO: report warning
                continue;
            }

            // Parse a member decl
            auto beginTokID  = GetCurrentTokenID();
            auto typeResult  = ParseQualType();
            auto declarators = ParseDeclaratorListNoInit();

            if (!InRecoveryMode()) {
                result.push_back(astBuilder.BuildBlockFieldDecl(CreateAstSyntaxRange(beginTokID), typeResult,
                                                                std::move(declarators)));
                ParseOrInferSemicolonHelper();
            }
            else {
                // Try to resume if we see a ';'
                if (TryConsumeToken(TokenKlass::Semicolon)) {
                    ExitRecoveryMode();
                }
                else {
                    break;
                }
            }

            if (TryTestToken(TokenKlass::RBrace)) {
                break;
            }
        }

        if (!TryConsumeToken(TokenKlass::RBrace)) {
            RecoverFromError(RecoveryMode::Brace);
            if (TryConsumeToken(TokenKlass::RBrace) && InRecoveryMode()) {
                ExitRecoveryMode();
            }
        }

        return result;
    }

    auto Parser::ParseInterfaceBlockDecl(SyntaxTokenID beginTokID, AstTypeQualifierSeq* quals) -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        // Parse interface block name
        GLSLD_ASSERT(TryTestToken(TokenKlass::Identifier, TokenKlass::LBrace));
        auto declTok = ParseOptionalDeclIdHelper();

        // Parse interface block body
        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        auto blockBody = ParseBlockBody();
        if (InRecoveryMode() || TryConsumeToken(TokenKlass::Semicolon)) {
            return astBuilder.BuildInterfaceBlockDecl(CreateAstSyntaxRange(beginTokID), quals, declTok,
                                                      std::move(blockBody), std::nullopt);
        }

        // Parse declarator if any
        std::optional<Declarator> declarator;
        if (TryTestToken(TokenKlass::Identifier)) {
            declarator = ParseDeclaratorNoInit();
        }

        // Parse ';'
        ParseOrInferSemicolonHelper();

        return astBuilder.BuildInterfaceBlockDecl(CreateAstSyntaxRange(beginTokID), quals, declTok,
                                                  std::move(blockBody), declarator);
    }

    auto Parser::ParsePrecisionDecl() -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::K_precision));
        auto beginTokID = GetCurrentTokenID();
        ConsumeToken();

        // Parse precision qualifier
        QualifierGroup qualifiers;
        auto precisionTokIndex = GetCurrentTokenID();
        if (TryConsumeToken(TokenKlass::K_highp)) {
            qualifiers.qHighp = true;
        }
        else if (TryConsumeToken(TokenKlass::K_mediump)) {
            qualifiers.qMediump = true;
        }
        else if (TryConsumeToken(TokenKlass::K_lowp)) {
            qualifiers.qLowp = true;
        }
        else {
            ReportError("expecting precision qualifier");
            EnterRecoveryMode();
        }
        auto qualifierSeq = astBuilder.BuildTypeQualifierSeq(CreateAstSyntaxRange(precisionTokIndex), qualifiers, {});

        // Parse type spec
        // FIXME: struct shouldn't be parsed here
        auto typeSpec = ParseTypeSpec(qualifierSeq);

        // Parse ';'
        ParseOrInferSemicolonHelper();

        return astBuilder.BuildPrecisionDecl(CreateAstSyntaxRange(beginTokID), typeSpec);
    }

#pragma endregion

#pragma region Parsing Expr

    auto Parser::ParseExpr() -> AstExpr*
    {
        auto beginTokID = GetCurrentTokenID();
        auto expr       = ParseCommaExpr();

        if (GetCurrentTokenID() == beginTokID) {
            EnterRecoveryMode();
        }

        return expr;
    }

    auto Parser::ParseExprNoComma() -> AstExpr*
    {
        auto beginTokID = GetCurrentTokenID();
        auto expr       = ParseAssignmentExpr();

        if (GetCurrentTokenID() == beginTokID) {
            EnterRecoveryMode();
        }

        return expr;
    }

    auto Parser::ParseCommaExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();
        auto lhsResult  = ParseAssignmentExpr();

        while (TryConsumeToken(TokenKlass::Comma)) {
            auto rhsResult = ParseAssignmentExpr();

            lhsResult =
                astBuilder.BuildBinaryExpr(CreateAstSyntaxRange(beginTokID), lhsResult, rhsResult, BinaryOp::Comma);
        }

        return lhsResult;
    }

    static auto TryParseAssignmentOp(TokenKlass klass) -> std::optional<BinaryOp>
    {
        switch (klass) {
        case TokenKlass::Assign:
            return BinaryOp::Assign;
        case TokenKlass::MulAssign:
            return BinaryOp::MulAssign;
        case TokenKlass::DivAssign:
            return BinaryOp::DivAssign;
        case TokenKlass::ModAssign:
            return BinaryOp::ModAssign;
        case TokenKlass::AddAssign:
            return BinaryOp::AddAssign;
        case TokenKlass::SubAssign:
            return BinaryOp::SubAssign;
        case TokenKlass::LShiftAssign:
            return BinaryOp::LShiftAssign;
        case TokenKlass::RShiftAssign:
            return BinaryOp::RShiftAssign;
        case TokenKlass::AndAssign:
            return BinaryOp::AndAssign;
        case TokenKlass::XorAssign:
            return BinaryOp::XorAssign;
        case TokenKlass::OrAssign:
            return BinaryOp::OrAssign;
        default:
            return std::nullopt;
        }
    }

    auto Parser::ParseAssignmentExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();
        auto lhsResult  = ParseUnaryExpr();

        return ParseAssignmentExprWithLhs(lhsResult);
    }

    auto Parser::ParseAssignmentExprWithLhs(AstExpr* lhs) -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = lhs->GetSyntaxRange().GetBeginID();

        if (auto parsedOp = TryParseAssignmentOp(PeekToken().klass); parsedOp) {
            ConsumeToken();
            GLSLD_ASSERT(InParsingMode());
            auto rhs = ParseAssignmentExpr();

            return astBuilder.BuildBinaryExpr(CreateAstSyntaxRange(beginTokID), lhs, rhs, *parsedOp);
        }
        else {
            return ParseConditionalExpr(beginTokID, lhs);
        }
    }

    auto Parser::ParseConditionalExpr(SyntaxTokenID beginTokID, AstExpr* firstTerm) -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto predicateOrExprResult = ParseBinaryExpr(beginTokID, firstTerm, 0);

        if (!TryConsumeToken(TokenKlass::Question)) {
            return predicateOrExprResult;
        }

        auto trueExpr = ParseCommaExpr();

        if (!TryConsumeToken(TokenKlass::Colon)) {
            if (InRecoveryMode()) {
                // we cannot infer a ':' in recovery mode, so just return early
                return astBuilder.BuildSelectExpr(CreateAstSyntaxRange(beginTokID), predicateOrExprResult, trueExpr,
                                                  CreateErrorExpr());
            }

            ReportError("expecting ':'");
        }

        // Even if ':' is missing, we'll continue parsing the nenative part
        auto falseExpr = ParseAssignmentExpr();
        return astBuilder.BuildSelectExpr(CreateAstSyntaxRange(beginTokID), predicateOrExprResult, trueExpr, falseExpr);
    }

    // These are all left-associative operators
    struct BinaryOpDesc
    {
        BinaryOp opcode;
        int precedence;
    };
    static auto TryParseBinaryOp(TokenKlass tok) -> std::optional<BinaryOpDesc>
    {
        switch (tok) {
        case TokenKlass::Or:
            return BinaryOpDesc{BinaryOp::LogicalOr, 0};
        case TokenKlass::Xor:
            return BinaryOpDesc{BinaryOp::LogicalXor, 1};
        case TokenKlass::And:
            return BinaryOpDesc{BinaryOp::LogicalAnd, 2};
        case TokenKlass::VerticalBar:
            return BinaryOpDesc{BinaryOp::BitwiseOr, 3};
        case TokenKlass::Caret:
            return BinaryOpDesc{BinaryOp::BitwiseXor, 4};
        case TokenKlass::Ampersand:
            return BinaryOpDesc{BinaryOp::BitwiseAnd, 5};
        case TokenKlass::Equal:
            return BinaryOpDesc{BinaryOp::Equal, 6};
        case TokenKlass::NotEqual:
            return BinaryOpDesc{BinaryOp::NotEqual, 6};
        case TokenKlass::LAngle:
            return BinaryOpDesc{BinaryOp::Less, 7};
        case TokenKlass::RAngle:
            return BinaryOpDesc{BinaryOp::Greater, 7};
        case TokenKlass::LessEq:
            return BinaryOpDesc{BinaryOp::LessEq, 7};
        case TokenKlass::GreaterEq:
            return BinaryOpDesc{BinaryOp::GreaterEq, 7};
        case TokenKlass::LShift:
            return BinaryOpDesc{BinaryOp::ShiftLeft, 8};
        case TokenKlass::RShift:
            return BinaryOpDesc{BinaryOp::ShiftRight, 8};
        case TokenKlass::Plus:
            return BinaryOpDesc{BinaryOp::Plus, 9};
        case TokenKlass::Dash:
            return BinaryOpDesc{BinaryOp::Minus, 9};
        case TokenKlass::Star:
            return BinaryOpDesc{BinaryOp::Mul, 10};
        case TokenKlass::Slash:
            return BinaryOpDesc{BinaryOp::Div, 10};
        case TokenKlass::Percent:
            return BinaryOpDesc{BinaryOp::Modulo, 10};
        default:
            return std::nullopt;
        }
    };

    auto Parser::ParseBinaryExpr(SyntaxTokenID beginTokID, AstExpr* firstTerm, int minPrecedence) -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto lhs = firstTerm;
        while (!Eof()) {
            if (InRecoveryMode()) {
                // NOTE `lhs` is either unary_expr or binary expr, meaning we have incoming 'EOF' or ';' or '}'.
                // Obviously, we cannot parse anything so just leave early
                break;
            }

            auto parsedOp = TryParseBinaryOp(PeekToken().klass);
            if (!(parsedOp.has_value() && parsedOp->precedence >= minPrecedence)) {
                break;
            }

            // Consume the operator
            ConsumeToken();

            auto rhsBeginTokIndex = GetCurrentTokenID();
            auto rhs              = ParseUnaryExpr();

            while (!Eof()) {
                auto parsedOpNext = TryParseBinaryOp(PeekToken().klass);
                if (!(parsedOpNext.has_value() && parsedOpNext->precedence > parsedOp->precedence)) {
                    break;
                }

                rhs = ParseBinaryExpr(rhsBeginTokIndex, rhs, parsedOp->precedence + 1);
            }

            lhs = astBuilder.BuildBinaryExpr(CreateAstSyntaxRange(beginTokID), lhs, rhs, parsedOp->opcode);

            // Since all binary operators in GLSL are left-associative, the `beginTokID` should stay the same
        }

        return lhs;
    }

    static auto TryParsePrefixUnaryOp(TokenKlass klass) -> std::optional<UnaryOp>
    {
        switch (klass) {
        case TokenKlass::Plus:
            return UnaryOp::Identity;
        case TokenKlass::Dash:
            return UnaryOp::Negate;
        case TokenKlass::Tilde:
            return UnaryOp::BitwiseNot;
        case TokenKlass::Bang:
            return UnaryOp::LogicalNot;
        case TokenKlass::Increment:
            return UnaryOp::PrefixInc;
        case TokenKlass::Decrement:
            return UnaryOp::PrefixDec;
        default:
            return std::nullopt;
        }
    }

    auto Parser::ParseUnaryExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto parsedOp = TryParsePrefixUnaryOp(PeekToken().klass);
        if (parsedOp) {
            auto beginTokID = GetCurrentTokenID();
            ConsumeToken();
            // TODO: avoid recursion
            auto operandExpr = ParseUnaryExpr();
            return astBuilder.BuildUnaryExpr(CreateAstSyntaxRange(beginTokID), operandExpr, *parsedOp);
        }
        else {
            return ParsePostfixExpr(nullptr);
        }
    }

    auto Parser::ParsePostfixExpr(AstExpr* parsedExpr) -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = parsedExpr ? parsedExpr->GetSyntaxRange().GetBeginID() : GetCurrentTokenID();

        // Parse primary_expr or constructor call
        AstExpr* result = parsedExpr;
        if (parsedExpr == nullptr) {
            if (GetGlslBuiltinType(PeekToken().klass) || PeekToken().klass == TokenKlass::K_struct) {
                // Obviously this is a constructor call
                auto typeSpec = ParseTypeSpec(nullptr);
                result        = ParseConstructorCallExpr(typeSpec);
            }
            else {
                if (TryTestToken(TokenKlass::Identifier)) {
                    // Could still be constructor call if it's a unknown type name
                    // Though we are conservative here and need to peek a following '(' to confirm.
                    // This means we'll parse a isolated type identifier as a bad name access.
                    if (astBuilder.IsStructName(PeekToken().text.StrView())) {
                        auto typeSpec = ParseTypeSpec(nullptr);
                        result        = ParseConstructorCallExpr(typeSpec);
                    }
                    else if (TryTestToken(TokenKlass::LParen, 1)) {
                        // Note there's no function pointer in GLSL.
                        auto functionName = GetCurrentToken();
                        ConsumeToken();
                        auto args = ParseFunctionArgumentList();
                        result    = astBuilder.BuildFuntionCallExpr(CreateAstSyntaxRange(beginTokID), functionName,
                                                                    std::move(args));
                    }
                }

                if (result == nullptr) {
                    // Parse a primary expression if the logic above failed to parse a call
                    result = ParsePrimaryExpr();
                }
            }
        }

        bool parsedPostfixOp = true;
        while (InParsingMode() && parsedPostfixOp) {
            switch (PeekToken().klass) {
            case TokenKlass::LBracket:
            {
                // Indexing access
                auto indexExpr = ParseBracketWrappedExpr(true);
                result         = astBuilder.BuildIndexAccessExpr(CreateAstSyntaxRange(beginTokID), result, indexExpr);
                break;
            }
            case TokenKlass::Dot:
            {
                // Member access or .length() call
                ConsumeToken();
                auto accessName = ParseOptionalDeclIdHelper();
                if (accessName.text.Equals("length") && TryTestToken(TokenKlass::LParen) &&
                    TryTestToken(TokenKlass::RParen, 1)) {
                    ConsumeToken();
                    ConsumeToken();
                    result = astBuilder.BuildUnaryExpr(CreateAstSyntaxRange(beginTokID), result, UnaryOp::Length);
                }
                else {
                    result = astBuilder.BuildDotAccessExpr(CreateAstSyntaxRange(beginTokID), result, accessName);
                }
                break;
            }
            case TokenKlass::Increment:
            {
                // Postfix inc
                ConsumeToken();
                result = astBuilder.BuildUnaryExpr(CreateAstSyntaxRange(beginTokID), result, UnaryOp::PostfixInc);
                break;
            }
            case TokenKlass::Decrement:
            {
                // Postfix dnc
                ConsumeToken();
                result = astBuilder.BuildUnaryExpr(CreateAstSyntaxRange(beginTokID), result, UnaryOp::PostfixDec);
                break;
            }
            default:
                parsedPostfixOp = false;
                break;
            }
        }

        return result;
    }

    auto Parser::ParseConstructorCallExpr(AstQualType* typeSpec) -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = typeSpec->GetSyntaxRange().GetBeginID();

        std::vector<AstExpr*> args;
        if (TryTestToken(TokenKlass::LParen)) {
            args = ParseFunctionArgumentList();
        }
        else {
            ReportError("expect '(' after constructor name");
        }
        return astBuilder.BuildConstructorCallExpr(CreateAstSyntaxRange(beginTokID), typeSpec, std::move(args));
    }

    auto Parser::ParsePrimaryExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        const auto& tok = PeekToken();
        switch (PeekToken().klass) {
        case TokenKlass::Identifier:
        {
            // Variable name
            auto name = GetCurrentToken();
            ConsumeToken();
            return astBuilder.BuildNameAccessExpr(CreateAstSyntaxRange(beginTokID), name);
        }
        case TokenKlass::IntegerConstant:
        case TokenKlass::FloatConstant:
            // Number literal
            ConsumeToken();
            return astBuilder.BuildLiteralExpr(CreateAstSyntaxRange(beginTokID),
                                               ParseNumberLiteral(tok.text.StrView()));
        case TokenKlass::K_true:
        case TokenKlass::K_false:
            // Boolean literal
            ConsumeToken();
            return astBuilder.BuildLiteralExpr(CreateAstSyntaxRange(beginTokID),
                                               ConstValue::CreateScalar(tok.klass == TokenKlass::K_true));
        case TokenKlass::LParen:
            // expr in wrapped parens
            return ParseParenWrappedExpr();
        default:
            // error, but we see this as a successful parse
            ReportError("expect an expression");
            return CreateErrorExpr();
        }
    }

    auto Parser::ParseParenWrappedExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        ParsingBalancedParenGuard parenGuard(*this);

        // A special case of empty parenthesis "()"
        if (TryTestToken(TokenKlass::RParen)) {
            return CreateErrorExpr();
        }

        return ParseExpr();
    };

    auto Parser::ParseBracketWrappedExpr(bool emptyIsError) -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        ParsingBalancedBracketGuard bracketGuard(*this);

        // A special case of empty brackets "[]"
        if (TryTestToken(TokenKlass::RBracket)) {
            return emptyIsError ? CreateErrorExpr() : nullptr;
        }

        return ParseExpr();
    }

    auto Parser::ParseFunctionArgumentList() -> std::vector<AstExpr*>
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::LParen));
        ParsingBalancedParenGuard parenGuard(*this);

        if (TryTestToken(TokenKlass::RParen)) {
            return {};
        }

        if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
            ConsumeToken();
            return {};
        }

        std::vector<AstExpr*> result;
        while (!Eof()) {
            result.push_back(ParseAssignmentExpr());

            if (!ParseCommaInParenHelper(parenGuard.GetLeftParenDepth())) {
                break;
            }
        }

        return result;
    }

#pragma endregion

#pragma region Parsing Stmt

    auto Parser::ParseStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();
        switch (PeekToken().klass) {
        case glsld::TokenKlass::Semicolon:
            ConsumeToken();
            return astBuilder.BuildEmptyStmt(beginTokID);
        case TokenKlass::LBrace:
        {
            // compound stmt
            astBuilder.EnterLexicalBlockScope();
            auto stmt = ParseCompoundStmt();
            astBuilder.LeaveLexicalBlockScope();
            return stmt;
        }
        case TokenKlass::K_if:
            // selection stmt
            return ParseSelectionStmt();
        case TokenKlass::K_do:
            // iteration stmt (do-while)
            return ParseDoWhileStmt();
        case TokenKlass::K_while:
            // iteration stmt (while)
            return ParseWhileStmt();
        case TokenKlass::K_for:
            // iteration stmt (for)
            return ParseForStmt();
        case TokenKlass::K_switch:
            // switch stmt
            return ParseSwitchStmt();
        case TokenKlass::K_break:
        case TokenKlass::K_continue:
        case TokenKlass::K_discard:
        case TokenKlass::K_return:
            // jump stmt
            return ParseJumpStmt();
        case TokenKlass::K_case:
        case TokenKlass::K_default:
            // label stmt
            return ParseLabelStmt();
        default:
            // expression/declaration stmt
            return ParseDeclOrExprStmt();
        }
    }

    auto Parser::ParseCompoundStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        // Parse '{'
        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        ConsumeToken();

        // Parse children statements
        std::vector<AstStmt*> children;
        while (!Eof() && !TryTestToken(TokenKlass::RBrace)) {
            children.push_back(ParseStmtAndTryRecover());
        }

        // Parse '}'
        if (TryConsumeToken(TokenKlass::RBrace)) {
            if (InRecoveryMode()) {
                ExitRecoveryMode();
            }
        }
        else {
            GLSLD_ASSERT(Eof());
            ReportError("Unexpected EOF");
        }

        return astBuilder.BuildCompoundStmt(CreateAstSyntaxRange(beginTokID), std::move(children));
    }

    auto Parser::ParseSelectionStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        // Parse 'K_if'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_if));
        ConsumeToken();

        // Parse predicate expression
        AstExpr* predicateExpr = ParseParenWrappedExprOrErrorHelper();

        if (InRecoveryMode()) {
            return astBuilder.BuildIfStmt(CreateAstSyntaxRange(beginTokID), predicateExpr, CreateErrorStmt());
        }

        // Parse if branch
        auto ifBranchStmt = ParseStmt();

        // Try parse 'K_else'
        // TODO: error handling
        if (!TryConsumeToken(TokenKlass::K_else)) {
            return astBuilder.BuildIfStmt(CreateAstSyntaxRange(beginTokID), predicateExpr, ifBranchStmt);
        }

        // TODO: check with if?
        auto elseBranchStmt = ParseStmt();
        // TODO: error handling
        return astBuilder.BuildIfStmt(CreateAstSyntaxRange(beginTokID), predicateExpr, ifBranchStmt, elseBranchStmt);
    }

    auto Parser::ParseForStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        // Parse 'K_for'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_for));
        ConsumeToken();

        if (!TryTestToken(TokenKlass::LParen)) {
            EnterRecoveryMode();
            return astBuilder.BuildForStmt(CreateAstSyntaxRange(beginTokID), CreateErrorStmt(), CreateErrorExpr(),
                                           CreateErrorExpr(), CreateErrorStmt());
        }

        astBuilder.EnterLexicalBlockScope();

        // Parse for loop header
        AstStmt* initClause    = nullptr;
        AstExpr* conditionExpr = nullptr;
        AstExpr* iterExpr      = nullptr;
        do {
            ParsingBalancedParenGuard parenGuard(*this);

            // Parse init clause
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                initClause = astBuilder.BuildEmptyStmt(GetCurrentTokenID());
            }
            else {
                initClause = ParseDeclOrExprStmt();
            }

            if (InRecoveryMode()) {
                conditionExpr = CreateErrorExpr();
                iterExpr      = CreateErrorExpr();
                break;
            }

            // Parse test clause
            if (!TryConsumeToken(TokenKlass::Semicolon)) {
                conditionExpr = ParseExpr();

                // TODO: error handling
                TryConsumeToken(TokenKlass::Semicolon);
            }

            if (InRecoveryMode()) {
                iterExpr = CreateErrorExpr();
                break;
            }

            // Parse proceed clause
            if (!TryTestToken(TokenKlass::RParen)) {
                iterExpr = ParseExpr();
            }
        } while (false);
        GLSLD_ASSERT(initClause);

        // Parse loop body
        // FIXME: is the recovery correct?
        AstStmt* loopBody = nullptr;
        if (InRecoveryMode()) {
            // Meaning we cannot parse the closing ')'. We cannot assume the following tokens form the loop body.
            loopBody = CreateErrorStmt();
        }
        else {
            loopBody = ParseStmt();
        }

        astBuilder.LeaveLexicalBlockScope();
        return astBuilder.BuildForStmt(CreateAstSyntaxRange(beginTokID), initClause, conditionExpr, iterExpr, loopBody);
    }

    auto Parser::ParseDoWhileStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        // Parse 'K_do'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_do));
        ConsumeToken();

        // Parse loop body
        auto bodyStmt = ParseStmt();

        // Parse 'K_while'
        if (!TryConsumeToken(TokenKlass::K_while)) {
            EnterRecoveryMode();
            return astBuilder.BuildDoWhileStmt(CreateAstSyntaxRange(beginTokID), CreateErrorExpr(), bodyStmt);
        }

        // Parse condition expr
        AstExpr* conditionExpr = ParseParenWrappedExprOrErrorHelper();

        // Parse ';'
        ParseOrInferSemicolonHelper();

        return astBuilder.BuildDoWhileStmt(CreateAstSyntaxRange(beginTokID), conditionExpr, bodyStmt);
    }

    auto Parser::ParseWhileStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        // Parse 'K_while'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_while));
        ConsumeToken();

        if (!TryTestToken(TokenKlass::LParen)) {
            EnterRecoveryMode();
            return astBuilder.BuildWhileStmt(CreateAstSyntaxRange(beginTokID), CreateErrorExpr(), CreateErrorStmt());
        }

        // Parse while loop header
        AstExpr* conditionExpr = ParseParenWrappedExpr();

        if (InRecoveryMode()) {
            return astBuilder.BuildWhileStmt(CreateAstSyntaxRange(beginTokID), conditionExpr, CreateErrorStmt());
        }

        // Parse loop body
        auto bodyStmt = ParseStmt();
        return astBuilder.BuildWhileStmt(CreateAstSyntaxRange(beginTokID), conditionExpr, bodyStmt);
    }

    auto Parser::ParseLabelStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();
        auto beginTokID = GetCurrentTokenID();

        AstExpr* caseExpr = nullptr;
        if (TryConsumeToken(TokenKlass::K_case)) {
            caseExpr = ParseExprNoComma();
        }
        else {
            GLSLD_ASSERT(TryTestToken(TokenKlass::K_default));
            ConsumeToken();
        }

        if (TryConsumeToken(TokenKlass::Colon)) {
            if (InRecoveryMode()) {
                ExitRecoveryMode();
            }
        }
        else {
            ReportError("expecting ':'");
        }

        return astBuilder.BuildLabelStmt(CreateAstSyntaxRange(beginTokID), caseExpr);
    }

    auto Parser::ParseSwitchStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        // Parse 'K_switch'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_switch));
        ConsumeToken();

        if (!TryTestToken(TokenKlass::LParen)) {
            EnterRecoveryMode();
            return astBuilder.BuildSwitchStmt(CreateAstSyntaxRange(beginTokID), CreateErrorExpr(), CreateErrorStmt());
        }

        // Parse switched expr
        AstExpr* testExpr = ParseParenWrappedExpr();

        // Parse switch body
        AstStmt* switchBody = nullptr;
        if (TryTestToken(TokenKlass::LBrace)) {
            astBuilder.EnterLexicalBlockScope();
            switchBody = ParseCompoundStmt();
            astBuilder.LeaveLexicalBlockScope();
        }
        else {
            EnterRecoveryMode();
            switchBody = CreateErrorStmt();
        }

        return astBuilder.BuildSwitchStmt(CreateAstSyntaxRange(beginTokID), testExpr, switchBody);
    }

    auto Parser::ParseJumpStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();
        switch (PeekToken().klass) {
        case TokenKlass::K_break:
            ConsumeToken();
            ParseOrInferSemicolonHelper();
            return astBuilder.BuildJumpStmt(CreateAstSyntaxRange(beginTokID), JumpType::Break);
        case TokenKlass::K_continue:
            ConsumeToken();
            ParseOrInferSemicolonHelper();
            return astBuilder.BuildJumpStmt(CreateAstSyntaxRange(beginTokID), JumpType::Continue);
        case TokenKlass::K_discard:
            ConsumeToken();
            ParseOrInferSemicolonHelper();
            return astBuilder.BuildJumpStmt(CreateAstSyntaxRange(beginTokID), JumpType::Discard);
        case TokenKlass::K_return:
            ConsumeToken();
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                return astBuilder.BuildReturnStmt(CreateAstSyntaxRange(beginTokID), nullptr);
            }
            else {
                auto returnedExpr = ParseExprNoComma();
                ParseOrInferSemicolonHelper();

                return astBuilder.BuildReturnStmt(CreateAstSyntaxRange(beginTokID), returnedExpr);
            }
        default:
            GLSLD_ASSERT(false && "Unexpected token");
            GLSLD_UNREACHABLE();
        }
    }

    auto Parser::ParseExprStmt(AstQualType* typeSpec) -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokID = GetCurrentTokenID();

        // Parse expr
        AstExpr* expr;
        if (typeSpec) {
            // We know for sure this starts with a constructor call
            GLSLD_ASSERT(TryTestToken(TokenKlass::LParen));
            expr = ParseAssignmentExprWithLhs(ParsePostfixExpr(ParseConstructorCallExpr(typeSpec)));
        }
        else {
            expr = ParseExpr();
        }

        // Parse ';'
        ParseOrInferSemicolonHelper();

        return astBuilder.BuildExprStmt(CreateAstSyntaxRange(beginTokID), expr);
    }

    auto Parser::ParseDeclStmt(AstQualType* typeSpec) -> AstStmt*
    {
        auto decl = ParseDeclAndTryRecover(typeSpec, false);
        return astBuilder.BuildDeclStmt(decl->GetSyntaxRange(), decl);
    }

    auto Parser::ParseDeclOrExprStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        // FIXME: infer expression statement more aggressively
        auto beginTokID = GetCurrentTokenID();
        switch (PeekToken().klass) {
            // It's definitely an expression statement if we see literals, '(' or operators
            //

            // Primary expr
        case TokenKlass::IntegerConstant:
        case TokenKlass::FloatConstant:
        case TokenKlass::K_true:
        case TokenKlass::K_false:
        case TokenKlass::LParen:
            // Unary expr
        case TokenKlass::Plus:
        case TokenKlass::Dash:
        case TokenKlass::Bang:
        case TokenKlass::Tilde:
        case TokenKlass::Increment:
        case TokenKlass::Decrement:
            // Assignment expr
        case TokenKlass::Assign:
        case TokenKlass::MulAssign:
        case TokenKlass::DivAssign:
        case TokenKlass::AddAssign:
        case TokenKlass::ModAssign:
        case TokenKlass::LShiftAssign:
        case TokenKlass::RShiftAssign:
        case TokenKlass::AndAssign:
        case TokenKlass::XorAssign:
        case TokenKlass::OrAssign:
        case TokenKlass::SubAssign:
            // Binary expr
        case TokenKlass::Or:
        case TokenKlass::Xor:
        case TokenKlass::And:
        case TokenKlass::VerticalBar:
        case TokenKlass::Caret:
        case TokenKlass::Ampersand:
        case TokenKlass::Equal:
        case TokenKlass::NotEqual:
        case TokenKlass::LAngle:
        case TokenKlass::RAngle:
        case TokenKlass::LessEq:
        case TokenKlass::GreaterEq:
        case TokenKlass::LShift:
        case TokenKlass::RShift:
        // case TokenKlass::Plus:
        // case TokenKlass::Dash:
        case TokenKlass::Star:
        case TokenKlass::Slash:
        case TokenKlass::Percent:
            // Comma expr
        case TokenKlass::Comma:
            return ParseExprStmt(nullptr);

            // It's definitely a declaration statement if we see qualifiers
            //

            // Storage qualifier
        case TokenKlass::K_const:
        case TokenKlass::K_in:
        case TokenKlass::K_out:
        case TokenKlass::K_inout:
        case TokenKlass::K_centroid:
        case TokenKlass::K_patch:
        case TokenKlass::K_sample:
        case TokenKlass::K_uniform:
        case TokenKlass::K_buffer:
        case TokenKlass::K_shared:
        case TokenKlass::K_coherent:
        case TokenKlass::K_volatile:
        case TokenKlass::K_restrict:
        case TokenKlass::K_readonly:
        case TokenKlass::K_writeonly:
        case TokenKlass::K_subroutine:
            // Layout qualifier
        case TokenKlass::K_layout:
            // Precision qualifier
        case TokenKlass::K_lowp:
        case TokenKlass::K_mediump:
        case TokenKlass::K_highp:
            // Interpolation qualifier
        case TokenKlass::K_smooth:
        case TokenKlass::K_flat:
        case TokenKlass::K_noperspective:
            // Invariant qualifier
        case TokenKlass::K_invariant:
            // Precise qualifier
        case TokenKlass::K_precise:
            return ParseDeclStmt(nullptr);

        default:
            if (TryTestToken(TokenKlass::K_struct) || TryTestToken(TokenKlass::Identifier) ||
                GetGlslBuiltinType(PeekToken().klass)) {
                if (TryTestToken(TokenKlass::Identifier) && !astBuilder.IsStructName(PeekToken().text.StrView())) {
                    // We see a regular identifier, meaning it's most likely an expression.
                    // But we'll try more heuristics to infer if it's a declaration.
                    if (TryTestToken(TokenKlass::Identifier, 1) &&
                        TryTestToken(TokenKlass::Semicolon, TokenKlass::LBracket, TokenKlass::Assign, 2)) {
                        // `T a;` or `T a[...` or `T a = ...`
                        return ParseDeclStmt(nullptr);
                    }
                    else {
                        return ParseExprStmt(nullptr);
                    }
                }
                else {
                    // We see a type specifier. We parse it first as we cannot determine if it's a declaration or
                    // expression until we see the next token after the type specifier.
                    auto typeSpec = ParseTypeSpec(nullptr);
                    if (TryTestToken(TokenKlass::LParen)) {
                        // It's a constructor call expression
                        return ParseExprStmt(typeSpec);
                    }
                    else {
                        // It's a declaration statement
                        return ParseDeclStmt(typeSpec);
                    }
                }
            }
            else {
                // We have no clue what the hack this is, so we infer an expression for now
                return ParseExprStmt(nullptr);
            }
        }
    }

#pragma endregion

    // FIXME: really need to carefully review this function
    auto Parser::RecoverFromError(RecoveryMode mode) -> void
    {
        GLSLD_TRACE_PARSER();

        size_t initParenDepth   = parenDepth;
        size_t initBracketDepth = bracketDepth;
        size_t initBraceDepth   = braceDepth;

        if (mode == RecoveryMode::Comma || mode == RecoveryMode::Paren) {
            GLSLD_ASSERT(initParenDepth > 0);
        }
        else if (mode == RecoveryMode::Bracket) {
            GLSLD_ASSERT(initBracketDepth > 0);
        }
        else if (mode == RecoveryMode::Brace) {
            GLSLD_ASSERT(initBraceDepth > 0);
        }

        // We may close a pair of parenthsis without the closing delimitor
        auto removeDepthIfUnclosed = [&]() {
            switch (mode) {
            case RecoveryMode::Paren:
                if (!TryTestToken(TokenKlass::RParen)) {
                    GLSLD_ASSERT(initParenDepth != 0);
                    parenDepth -= 1;
                }
                break;
            case RecoveryMode::Bracket:
                if (!TryTestToken(TokenKlass::RBracket)) {
                    GLSLD_ASSERT(initBracketDepth != 0);
                    bracketDepth -= 1;
                }
                break;
            case RecoveryMode::Brace:
            case RecoveryMode::IListBrace:
                if (!TryTestToken(TokenKlass::RBrace)) {
                    GLSLD_ASSERT(initBraceDepth != 0);
                    braceDepth -= 1;
                }
                break;
            default:
                break;
            }
        };

        while (!Eof()) {
            switch (PeekToken().klass) {
            case TokenKlass::RParen:
                if (parenDepth == 0) {
                    // skip an isolated ')' without leading '('
                    break;
                }
                if ((mode == RecoveryMode::Comma || mode == RecoveryMode::Paren) && parenDepth == initParenDepth) {
                    return;
                }
                break;
            case TokenKlass::RBracket:
                if (bracketDepth == 0) {
                    // skip an isolated ']' without leading '['
                    break;
                }
                if (mode == RecoveryMode::Bracket && bracketDepth == initBracketDepth) {
                    return;
                }
                break;
            case TokenKlass::RBrace:
                if (braceDepth == 0) {
                    // skip an isolated '}' without leading '{'
                    break;
                }
                if (braceDepth == initBraceDepth) {
                    return removeDepthIfUnclosed();
                }
                break;
            case TokenKlass::Comma:
                if (mode == RecoveryMode::Comma && parenDepth == initParenDepth && bracketDepth == initBracketDepth &&
                    braceDepth == initBraceDepth) {
                    return;
                }
                break;
            case TokenKlass::Semicolon:
                if ((mode == RecoveryMode::Comma || mode == RecoveryMode::Paren || mode == RecoveryMode::Bracket ||
                     mode == RecoveryMode::Semi) &&
                    braceDepth == initBraceDepth) {
                    return removeDepthIfUnclosed();
                }
                else if (mode == RecoveryMode::IListBrace) {
                    // Force brace depth to rebalance to where the initializer list started
                    braceDepth = ilistBraceDepth;
                    return;
                }
                break;
            default:
                break;
            }

            // skip token?
            ConsumeToken();
        }
    }

    auto Parser::ConsumeToken() -> void
    {
        if (!Eof()) {
#if defined(GLSLD_ENABLE_COMPILER_TRACE)
            compiler.GetCompilerTrace().TraceTokenConsumed(PeekToken());
#endif

            switch (PeekToken().klass) {
            case TokenKlass::LParen:
                parenDepth += 1;
                break;
            case TokenKlass::LBracket:
                bracketDepth += 1;
                break;
            case TokenKlass::LBrace:
                braceDepth += 1;
                break;
            case TokenKlass::RParen:
                if (parenDepth > 0) {
                    parenDepth -= 1;
                }
                break;
            case TokenKlass::RBracket:
                if (bracketDepth > 0) {
                    bracketDepth -= 1;
                }
                break;
            case TokenKlass::RBrace:
                if (braceDepth > 0) {
                    braceDepth -= 1;
                }
                break;
            default:
                break;
            }

            ++currentTok;
        }
    }

} // namespace glsld
#include "Parser.h"
#include "Compiler.h"

namespace glsld
{
    auto Parser::DoParse() -> void
    {
        GLSLD_TRACE_PARSER();

        RestoreTokenIndex(0);

        while (!Eof()) {
            compilerObject.GetAstContext().AddGlobalDecl(ParseDeclAndTryRecover());
        }
    }

#pragma region Parsing Misc

    auto Parser::ParsePermissiveSemicolonHelper() -> void
    {
        if (InParsingMode()) {
            if (!TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expecting ';'");
                // However, we don't do error recovery as if the ';' is inferred by the parser.
            }
        }
    }

    auto Parser::ParseClosingParenHelper() -> void
    {
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

    auto Parser::ParseClosingBracketHelper() -> void
    {
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

    auto Parser::ParseDeclIdHelper() -> SyntaxToken
    {
        if (PeekToken().IsIdentifier()) {
            auto result = PeekToken();
            ConsumeToken();
            return result;
        }
        else {
            ReportError("Expect identifier");
            return SyntaxToken{};
        }
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

#pragma endregion

#pragma region Parsing QualType
    auto Parser::ParseLayoutQualifier(std::vector<LayoutItem>& items) -> void
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::K_layout));

        auto beginTokIndex = GetTokenIndex();
        ConsumeToken();

        if (!TryConsumeToken(TokenKlass::LParen)) {
            EnterRecoveryMode();
            return;
        }

        while (!Eof()) {
            auto idToken = ParseDeclIdHelper();

            AstExpr* value = nullptr;
            if (TryConsumeToken(TokenKlass::Assign)) {
                value = ParseExprNoComma();
            }

            items.push_back(LayoutItem{idToken, value});

            if (!TryConsumeToken(TokenKlass::Comma)) {
                break;
            }

            // We'll continue parsing regardless if a ',' has been consumed
            if (InRecoveryMode()) {
                ExitRecoveryMode();
            }
        }

        ParseClosingParenHelper();
    }

    auto Parser::ParseTypeQualifiers() -> AstTypeQualifierSeq*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        QualifierGroup qualifiers;
        std::vector<LayoutItem> layoutQuals;

        bool inQualSeq = true;
        while (InParsingMode() && inQualSeq) {
            // FIXME: implement correctly
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

        // TODO: return nullptr if no qualifier is parsed
        return CreateAstNode<AstTypeQualifierSeq>(beginTokIndex, qualifiers, std::move(layoutQuals));
    }

    auto Parser::ParseStructBody() -> std::vector<AstStructMemberDecl*>
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        ConsumeToken();

        if (TryConsumeToken(TokenKlass::RBrace)) {
            // Empty struct body
            return {};
        }

        std::vector<AstStructMemberDecl*> result;
        while (!Eof()) {
            // Fast path. Parse an empty decl
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                // TODO: report warning
                continue;
            }

            // Parse a member decl
            auto beginTokIndex = GetTokenIndex();
            auto typeResult    = ParseQualType();
            auto declarators   = ParseDeclaratorList();

            if (!InRecoveryMode()) {
                result.push_back(CreateAstNode<AstStructMemberDecl>(beginTokIndex, typeResult, std::move(declarators)));
                ParsePermissiveSemicolonHelper();
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
            GLSLD_ASSERT(InRecoveryMode());
            RecoverFromError(RecoveryMode::Brace);
            if (TryConsumeToken(TokenKlass::RBrace)) {
                ExitRecoveryMode();
            }
        }

        return result;
    }

    auto Parser::ParseStructDefinition() -> AstStructDecl*
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::K_struct));
        auto beginTokIndex = GetTokenIndex();
        ConsumeToken();

        // Parse the declared struct type name
        std::optional<SyntaxToken> declTok = std::nullopt;
        if (TryTestToken(TokenKlass::Identifier)) {
            declTok = PeekToken();
            ConsumeToken();
        }

        // Parse the struct body
        if (TryTestToken(TokenKlass::LBrace)) {
            auto structBodyResult = ParseStructBody();
            return CreateAstNode<AstStructDecl>(beginTokIndex, declTok, structBodyResult);
        }
        else {
            // FIXME: could we do better on recovery?
            EnterRecoveryMode();
            return nullptr;
        }
    }

    auto Parser::ParseArraySpec() -> AstArraySpec*
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(PeekToken().klass == TokenKlass::LBracket);
        auto beginTokIndex = GetTokenIndex();

        std::vector<AstExpr*> sizes;
        while (TryConsumeToken(TokenKlass::LBracket)) {

            // Parse empty/unsized specifier
            if (TryConsumeToken(TokenKlass::RBracket)) {
                sizes.push_back(nullptr);
                continue;
            }

            // TODO: expr? assignment_expr?
            sizes.push_back(ParseExpr());

            ParseClosingBracketHelper();
            if (InRecoveryMode()) {
                break;
            }
        }

        return CreateAstNode<AstArraySpec>(beginTokIndex, std::move(sizes));
    }

    auto Parser::ParseType(AstTypeQualifierSeq* quals) -> AstQualType*
    {
        GLSLD_TRACE_PARSER();

        // The range of AstQualType should be at the start of AstTypeQualifierSeq
        auto beginTokIndex = quals ? quals->GetSyntaxRange().startTokenIndex : GetTokenIndex();

        if (TryTestToken(TokenKlass::K_struct)) {
            // Parse struct definition
            auto structDefinition   = ParseStructDefinition();
            AstArraySpec* arraySpec = nullptr;
            if (TryTestToken(TokenKlass::LBracket)) {
                arraySpec = ParseArraySpec();
            }
            return CreateAstNode<AstQualType>(beginTokIndex, quals, structDefinition, arraySpec);
        }
        else if (TryTestToken(TokenKlass::Identifier) || GetGlslBuiltinType(PeekToken()).has_value()) {
            auto tok = PeekToken();
            ConsumeToken();
            AstArraySpec* arraySpec = nullptr;
            if (TryTestToken(TokenKlass::LBracket)) {
                arraySpec = ParseArraySpec();
            }
            return CreateAstNode<AstQualType>(beginTokIndex, quals, tok, arraySpec);
        }
        else {
            // FIXME: how to handle error
            EnterRecoveryMode();
            return nullptr;
        }
    }

    auto Parser::ParseQualType() -> AstQualType*
    {
        GLSLD_TRACE_PARSER();

        auto qualifiers = ParseTypeQualifiers();
        return ParseType(qualifiers);
    }

#pragma endregion

#pragma region Parsing Decl

    auto Parser::ParseDeclaration() -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        if (TryConsumeToken(TokenKlass::Semicolon)) {
            // empty decl
            return CreateAstNode<AstEmptyDecl>(beginTokIndex);
        }

        if (TryTestToken(TokenKlass::K_precision)) {
            // precision decl
            // FIXME: implement this
            ReportError("precision decl not supported yet");
            EnterRecoveryMode();
            return CreateAstNode<AstEmptyDecl>(beginTokIndex);
        }

        auto quals = ParseTypeQualifiers();
        if (TryTestToken(TokenKlass::Semicolon)) {
            // default qualifier decl
            // FIXME: implement this
            ReportError("default qualifier decl not supported yet");
            EnterRecoveryMode();
            return CreateAstNode<AstEmptyDecl>(beginTokIndex);
        }
        else if (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::Semicolon, TokenKlass::Comma, 1)) {
            // qualifier overwrite decl
            // FIXME: implement this
            ReportError("qualifier overwrite decl not supported yet");
            EnterRecoveryMode();
            return CreateAstNode<AstEmptyDecl>(beginTokIndex);
        }
        if (TryTestToken(TokenKlass::LBrace) ||
            (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::LBrace, 1))) {
            // interface blocks
            // for example, uniform UBO { ... }
            // FIXME: only enter here if qualifier can actually start a interface block
            return ParseInterfaceBlockDecl(beginTokIndex, quals);
        }
        else {
            // function/variable decl
            auto type = ParseType(quals);
            if (InRecoveryMode()) {
                // FIXME: what to do here?
                ReportError("what to do here?");
                return CreateAstNode<AstEmptyDecl>(beginTokIndex);
            }

            if (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::LParen, 1)) {
                // function decl
                return ParseFunctionDecl(beginTokIndex, type);
            }
            else if (TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier)) {
                // type/variable decl
                return ParseTypeOrVariableDecl(beginTokIndex, type);
            }
            else {
                if (type->GetStructDecl()) {
                    // If type is a struct definition, we assume this is a type decl with missing ';' for better
                    // diagnostic This would always fail. We are just trying to infer the ';'
                    ParsePermissiveSemicolonHelper();
                }
                else {
                    // Otherwise, it's an unknown decl
                    ReportError("unknown decl");
                    EnterRecoveryMode();
                }

                // In all cases, we'll return an empty decl with parsed type
                return CreateAstNode<AstVariableDecl>(beginTokIndex, type);
            }
        }
    }

    auto Parser::ParseDeclAndTryRecover() -> AstDecl*
    {
        auto declResult = ParseDeclaration();
        if (InRecoveryMode()) {
            RecoverFromError(RecoveryMode::Semi);
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                ExitRecoveryMode();
            }
        }

        return declResult;
    }

    auto Parser::ParseInitializer() -> AstInitializer*
    {
        GLSLD_TRACE_PARSER();

        if (TryTestToken(TokenKlass::LBrace)) {
            return ParseInitializerList();
        }
        else {
            return ParseExprNoComma();
        }
    }

    auto Parser::ParseInitializerList() -> AstInitializerList*
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
        size_t beginTokIndex = GetTokenIndex();
        ConsumeToken();

        std::vector<AstInitializer*> initItems;
        while (!Eof()) {
            if (TryTestToken(TokenKlass::RBrace)) {
                break;
            }
            else if (TryTestToken(TokenKlass::LBrace)) {
                initItems.push_back(ParseInitializerList());

                if (!TryConsumeToken(TokenKlass::Comma)) {
                    break;
                }
            }
            else {
                initItems.push_back(ParseExprNoComma());

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

        return CreateAstNode<AstInitializerList>(beginTokIndex, std::move(initItems));
    }

    auto Parser::ParseDeclarator() -> VariableDeclarator
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::Identifier));

        VariableDeclarator result;

        // Parse declaration identifier
        result.declTok = ParseDeclIdHelper();

        // Parse array specifier
        if (TryTestToken(TokenKlass::LBracket)) {
            result.arraySize = ParseArraySpec();
        }

        if (TryConsumeToken(TokenKlass::Assign)) {
            result.init = ParseInitializer();
        }

        return result;
    }

    auto Parser::ParseDeclaratorList() -> std::vector<VariableDeclarator>
    {
        GLSLD_TRACE_PARSER();

        std::vector<VariableDeclarator> result;
        while (TryTestToken(TokenKlass::Identifier)) {
            result.push_back(ParseDeclarator());

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
        ConsumeToken();

        // Empty parameter list
        if (TryConsumeToken(TokenKlass::RParen)) {
            return {};
        }

        // TODO: Needed? spec doesn't include this grammar.
        if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
            ConsumeToken();
            ConsumeToken();
            return {};
        }

        // Parse parameters
        std::vector<AstParamDecl*> result;
        while (!TryTestToken(TokenKlass::Eof)) {
            auto beginTokIndex = GetTokenIndex();

            auto type = ParseQualType();
            if (InRecoveryMode()) {
                break;
            }

            std::optional<VariableDeclarator> declarator;
            if (TryTestToken(TokenKlass::Identifier)) {
                declarator = ParseDeclarator();
            }
            else if (!TryTestToken(TokenKlass::RParen) && TryTestToken(TokenKlass::Comma, 1)) {
                // This is a special handling to permissively parse construct like the following:
                // `foo(int int, int y)`
                ConsumeToken();
            }

            result.push_back(CreateAstNode<AstParamDecl>(beginTokIndex, type, declarator));

            if (!TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        ParseClosingParenHelper();
        return std::move(result);
    }

    auto Parser::ParseFunctionDecl(size_t beginTokIndex, AstQualType* returnType) -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        // Parse function name
        auto declTok = ParseDeclIdHelper();

        // Parse function parameter list
        auto params = ParseFunctionParamList();

        if (TryTestToken(TokenKlass::LBrace)) {
            // This is a definition
            GLSLD_ASSERT(InParsingMode());

            auto body = ParseCompoundStmt();
            return CreateAstNode<AstFunctionDecl>(beginTokIndex, returnType, declTok, std::move(params), body);
        }
        else {
            // This is a declaration

            // Parse trailing ';'
            // NOTE we will keep the ';' for recovery if parsing failed
            if (InParsingMode() && !TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expect ';' or function body");
            }

            return CreateAstNode<AstFunctionDecl>(beginTokIndex, returnType, declTok, std::move(params));
        }
    }

    auto Parser::ParseTypeOrVariableDecl(size_t beginTokIndex, AstQualType* variableType) -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier));

        // Parse type decl
        if (TryConsumeToken(TokenKlass::Semicolon)) {
            return CreateAstNode<AstVariableDecl>(beginTokIndex, variableType);
        }

        // Parse variable decl
        auto declarators = ParseDeclaratorList();

        // Parse ';'
        ParsePermissiveSemicolonHelper();

        return CreateAstNode<AstVariableDecl>(beginTokIndex, variableType, declarators);
    }

    auto Parser::ParseInterfaceBlockDecl(size_t beginTokIndex, AstTypeQualifierSeq* quals) -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        // Parse interface block name
        GLSLD_ASSERT(TryTestToken(TokenKlass::Identifier, TokenKlass::LBrace));
        auto declTok = ParseDeclIdHelper();

        // Parse interface block body
        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        auto blockBody = ParseStructBody();
        if (InRecoveryMode() || TryConsumeToken(TokenKlass::Semicolon)) {
            return CreateAstNode<AstInterfaceBlockDecl>(beginTokIndex, quals, declTok, blockBody, std::nullopt);
        }

        // Parse declarator if any
        std::optional<VariableDeclarator> declarator;
        if (TryTestToken(TokenKlass::Identifier)) {
            declarator = ParseDeclarator();
        }

        // Parse ';'
        ParsePermissiveSemicolonHelper();

        return CreateAstNode<AstInterfaceBlockDecl>(beginTokIndex, quals, declTok, blockBody, declarator);
    }

    auto Parser::ParsePrecisionDecl() -> AstDecl*
    {
        GLSLD_TRACE_PARSER();

        GLSLD_NO_IMPL();
    }

#pragma endregion

#pragma region Parsing Expr

    auto Parser::ParseExpr() -> AstExpr*
    {
        auto beginTokIndex = GetTokenIndex();
        auto expr          = ParseCommaExpr();

        if (GetTokenIndex() == beginTokIndex) {
            EnterRecoveryMode();
        }

        return expr;
    }

    auto Parser::ParseExprNoComma() -> AstExpr*
    {
        auto beginTokIndex = GetTokenIndex();
        auto expr          = ParseAssignmentExpr();

        if (GetTokenIndex() == beginTokIndex) {
            EnterRecoveryMode();
        }

        return expr;
    }

    static auto GetPrefixUnaryOpDesc(TokenKlass klass) -> std::optional<UnaryOp>
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

    // These are all left-associative operators
    struct BinaryOpDesc
    {
        BinaryOp op;
        int precedence;
    };
    static auto GetBinaryOpDesc(TokenKlass tok) -> std::optional<BinaryOpDesc>
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

    static auto GetAssignmentOpDesc(TokenKlass klass) -> std::optional<BinaryOp>
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

    auto Parser::ParseCommaExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto lhsResult     = ParseAssignmentExpr();

        while (TryConsumeToken(TokenKlass::Comma)) {
            auto rhsResult = ParseAssignmentExpr();

            lhsResult = CreateAstNode<AstBinaryExpr>(beginTokIndex, BinaryOp::Comma, lhsResult, rhsResult);
        }

        return lhsResult;
    }

    auto Parser::ParseAssignmentExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto lhsResult     = ParseUnaryExpr();

        AstExpr* result = nullptr;
        auto opDesc     = GetAssignmentOpDesc(PeekToken().klass);
        if (opDesc) {
            ConsumeToken();
            GLSLD_ASSERT(InParsingMode());
            auto rhsResult = ParseAssignmentExpr();

            result = CreateAstNode<AstBinaryExpr>(beginTokIndex, *opDesc, lhsResult, rhsResult);
        }
        else {
            result = ParseBinaryOrConditionalExpr(beginTokIndex, lhsResult);
        }

        return result;
    }

    auto Parser::ParseBinaryOrConditionalExpr(size_t beginTokIndex, AstExpr* firstTerm) -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto predicateOrExprResult = ParseBinaryExpr(beginTokIndex, firstTerm, 0);

        if (!TryConsumeToken(TokenKlass::Question)) {
            return predicateOrExprResult;
        }

        auto ifBranchExpr = ParseCommaExpr();

        if (!TryConsumeToken(TokenKlass::Colon)) {
            if (InRecoveryMode()) {
                // we cannot infer a ':' in recovery mode, so just return early
                return CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult, ifBranchExpr,
                                                    CreateErrorExpr());
            }

            ReportError("expecting ':'");
        }

        // Even if ':' is missing, we'll continue parsing the nenative part
        auto elseBranchExpr = ParseAssignmentExpr();

        return CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult, ifBranchExpr, elseBranchExpr);
    }

    auto Parser::ParseBinaryExpr(size_t beginTokIndex, AstExpr* firstTerm, int minPrecedence) -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto lhs = firstTerm;
        while (true) {
            if (InRecoveryMode()) {
                // NOTE `lhs` is either unary_expr or binary expr, meaning we have incoming 'EOF' or ';' or '}'.
                // Obviously, we cannot parse anything so just leave early
                break;
            }

            auto opDesc = GetBinaryOpDesc(PeekToken().klass);
            if (!(opDesc.has_value() && opDesc->precedence >= minPrecedence)) {
                break;
            }

            // Consume the operator
            ConsumeToken();

            auto rhsBeginTokIndex = GetTokenIndex();
            auto rhs              = ParseUnaryExpr();

            while (true) {
                auto opDescNext = GetBinaryOpDesc(PeekToken().klass);
                if (!(opDescNext.has_value() && opDescNext->precedence > opDesc->precedence)) {
                    break;
                }

                rhs = ParseBinaryExpr(rhsBeginTokIndex, rhs, opDesc->precedence + 1);
            }

            lhs = CreateAstNode<AstBinaryExpr>(beginTokIndex, opDesc->op, lhs, rhs);
            // Since all binary operators are left-associative, the `beginTokIndex` should stay the same
        }

        return lhs;
    }

    auto Parser::ParseUnaryExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto unaryOp = GetPrefixUnaryOpDesc(PeekToken().klass);
        if (unaryOp) {
            auto beginTokIndex = GetTokenIndex();
            ConsumeToken();
            // TODO: avoid recursion
            auto childExpr = ParseUnaryExpr();
            return CreateAstNode<AstUnaryExpr>(beginTokIndex, *unaryOp, childExpr);
        }
        else {
            return ParsePostfixExpr();
        }
    }

    auto Parser::ParsePostfixExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse primary_expr or constructor call
        AstExpr* result = nullptr;
        if (GetGlslBuiltinType(PeekToken())) {
            auto tok = PeekToken();
            ConsumeToken();
            result = CreateAstNode<AstNameAccessExpr>(beginTokIndex, tok);
        }
        else {
            result = ParsePrimaryExpr();
        }

        bool parsedPostfix = true;
        while (InParsingMode() && parsedPostfix) {
            switch (PeekToken().klass) {
            case TokenKlass::LParen:
            {
                // Function call
                // NOTE we could have parsed `type_spec '(' ??? ')'`, which would be handled later
                auto args = ParseFunctionArgumentList();
                result    = CreateAstNode<AstInvokeExpr>(beginTokIndex, result, std::move(args));
                break;
            }
            case TokenKlass::LBracket:
            {
                // Indexing access
                auto arraySpec = ParseArraySpec();
                result         = CreateAstNode<AstIndexAccessExpr>(beginTokIndex, result, std::move(arraySpec));
                break;
            }
            case TokenKlass::Dot:
            {
                // Member access or .length() call
                ConsumeToken();
                auto accessName = ParseDeclIdHelper();
                if (accessName.text.Equals("length") && TryTestToken(TokenKlass::LParen) &&
                    TryTestToken(TokenKlass::RParen, 1)) {
                    ConsumeToken();
                    ConsumeToken();
                    result = CreateAstNode<AstUnaryExpr>(beginTokIndex, UnaryOp::Length, result);
                }
                else {
                    result = CreateAstNode<AstNameAccessExpr>(beginTokIndex, result, accessName);
                }
                break;
            }
            case TokenKlass::Increment:
            {
                // Postfix inc
                ConsumeToken();
                result = CreateAstNode<AstUnaryExpr>(beginTokIndex, UnaryOp::PostfixInc, result);
                break;
            }
            case TokenKlass::Decrement:
            {
                // Postfix dnc
                ConsumeToken();
                result = CreateAstNode<AstUnaryExpr>(beginTokIndex, UnaryOp::PostfixDec, result);
                break;
            }
            default:
                parsedPostfix = false;
                break;
            }
        }

        return result;
    }

    auto Parser::ParsePrimaryExpr() -> AstExpr*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        auto tok = PeekToken();
        switch (PeekToken().klass) {
        case TokenKlass::Identifier:
            // variable name
            ConsumeToken();
            return CreateAstNode<AstNameAccessExpr>(beginTokIndex, tok);
        case TokenKlass::IntegerConstant:
        case TokenKlass::FloatConstant:
        case TokenKlass::K_true:
        case TokenKlass::K_false:
            // constant
            ConsumeToken();
            return CreateAstNode<AstConstantExpr>(beginTokIndex, tok);
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

        GLSLD_ASSERT(TryTestToken(TokenKlass::LParen));
        ConsumeToken();

        if (TryConsumeToken(TokenKlass::RParen)) {
            return CreateErrorExpr();
        }

        auto result = ParseExpr();
        ParseClosingParenHelper();

        return result;
    };

    auto Parser::ParseFunctionArgumentList() -> std::vector<AstExpr*>
    {
        GLSLD_TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::LParen));
        ConsumeToken();

        if (TryConsumeToken(TokenKlass::RParen)) {
            return {};
        }

        if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
            ConsumeToken();
            ConsumeToken();
            return {};
        }

        std::vector<AstExpr*> result;
        while (!Eof()) {
            auto arg = ParseAssignmentExpr();
            if (InRecoveryMode()) {
                break;
            }

            result.push_back(arg);
            if (!TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        ParseClosingParenHelper();
        return result;
    }

#pragma endregion

#pragma region Parsing Stmt

    auto Parser::ParseStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        switch (PeekToken().klass) {
        case TokenKlass::LBrace:
            // compound stmt
            return ParseCompoundStmt();
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

    auto Parser::ParseCompoundStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse '{'
        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        ConsumeToken();

        // Parse child statements
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

        return CreateAstNode<AstCompoundStmt>(beginTokIndex, std::move(children));
    }

    auto Parser::ParseSelectionStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse 'K_if'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_if));
        ConsumeToken();

        // Parse predicate expression
        AstExpr* predicateExpr = ParseParenWrappedExprOrErrorHelper();

        if (InRecoveryMode()) {
            return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExpr, CreateErrorStmt());
        }

        // Parse if branch
        auto ifBranchStmt = ParseStmt();

        // Try parse 'K_else'
        // TODO: error handling
        if (!TryConsumeToken(TokenKlass::K_else)) {
            return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExpr, ifBranchStmt);
        }

        // TODO: check with if?
        auto elseBranchStmt = ParseStmt();
        // TODO: error handling
        return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExpr, ifBranchStmt, elseBranchStmt);
    }

    auto Parser::ParseForStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse 'K_for'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_for));
        ConsumeToken();

        // Parse '('
        if (!TryConsumeToken(TokenKlass::LParen)) {
            EnterRecoveryMode();
            return CreateAstNode<AstForStmt>(beginTokIndex, CreateErrorStmt(), CreateErrorExpr(), CreateErrorExpr(),
                                             CreateErrorStmt());
        }

        // Parse init clause
        // FIXME: must be simple stmt (non-compound)
        auto initClause = ParseStmt();
        if (InRecoveryMode()) {
            // FIXME: how to recover?
            return CreateAstNode<AstForStmt>(beginTokIndex, initClause, CreateErrorExpr(), CreateErrorExpr(),
                                             CreateErrorStmt());
        }

        // Parse test clause
        AstExpr* condExpr = nullptr;
        if (!TryConsumeToken(TokenKlass::Semicolon)) {
            condExpr = ParseExpr();

            if (!TryConsumeToken(TokenKlass::Semicolon)) {
                EnterRecoveryMode();
                return CreateAstNode<AstForStmt>(beginTokIndex, initClause, condExpr, CreateErrorExpr(),
                                                 CreateErrorStmt());
            }
        }

        // Parse proceed clause
        AstExpr* iterationExpr = nullptr;
        if (!TryTestToken(TokenKlass::RParen)) {
            iterationExpr = ParseExpr();
        }

        // FIXME: is the recovery correct?
        ParseClosingParenHelper();
        if (InRecoveryMode()) {
            return CreateAstNode<AstForStmt>(beginTokIndex, initClause, condExpr, iterationExpr, CreateErrorStmt());
        }

        // Parse loop body
        auto loopBody = ParseStmt();
        return CreateAstNode<AstForStmt>(beginTokIndex, initClause, condExpr, iterationExpr, loopBody);
    }

    auto Parser::ParseDoWhileStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse 'K_do'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_do));
        ConsumeToken();

        // Parse loop body
        auto loopBodyStmt = ParseStmt();

        // Parse 'K_while'
        if (!TryConsumeToken(TokenKlass::K_while)) {
            EnterRecoveryMode();
            return CreateAstNode<AstDoWhileStmt>(beginTokIndex, CreateErrorExpr(), loopBodyStmt);
        }

        // Parse predicate expr
        AstExpr* precidateExpr = ParseParenWrappedExprOrErrorHelper();

        // Parse ';'
        ParsePermissiveSemicolonHelper();

        return CreateAstNode<AstDoWhileStmt>(beginTokIndex, precidateExpr, loopBodyStmt);
    }

    auto Parser::ParseWhileStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER(TokenKlass::K_while);

        auto beginTokIndex = GetTokenIndex();

        // Parse 'K_while'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_while));
        ConsumeToken();

        // Parse predicate expr
        AstExpr* predicateExpr = ParseParenWrappedExprOrErrorHelper();

        if (InRecoveryMode()) {
            return CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr, CreateErrorStmt());
        }

        // Parse loop body
        auto bodyStmt = ParseStmt();
        return CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr, bodyStmt);
    }

    auto Parser::ParseLabelStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();
        auto beginTokIndex = GetTokenIndex();

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
        return CreateAstNode<AstLabelStmt>(beginTokIndex, caseExpr);
    }

    auto Parser::ParseSwitchStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER(TokenKlass::K_switch);

        auto beginTokIndex = GetTokenIndex();

        // Parse 'K_switch'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_switch));
        ConsumeToken();

        // Parse switched expr
        AstExpr* switchedExpr = ParseParenWrappedExprOrErrorHelper();

        // Parse switch body
        AstStmt* switchBody = nullptr;
        if (TryTestToken(TokenKlass::LBrace)) {
            switchBody = ParseCompoundStmt();
        }
        else {
            switchBody = CreateErrorStmt();
        }
        return CreateAstNode<AstSwitchStmt>(beginTokIndex, switchedExpr, switchBody);
    }

    auto Parser::ParseJumpStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER(TokenKlass::K_break, TokenKlass::K_continue, TokenKlass::K_discard, TokenKlass::K_return);

        auto beginTokIndex = GetTokenIndex();
        switch (PeekToken().klass) {
        case TokenKlass::K_break:
            ConsumeToken();
            ParsePermissiveSemicolonHelper();
            return CreateAstNode<AstJumpStmt>(beginTokIndex, JumpType::Break);
        case TokenKlass::K_continue:
            ConsumeToken();
            ParsePermissiveSemicolonHelper();
            return CreateAstNode<AstJumpStmt>(beginTokIndex, JumpType::Continue);
        case TokenKlass::K_discard:
            ConsumeToken();
            ParsePermissiveSemicolonHelper();
            return CreateAstNode<AstJumpStmt>(beginTokIndex, JumpType::Discard);
        case TokenKlass::K_return:
            ConsumeToken();
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                return CreateAstNode<AstReturnStmt>(beginTokIndex);
            }
            else {
                auto returnedExpr = ParseExprNoComma();

                ParsePermissiveSemicolonHelper();
                return CreateAstNode<AstReturnStmt>(beginTokIndex, returnedExpr);
            }
        default:
            GLSLD_UNREACHABLE();
        }
    }

    auto Parser::ParseExprStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse expr
        auto exprResult = ParseExpr();

        // Parse ';'
        ParsePermissiveSemicolonHelper();

        return CreateAstNode<AstExprStmt>(beginTokIndex, exprResult);
    }

    auto Parser::ParseDeclOrExprStmt() -> AstStmt*
    {
        GLSLD_TRACE_PARSER();

        // FIXME: infer expression statement more aggressively
        auto beginTokIndex = GetTokenIndex();
        switch (PeekToken().klass) {
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
            // Comma expr
        case TokenKlass::Comma:
            return ParseExprStmt();
        default:
            // FIXME: this is really hacky, we should do better
            if (TryTestToken(TokenKlass::Identifier) || PeekToken().IsKeyword()) {
                if (TryTestToken(TokenKlass::Identifier, 1)) {
                    // Case: `S x;`
                    return CreateAstNode<AstDeclStmt>(beginTokIndex, ParseDeclAndTryRecover());
                }
                else if (TryTestToken(TokenKlass::LBracket, 1)) {
                    // Case: `S[1] x;` or `S[1] = x;`

                    // We scan ahead to see the next token after ']'
                    size_t nextLookahead = 2;
                    int bracketDepth     = 1;
                    while (true) {
                        const auto lookaheadTok = PeekToken(nextLookahead);
                        nextLookahead += 1;

                        if (lookaheadTok.klass == TokenKlass::LBracket) {
                            bracketDepth += 1;
                        }
                        else if (lookaheadTok.klass == TokenKlass::RBracket) {
                            bracketDepth -= 1;
                            if (bracketDepth == 0) {
                                break;
                            }
                        }
                        else if (lookaheadTok.klass == TokenKlass::Eof || lookaheadTok.klass == TokenKlass::Semicolon) {
                            break;
                        }
                    }

                    if (bracketDepth == 0 && TryTestToken(TokenKlass::Identifier, nextLookahead)) {
                        return CreateAstNode<AstDeclStmt>(beginTokIndex, ParseDeclAndTryRecover());
                    }
                    else {
                        return ParseExprStmt();
                    }
                }
                else {
                    // Other cases
                    // FIXME: could we assume this?
                    return ParseExprStmt();
                }
            }
            else {
                // FIXME: need we recover here?
                return CreateAstNode<AstDeclStmt>(beginTokIndex, ParseDeclAndTryRecover());
            }
        }
    }

#pragma endregion

    auto Parser::RecoverFromError(RecoveryMode mode) -> void
    {
        // FIXME: is this a parser?
        GLSLD_TRACE_PARSER();

        auto desiredToken = static_cast<TokenKlass>(mode);

        size_t initParenDepth   = parenDepth;
        size_t initBracketDepth = bracketDepth;
        size_t initBraceDepth   = braceDepth;

        if (mode == RecoveryMode::Paren) {
            GLSLD_ASSERT(initParenDepth > 0);
        }
        if (mode == RecoveryMode::Bracket) {
            GLSLD_ASSERT(initBracketDepth > 0);
        }
        if (mode == RecoveryMode::Brace) {
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
                    // skip an isolated ')'
                    break;
                }
                if (mode == RecoveryMode::Paren && parenDepth == initParenDepth) {
                    return;
                }
                break;
            case TokenKlass::RBracket:
                if (bracketDepth == 0) {
                    // skip an isolated ']'
                    break;
                }
                if (mode == RecoveryMode::Bracket && bracketDepth == initBracketDepth) {
                    return;
                }
                break;
            case TokenKlass::RBrace:
                if (braceDepth == 0) {
                    // skip an isolated '}'
                    break;
                }
                if ((mode == RecoveryMode::Paren || mode == RecoveryMode::Bracket || mode == RecoveryMode::Brace ||
                     mode == RecoveryMode::IListBrace || mode == RecoveryMode::Semi) &&
                    braceDepth == initBraceDepth) {
                    return removeDepthIfUnclosed();
                }
                break;
            case TokenKlass::Semicolon:
                if ((mode == RecoveryMode::Paren || mode == RecoveryMode::Bracket || mode == RecoveryMode::Semi) &&
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
} // namespace glsld
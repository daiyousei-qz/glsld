#include "Parser.h"

namespace glsld
{

#pragma region Parsing Misc

    auto Parser::ParsePermissiveSemicolon() -> void
    {
        if (InParsingMode()) {
            if (!TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expecting ';'");
                // However, we don't do error recovery as if the ';' is inferred by the parser.
            }
        }
    }

    auto Parser::ParseClosingParen() -> void
    {
        if (TryConsumeToken(TokenKlass::RParen)) {
            if (InRecoveryMode()) {
                ExitRecoveryMode();
            }
        }
        else {
            ReportError("expect ')'");

            RecoverFromError(RecoveryMode::Paren);
            if (!TryConsumeToken(TokenKlass::RParen)) {
                // we cannot find the closing ')' to continue parsing
                EnterRecoveryMode();
            }
        }
    }

    auto Parser::ParseClosingBracket() -> void
    {
        if (TryConsumeToken(TokenKlass::RBracket)) {
            if (InRecoveryMode()) {
                ExitRecoveryMode();
            }
        }
        else {
            ReportError("expect ']'");

            RecoverFromError(RecoveryMode::Bracket);
            if (!TryConsumeToken(TokenKlass::RBracket)) {
                // we cannot find the closing ')' to continue parsing
                EnterRecoveryMode();
            }
        }
    }

    auto Parser::ParseDeclId() -> SyntaxToken
    {
        if (PeekToken().IsIdentifier()) {
            auto result = PeekToken();
            ConsumeToken();
            return result;
        }
        else {
            // FIXME: which token should we return? RECOVERY?
            ReportError("Expect identifier");
            return SyntaxToken{};
        }
    }

    auto Parser::ParseParenWrappedExprOrError() -> AstExpr*
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
    auto Parser::ParseLayoutQualifier() -> AstLayoutQualifier*
    {
        TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::K_layout));

        auto beginTokIndex = GetTokenIndex();
        ConsumeToken();

        std::vector<LayoutItem> result;
        if (!TryConsumeToken(TokenKlass::LParen)) {
            return CreateAstNode<AstLayoutQualifier>(beginTokIndex, result);
        }

        while (!Eof()) {
            auto idToken = ParseDeclId();

            AstExpr* value = nullptr;
            if (TryConsumeToken(TokenKlass::Assign)) {
                value = ParseExprNoComma();
            }

            if (InRecoveryMode() || !TryConsumeToken(TokenKlass::Comma)) {
                break;
            }

            result.push_back(LayoutItem{idToken, value});
        }

        ParseClosingParen();
        return CreateAstNode<AstLayoutQualifier>(beginTokIndex, std::move(result));
    }

    auto Parser::ParseTypeQualifiers() -> AstTypeQualifierSeq*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        QualifierGroup qualifiers;
        std::vector<AstLayoutQualifier*> layoutQuals;

        bool inQualSeq = true;
        while (InParsingMode() && inQualSeq) {
            // FIXME: implement correctly
            switch (PeekToken().klass) {
                // precision qualifier
            case TokenKlass::K_highp:
                qualifiers.SetHighp();
                break;
            case TokenKlass::K_mediump:
                qualifiers.SetMediump();
                break;
            case TokenKlass::K_lowp:
                qualifiers.SetLowp();
                break;
                // invariance qual?
            case TokenKlass::K_invariant:
                // precise qual?
            case TokenKlass::K_precise:
                // memory qualifier
            case TokenKlass::K_coherent:
            case TokenKlass::K_volatile:
            case TokenKlass::K_restrict:
            case TokenKlass::K_readonly:
            case TokenKlass::K_writeonly:
                break;
                // storage qual (also, parameter qual)
            case TokenKlass::K_const:
                qualifiers.SetConst();
                break;
            case TokenKlass::K_in:
                qualifiers.SetIn();
                break;
            case TokenKlass::K_out:
                qualifiers.SetOut();
                break;
            case TokenKlass::K_inout:
                qualifiers.SetInout();
                break;
            case TokenKlass::K_attribute:
                qualifiers.SetAttribute();
                break;
            case TokenKlass::K_uniform:
                qualifiers.SetUniform();
                break;
            case TokenKlass::K_varying:
                qualifiers.SetVarying();
                break;
            case TokenKlass::K_buffer:
                qualifiers.SetBuffer();
                break;
            case TokenKlass::K_shared:
                qualifiers.SetShared();
                break;
                // auxiliary storage qual
            case TokenKlass::K_centroid:
                qualifiers.SetCentroid();
                break;
            case TokenKlass::K_sample:
                qualifiers.SetSample();
                break;
            case TokenKlass::K_patch:
                qualifiers.SetPatch();
                break;
                // Interpolation qual
            case TokenKlass::K_smooth:
            case TokenKlass::K_flat:
            case TokenKlass::K_noperspective:
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
                layoutQuals.push_back(ParseLayoutQualifier());
                inQualSeq = true;
            }
        }

        // TODO: return nullptr if not set
        return CreateAstNode<AstTypeQualifierSeq>(beginTokIndex, qualifiers, std::move(layoutQuals));
    }

    auto Parser::ParseStructBody() -> std::vector<AstStructMemberDecl*>
    {
        TRACE_PARSER();

        ConsumeTokenAssert(TokenKlass::LBrace);

        if (TryConsumeToken(TokenKlass::RBrace)) {
            // empty struct body
            return {};
        }

        std::vector<AstStructMemberDecl*> result;
        while (!Eof()) {
            // Parse empty decl
            // FIXME: report error?
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                continue;
            }

            auto beginTokIndex = GetTokenIndex();
            auto typeResult    = ParseQualType();
            auto declarators   = ParseVariableDeclarators();

            if (!InRecoveryMode()) {
                result.push_back(CreateAstNode<AstStructMemberDecl>(beginTokIndex, typeResult, std::move(declarators)));
                ParsePermissiveSemicolon();
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
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        ConsumeTokenAssert(TokenKlass::K_struct);

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

        // FIXME: could we do better on recovery?
        EnterRecoveryMode();
        return nullptr;
    }

    auto Parser::ParseArraySpec() -> AstArraySpec*
    {
        TRACE_PARSER();

        GLSLD_ASSERT(PeekToken().klass == TokenKlass::LBracket);
        auto beginTokIndex = GetTokenIndex();

        std::vector<AstExpr*> sizes;
        while (TryConsumeToken(TokenKlass::LBracket)) {

            // Parse unsized specifier
            if (TryConsumeToken(TokenKlass::RBracket)) {
                sizes.push_back(nullptr);
                continue;
            }

            // TODO: expr? assignment_expr?
            sizes.push_back(ParseExpr());

            ParseClosingBracket();
            if (InRecoveryMode()) {
                break;
            }
        }

        return CreateAstNode<AstArraySpec>(beginTokIndex, std::move(sizes));
    }

    auto Parser::ParseType(AstTypeQualifierSeq* quals) -> AstQualType*
    {
        TRACE_PARSER();

        // FIXME: shouldn't this be the beginning of qualifiers?
        auto beginTokIndex = GetTokenIndex();

        if (TryTestToken(TokenKlass::K_struct)) {
            auto structDefinitionResult = ParseStructDefinition();
            AstArraySpec* arraySpec     = nullptr;
            if (TryTestToken(TokenKlass::LBracket)) {
                arraySpec = ParseArraySpec();
            }
            return CreateAstNode<AstQualType>(beginTokIndex, quals, structDefinitionResult, arraySpec);
        }
        else if (TryTestToken(TokenKlass::Identifier) || GetBuiltinType(PeekToken()).has_value()) {
            auto tok = PeekToken();
            ConsumeToken();
            AstArraySpec* arraySpec = nullptr;
            if (TryTestToken(TokenKlass::LBracket)) {
                arraySpec = ParseArraySpec();
            }
            return CreateAstNode<AstQualType>(beginTokIndex, quals, tok, arraySpec);
        }

        // FIXME: how to handle error
        EnterRecoveryMode();
        return nullptr;
    }

    auto Parser::ParseQualType() -> AstQualType*
    {
        TRACE_PARSER();

        auto qualifiers = ParseTypeQualifiers();
        return ParseType(qualifiers);
    }

#pragma endregion

#pragma region Parsing Decl

    auto Parser::ParseDeclaration() -> AstDecl*
    {
        TRACE_PARSER();

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

            if (TryTestToken(TokenKlass::LParen) ||
                (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::LParen, 1))) {
                // function decl
                return ParseFunctionDecl(beginTokIndex, type);
            }
            else if (TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier)) {
                // type/variable decl
                return ParseTypeOrVariableDecl(beginTokIndex, type);
            }
            else if (type->GetStructDecl()) {
                // If type is a struct definition, we assume it's type decl with missing ';' for better diagnostic

                // This would always fail, so we are just trying to infer the ';'
                ParsePermissiveSemicolon();

                return CreateAstNode<AstVariableDecl>(beginTokIndex, type);
            }
            else {
                // unknown decl
                ReportError("unknown decl");
                EnterRecoveryMode();
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

    auto Parser::ParseVariableDeclarators() -> std::vector<VariableDeclarator>
    {
        TRACE_PARSER();

        std::vector<VariableDeclarator> result;
        while (!Eof()) {
            auto declTok = ParseDeclId();

            AstArraySpec* arraySpec = nullptr;
            if (TryTestToken(TokenKlass::LBracket)) {
                arraySpec = ParseArraySpec();
            }

            // FIXME: parse initializer
            AstExpr* init = nullptr;
            if (TryConsumeToken(TokenKlass::Assign)) {
                if (TryTestToken(TokenKlass::LBrace)) {
                    init = ParseInitializerExpr();
                }
                else {
                    init = ParseExpr();
                }
            }

            result.push_back(VariableDeclarator{.declTok = declTok, .arraySize = arraySpec, .init = init});
            if (InRecoveryMode() || !TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        return std::move(result);
    }

    auto Parser::ParseFunctionParamList() -> std::vector<AstParamDecl*>
    {
        TRACE_PARSER();

        ConsumeTokenAssert(TokenKlass::LParen);

        // empty parameter list
        if (TryConsumeToken(TokenKlass::RParen)) {
            return {};
        }

        // TODO: needed? spec doesn't include this grammar.
        if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
            ConsumeToken();
            ConsumeToken();
            return {};
        }

        // parse parameters
        std::vector<AstParamDecl*> result;
        while (PeekToken().klass != TokenKlass::Eof) {
            auto beginTokIndex = GetTokenIndex();

            auto type = ParseQualType();
            if (InRecoveryMode()) {
                break;
            }

            std::optional<SyntaxToken> id;
            if (TryTestToken(TokenKlass::Identifier)) {
                id = ParseDeclId();
            }
            else if (TryTestToken(TokenKlass::Comma, 1)) {
                // FIXME: do we want to do such recovery here?
                ConsumeToken();
            }

            result.push_back(CreateAstNode<AstParamDecl>(beginTokIndex, type, id));

            if (!TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        ParseClosingParen();
        return std::move(result);
    }

    auto Parser::ParseFunctionDecl(size_t beginTokIndex, AstQualType* returnType) -> AstDecl*
    {
        TRACE_PARSER();

        // Parse function name
        auto declTok = ParseDeclId();

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
        TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier));

        // Parse type decl
        if (TryConsumeToken(TokenKlass::Semicolon)) {
            return CreateAstNode<AstVariableDecl>(beginTokIndex, variableType);
        }

        // Parse variable decl
        auto declarators = ParseVariableDeclarators();

        // Parse ';'
        ParsePermissiveSemicolon();

        return CreateAstNode<AstVariableDecl>(beginTokIndex, variableType, declarators);
    }

    auto Parser::ParseInterfaceBlockDecl(size_t beginTokIndex, AstTypeQualifierSeq* quals) -> AstDecl*
    {
        TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::Identifier, TokenKlass::LBrace));
        auto declTok = ParseDeclId();

        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        auto blockBody = ParseStructBody();
        if (InRecoveryMode() || TryConsumeToken(TokenKlass::Semicolon)) {
            return CreateAstNode<AstInterfaceBlockDecl>(beginTokIndex, declTok, blockBody);
        }

        std::vector<VariableDeclarator> declarators;
        if (TryTestToken(TokenKlass::Identifier)) {
            declarators = ParseVariableDeclarators();
        }
        ParsePermissiveSemicolon();

        if (declarators.size() > 1) {
            ReportError("interface block could only have one declatorator");
        }

        if (declarators.size() > 0) {
            return CreateAstNode<AstInterfaceBlockDecl>(beginTokIndex, declTok, blockBody, declarators[0]);
        }
        else {
            return CreateAstNode<AstInterfaceBlockDecl>(beginTokIndex, declTok, blockBody);
        }
    }

    auto Parser::ParsePrecisionDecl() -> AstDecl*
    {
        TRACE_PARSER();

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

    // Parse an expression without comma operator. If no token is consumed, enter recovery mode to advance parsing.
    //
    // PARSE: expr_nocomma
    //      - expr_nocomma := assignment_expr
    //
    // RECOVERY: ?
    auto Parser::ParseExprNoComma() -> AstExpr*
    {
        auto beginTokIndex = GetTokenIndex();
        auto expr          = ParseAssignmentExpr();

        if (GetTokenIndex() == beginTokIndex) {
            EnterRecoveryMode();
        }

        return expr;
    }

    // EXPECT: '{'
    //
    // PARSE: '{' ... '}'
    auto Parser::ParseInitializerExpr() -> AstExpr*
    {
        auto beginTokIndex = GetTokenIndex();

        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        ConsumeToken();

        // FIXME: implement this
        RecoverFromError(RecoveryMode::Brace);

        if (!TryConsumeToken(TokenKlass::RBrace)) {
            EnterRecoveryMode();
        }

        return CreateErrorExpr();
    }

    static auto GetPrefixUnaryOpDesc(TokenKlass klass) -> std::optional<UnaryOp>
    {
        switch (klass) {
        case TokenKlass::Plus:
            return UnaryOp::Identity;
        case TokenKlass::Dash:
            return UnaryOp::Nagate;
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
        TRACE_PARSER();

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
        TRACE_PARSER();

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
        TRACE_PARSER();

        auto predicateOrExprResult = ParseBinaryExpr(beginTokIndex, firstTerm, 0);

        if (!TryConsumeToken(TokenKlass::Question)) {
            return predicateOrExprResult;
        }

        auto ifBranchExpr = ParseCommaExpr();

        if (!TryConsumeToken(TokenKlass::Colon)) {
            if (InRecoveryMode()) {
                // we cannot infer a ':' in recovery mode, so just return early
                auto tokIndex = GetTokenIndex();
                return CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult, ifBranchExpr,
                                                    CreateRangedAstNode<AstErrorExpr>(tokIndex, tokIndex));
            }

            ReportError("expecting ':'");
        }

        // Even if ':' is missing, we'll continue parsing the nenative part
        auto elseBranchExpr = ParseAssignmentExpr();

        return CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult, ifBranchExpr, elseBranchExpr);
    }

    auto Parser::ParseBinaryExpr(size_t beginTokIndex, AstExpr* firstTerm, int minPrecedence) -> AstExpr*
    {
        TRACE_PARSER();

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
        TRACE_PARSER();

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
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse primary_expr or constructor call
        AstExpr* result = nullptr;
        if (GetBuiltinType(PeekToken())) {
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
                // Member access
                ConsumeToken();
                auto accessName = ParseDeclId();
                result          = CreateAstNode<AstNameAccessExpr>(beginTokIndex, result, accessName);
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
        TRACE_PARSER();

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
            return CreateRangedAstNode<AstErrorExpr>(beginTokIndex, beginTokIndex);
        }
    }

    auto Parser::ParseParenWrappedExpr() -> AstExpr*
    {
        TRACE_PARSER();

        ConsumeTokenAssert(TokenKlass::LParen);

        if (TryConsumeToken(TokenKlass::RParen)) {
            return CreateErrorExpr();
        }

        auto result = ParseExpr();
        ParseClosingParen();

        return result;
    };

#pragma endregion

#pragma region Parsing Stmt

    auto Parser::ParseStmt() -> AstStmt*
    {
        TRACE_PARSER();

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
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse '{'
        GLSLD_ASSERT(TryTestToken(TokenKlass::LBrace));
        ConsumeTokenAssert(TokenKlass::LBrace);

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
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse 'K_if'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_if));
        ConsumeTokenAssert(TokenKlass::K_if);

        // Parse predicate expression
        AstExpr* predicateExpr = ParseParenWrappedExprOrError();

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
        TRACE_PARSER();

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
        ParseClosingParen();
        if (InRecoveryMode()) {
            return CreateAstNode<AstForStmt>(beginTokIndex, initClause, condExpr, iterationExpr, CreateErrorStmt());
        }

        // Parse loop body
        auto loopBody = ParseStmt();
        return CreateAstNode<AstForStmt>(beginTokIndex, initClause, condExpr, iterationExpr, loopBody);
    }

    auto Parser::ParseDoWhileStmt() -> AstStmt*
    {
        TRACE_PARSER();

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
        AstExpr* precidateExpr = ParseParenWrappedExprOrError();

        // Parse ';'
        ParsePermissiveSemicolon();

        return CreateAstNode<AstDoWhileStmt>(beginTokIndex, precidateExpr, loopBodyStmt);
    }

    auto Parser::ParseWhileStmt() -> AstStmt*
    {
        TRACE_PARSER(TokenKlass::K_while);

        auto beginTokIndex = GetTokenIndex();

        // Parse 'K_while'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_while));
        ConsumeToken();

        // Parse predicate expr
        AstExpr* predicateExpr = ParseParenWrappedExprOrError();

        if (InRecoveryMode()) {
            return CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr, CreateErrorStmt());
        }

        // Parse loop body
        auto bodyStmt = ParseStmt();
        return CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr, bodyStmt);
    }

    auto Parser::ParseLabelStmt() -> AstStmt*
    {
        TRACE_PARSER();
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
        TRACE_PARSER(TokenKlass::K_switch);

        auto beginTokIndex = GetTokenIndex();

        // Parse 'K_switch'
        GLSLD_ASSERT(TryTestToken(TokenKlass::K_switch));
        ConsumeToken();

        // Parse switched expr
        AstExpr* switchedExpr = ParseParenWrappedExprOrError();

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
        TRACE_PARSER(TokenKlass::K_break, TokenKlass::K_continue, TokenKlass::K_discard, TokenKlass::K_return);

        auto beginTokIndex = GetTokenIndex();
        switch (PeekToken().klass) {
        case TokenKlass::K_break:
            ConsumeToken();
            ParsePermissiveSemicolon();
            return CreateAstNode<AstJumpStmt>(beginTokIndex, JumpType::Break);
        case TokenKlass::K_continue:
            ConsumeToken();
            ParsePermissiveSemicolon();
            return CreateAstNode<AstJumpStmt>(beginTokIndex, JumpType::Continue);
        case TokenKlass::K_discard:
            ConsumeToken();
            ParsePermissiveSemicolon();
            return CreateAstNode<AstJumpStmt>(beginTokIndex, JumpType::Discard);
        case TokenKlass::K_return:
            ConsumeToken();
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                return CreateAstNode<AstReturnStmt>(beginTokIndex);
            }
            else {
                auto returnedExpr = ParseExprNoComma();

                ParsePermissiveSemicolon();
                return CreateAstNode<AstReturnStmt>(beginTokIndex, returnedExpr);
            }
        default:
            GLSLD_UNREACHABLE();
        }
    }

    auto Parser::ParseExprStmt() -> AstStmt*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse expr
        auto exprResult = ParseExpr();

        // Parse ';'
        ParsePermissiveSemicolon();

        return CreateAstNode<AstExprStmt>(beginTokIndex, exprResult);
    }

    auto Parser::ParseDeclOrExprStmt() -> AstStmt*
    {
        TRACE_PARSER();

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
        case TokenKlass::Identifier:
            if (!TryTestToken(TokenKlass::Identifier, 1)) {
                return ParseExprStmt();
            }
            [[fallthrough]];
        default:
            // FIXME: need we recover here?
            // FIXME: `vec3(0);` is being treated as function declaration but it's a constructor call expr
            return CreateAstNode<AstDeclStmt>(beginTokIndex, ParseDeclAndTryRecover());
        }
    }

#pragma endregion
} // namespace glsld
#include "Parser.h"

namespace glsld
{
#pragma region Parsing QualType
    auto Parser::ParseLayoutQualifier() -> void
    {
        TRACE_PARSER();
    }

    auto Parser::ParseTypeQualifiers() -> AstTypeQualifierSeq*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        AstTypeQualifierSeq result;

        bool inQualSeq = true;
        while (inQualSeq) {
            // FIXME: implement correctly
            switch (PeekToken().klass) {
                // precision qualifier
            case TokenKlass::K_highp:
                result.SetHighp();
                break;
            case TokenKlass::K_mediump:
                result.SetMediump();
                break;
            case TokenKlass::K_lowp:
                result.SetLowp();
                break;
                // invariance qual?
                // precise qual?
                // memory qualifier
            case TokenKlass::K_coherent:
            case TokenKlass::K_volatile:
            case TokenKlass::K_restrict:
            case TokenKlass::K_readonly:
            case TokenKlass::K_writeonly:
                break;
                // layout qual
            case TokenKlass::K_layout:
                break;
                // storage qual (also, parameter qual)
            case TokenKlass::K_const:
                result.SetConst();
                break;
            case TokenKlass::K_in:
                result.SetIn();
                break;
            case TokenKlass::K_out:
                result.SetOut();
                break;
            case TokenKlass::K_inout:
                result.SetInout();
                break;
            case TokenKlass::K_attribute:
                result.SetAttribute();
                break;
            case TokenKlass::K_uniform:
                result.SetUniform();
                break;
            case TokenKlass::K_varying:
                result.SetVarying();
                break;
            case TokenKlass::K_buffer:
                result.SetBuffer();
                break;
            case TokenKlass::K_shared:
                result.SetShared();
                break;
                // auxiliary storage qual
            case TokenKlass::K_centroid:
                result.SetCentroid();
                break;
            case TokenKlass::K_sample:
                result.SetSample();
                break;
            case TokenKlass::K_patch:
                result.SetPatch();
                break;
            default:
                inQualSeq = false;
                break;
            }

            if (inQualSeq) {
                ConsumeToken();
            }
        }

        // TODO: return nullptr if not set
        return CreateAstNode<AstTypeQualifierSeq>(beginTokIndex, result);
    }

    auto Parser::ParseStructBody() -> ParseResult<std::vector<AstStructMemberDecl*>>
    {
        TRACE_PARSER();

        ConsumeTokenAssert(TokenKlass::LBrace);

        if (TryTestToken(TokenKlass::RBrace)) {
            // empty struct body
            return {{}};
        }

        std::vector<AstStructMemberDecl*> result;
        while (true) {
            auto typeResult  = ParseQualType();
            auto declarators = ParseVariableDeclarators();

            if (!TryTestToken(TokenKlass::Semicolon, TokenKlass::RBrace)) {
                // error recovery
            }
            if (TryTestToken(TokenKlass::Semicolon)) {
                ConsumeToken();
            }
            if (TryTestToken(TokenKlass::RBrace)) {
                ConsumeToken();
                break;
            }
        }
        return result;
    }

    auto Parser::ParseStructDefinition() -> ParseResult<AstStructDecl*>
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
            return {structBodyResult.Success(),
                    CreateAstNode<AstStructDecl>(beginTokIndex, declTok, structBodyResult.Move())};
        }

        // FIXME: could we do better on recovery?
        return {false, nullptr};
    }

    auto Parser::ParseArraySpec() -> ParseResult<AstArraySpec*>
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
            sizes.push_back(ParseExpr().Move());

            if (!TryConsumeToken(TokenKlass::RBracket)) {
                ReportError("expecting ]");
                // FIXME: end global decl?
                RecoverFromError(RecoveryMode::Semi);
                if (!TryConsumeToken(TokenKlass::RBracket)) {
                    return {false, CreateAstNode<AstArraySpec>(beginTokIndex, std::move(sizes))};
                }
            }
        }

        GLSLD_ASSERT(!sizes.empty());
        return CreateAstNode<AstArraySpec>(beginTokIndex, std::move(sizes));
    }

    auto Parser::ParseType(AstTypeQualifierSeq* quals) -> ParseResult<AstQualType*>
    {
        TRACE_PARSER();

        // FIXME: shouldn't this be the beginning of qualifiers?
        auto beginTokIndex = GetTokenIndex();

        // FIXME: parse array spec
        if (TryTestToken(TokenKlass::K_struct)) {
            auto structDefinitionResult = ParseStructDefinition();
            return {structDefinitionResult.Success(),
                    CreateAstNode<AstQualType>(beginTokIndex, quals, structDefinitionResult.Get())};
        }

        if (TryTestToken(TokenKlass::Identifier) || GetBuiltinType(PeekToken()).has_value()) {
            auto tok = PeekToken();
            ConsumeToken();
            return CreateAstNode<AstQualType>(beginTokIndex, quals, tok);
        }

        // TODO: handle error
        return nullptr;
    }

    auto Parser::ParseQualType() -> ParseResult<AstQualType*>
    {
        TRACE_PARSER();

        auto qualifiers = ParseTypeQualifiers();
        return ParseType(qualifiers);
    }

#pragma endregion

#pragma region Parsing Decl

    auto Parser::ParseDeclaration() -> ParseResult<AstDecl*>
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
            return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
        }

        auto quals = ParseTypeQualifiers();
        if (TryTestToken(TokenKlass::Semicolon)) {
            // default qualifier decl
            // FIXME: implement this
            ReportError("default qualifier decl not supported yet");
            return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
        }
        else if (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::Semicolon, TokenKlass::Comma, 1)) {
            // qualifier overwrite decl
            // FIXME: implement this
            ReportError("qualifier overwrite decl not supported yet");
            return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
        }
        if (TryTestToken(TokenKlass::LBrace) || TryTestToken(TokenKlass::LBrace, 1)) {
            // interface blocks
            // for example, uniform UBO { ... }
            // FIXME: only enter here if qualifier can actually start a interface block
            return ParseInterfaceBlockDecl(beginTokIndex, quals);
        }
        else {
            // function/variable decl
            auto typeResult = ParseType(quals);
            if (!typeResult.Success()) {
                // FIXME: what to do here?
                ReportError("what to do here?");
                return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
            }

            if (TryTestToken(TokenKlass::LParen) ||
                (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::LParen, 1))) {
                // function decl
                return ParseFunctionDecl(beginTokIndex, typeResult.Get());
            }
            else if (TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier)) {
                // type/variable decl
                return ParseTypeOrVariableDecl(beginTokIndex, typeResult.Get());
            }
            else {
                // unknown decl
                ReportError("unknown decl");
                return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
            }
        }
    }

    auto Parser::ParseDeclNoRecovery() -> ParseResult<AstDecl*>
    {
        auto declResult = ParseDeclaration();
        if (!declResult.Success()) {
            RecoverFromBadDecl();
        }

        return {true, declResult.Get()};
    }

    auto Parser::ParseVariableDeclarators() -> ParseResult<std::vector<VariableDeclarator>>
    {
        TRACE_PARSER();

        bool recoveryMode = false;
        std::vector<VariableDeclarator> result;
        while (!Eof()) {
            auto declTok = ParseDeclId();

            AstArraySpec* arraySpec = nullptr;
            if (TryTestToken(TokenKlass::LBracket)) {
                auto arraySpecResult = ParseArraySpec();
                if (!arraySpecResult.Success()) {
                    recoveryMode = true;
                }

                arraySpec = arraySpecResult.Get();
            }

            // FIXME: parse init
            AstExpr* init = nullptr;
            if (TryConsumeToken(TokenKlass::Assign)) {
                auto initResult = ParseExpr();
                if (!initResult.Success()) {
                    recoveryMode = true;
                }

                init = initResult.Get();
            }

            result.push_back(VariableDeclarator{.declTok = declTok, .arraySize = arraySpec, .init = init});
            if (recoveryMode || !TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        return {!recoveryMode, std::move(result)};
    }

    auto Parser::ParseFunctionParamList() -> ParseResult<std::vector<AstParamDecl*>>
    {
        TRACE_PARSER();

        ConsumeTokenAssert(TokenKlass::LParen);

        // empty parameter list
        if (TryConsumeToken(TokenKlass::RParen)) {
            return {{}};
        }

        // TODO: needed? spec doesn't include this grammar.
        if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
            ConsumeToken();
            ConsumeToken();
            return {{}};
        }

        // parse parameters
        std::vector<AstParamDecl*> result;
        while (PeekToken().klass != TokenKlass::Eof) {
            auto beginTokIndex = GetTokenIndex();

            auto type = ParseQualType();
            if (!type.Success()) {
                break;
            }

            auto id = ParseDeclId();
            if (id.klass == TokenKlass::Error) {
                break;
            }

            result.push_back(CreateAstNode<AstParamDecl>(beginTokIndex, type.Move(), id));

            if (!TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        bool closed = TryConsumeToken(TokenKlass::RParen);
        if (!closed) {
            // FIXME: end local paren?
            RecoverFromError(RecoveryMode::Paren);
            closed = TryConsumeToken(TokenKlass::RParen);
        }

        return {closed, std::move(result)};
    }

    auto Parser::ParseFunctionDecl(size_t beginTokIndex, AstQualType* returnType) -> ParseResult<AstDecl*>
    {
        TRACE_PARSER();

        // Parse function name
        auto declTok = ParseDeclId();

        // Parse function parameter list
        auto paramsResult = ParseFunctionParamList();

        if (TryTestToken(TokenKlass::LBrace)) {
            GLSLD_ASSERT(paramsResult.Success());

            auto bodyResult = ParseCompoundStmt();
            return {bodyResult.Success(), CreateAstNode<AstFunctionDecl>(beginTokIndex, returnType, declTok,
                                                                         paramsResult.Move(), bodyResult.Get())};
        }

        // Parse trailing ';'
        // NOTE we will keep the ';' for recovery if parsing failed
        if (paramsResult.Success() && !TryConsumeToken(TokenKlass::Semicolon)) {
            ReportError("expect ';' or function body");
        }

        return {paramsResult.Success(),
                CreateAstNode<AstFunctionDecl>(beginTokIndex, returnType, declTok, paramsResult.Move())};
    }

    auto Parser::ParseTypeOrVariableDecl(size_t beginTokIndex, AstQualType* variableType) -> ParseResult<AstDecl*>
    {
        TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier));

        // Parse type decl
        if (TryConsumeToken(TokenKlass::Semicolon)) {
            return CreateAstNode<AstVariableDecl>(beginTokIndex, variableType);
        }

        // Parse variable decl
        auto declaratorsResult = ParseVariableDeclarators();

        // Parse trailing ';'
        // NOTE we will keep the ';' for recovery if parsing failed
        if (declaratorsResult.Success()) {
            ParsePermissiveSemicolon();
        }

        return {declaratorsResult.Success(),
                CreateAstNode<AstVariableDecl>(beginTokIndex, variableType, declaratorsResult.Move())};
    }

    auto Parser::ParseInterfaceBlockDecl(size_t beginTokIndex, AstTypeQualifierSeq* quals) -> ParseResult<AstDecl*>
    {
        TRACE_PARSER();

        auto declTok = ParseDeclId();

        auto blockBodyResult = ParseStructBody();
        if (!blockBodyResult.Success() || TryConsumeToken(TokenKlass::Semicolon)) {
            return {blockBodyResult.Success(),
                    CreateAstNode<AstInterfaceBlockDecl>(beginTokIndex, declTok, blockBodyResult.Move())};
        }

        auto declaratorsResult = ParseVariableDeclarators();
        if (declaratorsResult.Success()) {
            ParsePermissiveSemicolon();
        }

        // FIXME: error on multiple declarators properly
        GLSLD_ASSERT(declaratorsResult.Get().size() == 1);
        return {declaratorsResult.Success(),
                CreateAstNode<AstInterfaceBlockDecl>(beginTokIndex, declTok, blockBodyResult.Move(),
                                                     declaratorsResult.Get().front())};
    }

    auto Parser::ParsePrecisionDecl() -> ParseResult<AstDecl*>
    {
        TRACE_PARSER();

        GLSLD_NO_IMPL();
    }

#pragma endregion

#pragma region Parsing Expr

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

    auto Parser::ParseExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto lhsResult     = ParseAssignmentExpr();

        while (TryConsumeToken(TokenKlass::Comma)) {
            GLSLD_ASSERT(lhsResult.Success());
            auto rhsResult = ParseAssignmentExpr();

            lhsResult = {rhsResult.Success(), CreateAstNode<AstBinaryExpr>(beginTokIndex, BinaryOp::Comma,
                                                                           lhsResult.Get(), rhsResult.Get())};
        }

        // NOTE an expression parser could consume zero token and return an error expr.
        // If so, we explicit put the parser into recovery mode.
        if (GetTokenIndex() == beginTokIndex) {
            return {false, lhsResult.Get()};
        }

        return lhsResult;
    }

    auto Parser::ParseAssignmentExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto lhsResult     = ParseUnaryExpr();

        auto opDesc = GetAssignmentOpDesc(PeekToken().klass);
        if (opDesc) {
            ConsumeToken();
            GLSLD_ASSERT(lhsResult.Success());
            auto rhsResult = ParseAssignmentExpr();
            return {rhsResult.Success(),
                    CreateAstNode<AstBinaryExpr>(beginTokIndex, *opDesc, lhsResult.Move(), rhsResult.Move())};
        }
        else {
            return ParseBinaryOrConditionalExpr(beginTokIndex, lhsResult);
        }
    }

    auto Parser::ParseBinaryOrConditionalExpr(size_t beginTokIndex, ParseResult<AstExpr*> firstTerm)
        -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto predicateOrExprResult = ParseBinaryExpr(beginTokIndex, firstTerm, 0);

        if (!TryConsumeToken(TokenKlass::Question)) {
            return predicateOrExprResult;
        }

        auto positveExprResult = ParseExpr();

        if (!positveExprResult.Success()) {
            auto tokIndex     = GetTokenIndex();
            auto negativeExpr = CreateRangedAstNode<AstErrorExpr>(tokIndex, tokIndex);
            return {false, CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult.Move(),
                                                        positveExprResult.Move(), negativeExpr)};
        }

        if (!TryConsumeToken(TokenKlass::Colon)) {
            ReportError("expecting ':'");
        }

        // Even if ':' is missing, we'll continue parsing the nenative part
        auto negativeExprResult = ParseAssignmentExpr();

        return {negativeExprResult.Success(),
                CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult.Move(), positveExprResult.Move(),
                                             negativeExprResult.Move())};
    }

    auto Parser::ParseBinaryExpr(size_t beginTokIndex, ParseResult<AstExpr*> firstTerm, int minPrecedence)
        -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto lhs = firstTerm;
        while (true) {
            if (!lhs.Success()) {
                // NOTE lhs is either unary_expr or binary expr, meaning we have incoming 'EOF' or ';' or '}'.
                // Obviously, we cannot parse anything so just quit early
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

            lhs = {rhs.Success(), CreateAstNode<AstBinaryExpr>(beginTokIndex, opDesc->op, lhs.Get(), rhs.Get())};
            // Since all binary operators are left-associative, the `beginTokIndex` should stay the same
        }

        return lhs;
    }

    auto Parser::ParseUnaryExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto opDesc = GetPrefixUnaryOpDesc(PeekToken().klass);
        if (opDesc) {
            auto beginTokIndex = GetTokenIndex();
            ConsumeToken();
            // TODO: avoid recursion
            auto childExpr = ParseUnaryExpr();
            return ParseResult<AstExpr*>(childExpr.Success(),
                                         CreateAstNode<AstUnaryExpr>(beginTokIndex, *opDesc, childExpr.Move()));
        }
        else {
            return ParsePostfixExpr();
        }
    }

    auto Parser::ParsePostfixExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse primary_expr or constructor call
        ParseResult<AstExpr*> result{false, nullptr};
        if (GetBuiltinType(PeekToken())) {
            result = ParseConstructorCall();
        }
        else {
            // FIXME: construct like 'S[2](1, 2)' where S[2] is a type could fall into this case.
            //        handle correctly!
            result = ParsePrimaryExpr();
        }

        // FIXME: implement this
        bool parsedPostfix = true;
        while (!result.Success() && parsedPostfix) {
            parsedPostfix = false;
            switch (PeekToken().klass) {
            case TokenKlass::LParen:
            {
                // function call
                // FIXME: constructor call is "type_spec '(' ??? ')'"
                auto args = ParseFunctionArgumentList();
                result    = ParseResult<AstExpr*>(args.Success(),
                                               CreateAstNode<AstInvokeExpr>(beginTokIndex, InvocationType::FunctionCall,
                                                                            result.Move(), args.Move()));
                break;
            }
            case TokenKlass::LBracket:
            {
                // indexing access
                // FIXME: impl this
                GLSLD_NO_IMPL();
                // auto args = ParseArraySpec();
                // result    = ParseResult<AstExpr*>(
                //     args.Success(),
                //     CreateAstNode<AstInvokeExpr>(beginTokIndex, InvocationType::Indexing, result.Move(),
                //     args.Move()));
                break;
            }
            case TokenKlass::Dot:
            {
                // name access
                // FIXME: recovery?
                ConsumeToken();
                auto accessName = ParseDeclId();
                result          = CreateAstNode<AstNameAccessExpr>(beginTokIndex, result.Get(), accessName);
            }

            break;
            case TokenKlass::Increment:
            {
                // postfix inc
                ConsumeToken();
                result = CreateAstNode<AstUnaryExpr>(beginTokIndex, UnaryOp::PostfixInc, result.Get());
                break;
            }
            case TokenKlass::Decrement:
            {
                // postfix dnc
                ConsumeToken();
                result = CreateAstNode<AstUnaryExpr>(beginTokIndex, UnaryOp::PostfixDec, result.Get());
                break;
            }
            default:
                break;
            }
        }

        return result;
    }

    auto Parser::ParsePrimaryExpr() -> ParseResult<AstExpr*>
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
            // TODO: save more info?
            ConsumeToken();
            return CreateAstNode<AstConstantExpr>(beginTokIndex, tok.text);
        case TokenKlass::LParen:
            // expr in wrapped parens
            return ParseParenWrappedExpr();
        default:
            // error, but we see this as a successful parse
            ReportError("expect identifier or constant");
            return CreateRangedAstNode<AstErrorExpr>(beginTokIndex, beginTokIndex);
        }
    }

    auto Parser::ParseParenWrappedExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        ConsumeTokenAssert(TokenKlass::LParen);

        if (TryConsumeToken(TokenKlass::RParen)) {
            return CreateErrorExpr();
        }

        auto result = ParseExpr();

        // consume ")"
        bool closed = TryConsumeToken(TokenKlass::RParen);
        if (!closed) {
            ReportError("expect ')'");

            RecoverFromError(RecoveryMode::Paren);
            closed = TryConsumeToken(TokenKlass::RParen);
        }

        return {closed, result.Move()};
    };

#pragma endregion

#pragma region Parsing Stmt

    auto Parser::ParseStmt() -> ParseResult<AstStmt*>
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
            // labeled stmt
            GLSLD_NO_IMPL();
        default:
            // expression/declaration stmt
            return ParseDeclOrExprStmt();
        }
    }

    auto Parser::ParseStmtNoRecovery() -> ParseResult<AstStmt*>
    {
        auto stmtResult = ParseStmt();
        if (!stmtResult.Success()) {
            RecoverFromBadStmt();
        }

        return {true, stmtResult.Get()};
    }

    auto Parser::ParseCompoundStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::LBrace);

        auto beginTokIndex = GetTokenIndex();

        // consume "{"
        ConsumeTokenAssert(TokenKlass::LBrace);

        std::vector<AstStmt*> children;
        while (!Eof() && !TryTestToken(TokenKlass::RBrace)) {
            children.push_back(ParseStmtNoRecovery().Get());
        }

        if (Eof()) {
            ReportError("unexpected EOF");
        }
        else {
            ConsumeTokenAssert(TokenKlass::RBrace);
        }

        return {!Eof(), CreateAstNode<AstCompoundStmt>(beginTokIndex, std::move(children))};
    }

    auto Parser::ParseSelectionStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::K_if);

        auto beginTokIndex = GetTokenIndex();

        // parse "K_if"
        ConsumeTokenAssert(TokenKlass::K_if);

        // parse predicate condition
        auto predicateExprResult = ParseParenWrappedExpr();
        if (!predicateExprResult.Success()) {
            return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExprResult.Get(), CreateErrorStmt());
        }

        // parse positive branch
        auto positiveStmtResult = ParseStmt();
        // TODO: error handling

        if (!TryConsumeToken(TokenKlass::K_else)) {
            return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExprResult.Get(), positiveStmtResult.Get());
        }

        // TODO: check with if?
        auto negativeStmtResult = ParseStmt();
        // TODO: error handling
        return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExprResult.Get(), positiveStmtResult.Get(),
                                        negativeStmtResult.Get());
    }

    auto Parser::ParseForStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::K_for);

        GLSLD_NO_IMPL();
    }

    auto Parser::ParseWhileStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::K_while);

        auto beginTokIndex = GetTokenIndex();

        // parse 'K_while'
        ConsumeTokenAssert(TokenKlass::K_while);

        // parse predicate condition
        auto predicateExpr = ParseParenWrappedExpr();
        if (!predicateExpr.Success()) {
            return {false, CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr.Get(), CreateErrorStmt())};
        }

        // parse loop body
        auto bodyStmt = ParseStmt();
        return {bodyStmt.Success(), CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr.Get(), bodyStmt.Get())};
    }

    auto Parser::ParseSwitchStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::K_switch);
        GLSLD_NO_IMPL();
    }

    auto Parser::ParseJumpStmt() -> ParseResult<AstStmt*>
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
                // FIXME: recovery mode?
                auto returnExprResult = ParseExpr();

                if (returnExprResult.Success()) {
                    ParsePermissiveSemicolon();
                }
                return {returnExprResult.Success(),
                        CreateAstNode<AstReturnStmt>(beginTokIndex, returnExprResult.Get())};
            }
        default:
            GLSLD_UNREACHABLE();
        }
    }

    auto Parser::ParseExprStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto exprResult    = ParseExpr();

        // if failed, we keep the ';' for error recovery
        if (exprResult.Success()) {
            ParsePermissiveSemicolon();
        }

        return {exprResult.Success(), CreateAstNode<AstExprStmt>(beginTokIndex, exprResult.Get())};
    }

    auto Parser::ParseDeclOrExprStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        switch (PeekToken().klass) {
        case TokenKlass::IntegerConstant:
        case TokenKlass::FloatConstant:
        case TokenKlass::K_true:
        case TokenKlass::K_false:
        case TokenKlass::LParen:
            return ParseExprStmt();
        case TokenKlass::Identifier:
            if (!TryTestToken(TokenKlass::Identifier, 1)) {
                return ParseExprStmt();
            }
            [[fallthrough]];
        default:
            return CreateAstNode<AstDeclStmt>(beginTokIndex, ParseDeclNoRecovery().Get());
        }
    }

#pragma endregion
} // namespace glsld
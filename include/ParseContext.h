#pragma once
#include "DiagnosticManager.h"
#include "Tokenizer.h"

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace glsld
{
    class ParseContext;

    template <typename T> struct ParseResult
    {
        std::optional<T*> result;
    };

    using ParseResultExpr = std::optional<AstExpr*>;
    using ParseResultStmt = std::optional<AstStmt*>;
    using ParseResultDecl = std::optional<AstDecl*>;
    using ParseResultType = std::optional<AstQualType*>;

    template <typename Parser> using ParserResultType = std::invoke_result_t<Parser, ParseContext>;

    class ParseContext
    {
    public:
        ParseContext(std::vector<SyntaxToken> tokens) : currentTokIndex(0), tokenStream(tokens)
        {
        }

        auto Eof() -> bool
        {
            return PeekToken().klass == TokenKlass::Eof;
        }

        template <typename Parser> auto SepBy(TokenKlass sep, Parser elemParser)
        {
            std::vector<int> result;
        }

        template <typename Parser1, typename Parser2>
        auto Seq(Parser1 parser1, Parser2 parser2) -> std::tuple<ParserResultType<Parser1>, ParserResultType<Parser2>>
        {
            auto result1 = std::invoke(parser1, this);
            if (!result1) {
                return {std::move(result1), std::nullopt};
            }

            auto result2 = std::invoke(parser2, this);
            return {std::move(result1), std::move(result2)};
        }

        auto ParseDeclId() -> std::optional<AstDeclId*>
        {
            if (PeekToken().klass == TokenKlass::Identifier) {
                auto result = CreateAstNode<AstDeclId>({}, std::string{PeekToken().text});
                ConsumeToken();
                return result;
            }
            else {
                // error
                return nullptr;
            }
        }

        auto ParseTypeQualifiers() -> AstTypeQualifierSeq*
        {
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
            return CreateAstNode<AstTypeQualifierSeq>({}, result);
        }

        // EXPECT: ID
        // PARSE: (ID[N] = init) ,...
        auto ParseVariableDeclarators() -> std::vector<VariableDeclarator>
        {
            std::vector<VariableDeclarator> result;

            while (!Eof()) {
                AstDeclId* id = *ParseDeclId();

                AstArraySpec* arraySpec = nullptr;
                if (TryToken(TokenKlass::LBracket)) {
                    arraySpec = ParseArraySpec();
                }

                AstExpr* init = nullptr;
                if (TryToken(TokenKlass::Assign)) {
                    // parse init
                    GLSLD_NO_IMPL();
                }

                result.push_back(VariableDeclarator{.id = id, .arraySize = arraySpec, .init = init});
                if (TryToken(TokenKlass::Comma)) {
                    ConsumeToken();
                }
                else {
                    break;
                }
            }

            return result;
        }

        // EXPECT: LBrace
        // PARSE: { }
        auto ParseStructBody() -> std::vector<AstStructMemberDecl*>
        {
            ConsumeTokenAssert(TokenKlass::LBrace);

            if (TryToken(TokenKlass::RBrace)) {
                // empty struct body
                return {};
            }

            std::vector<AstStructMemberDecl*> result;
            while (true) {
                // TODO: parse layout qualifiers
                AstQualType* type = ParseType(nullptr);
                // TODO: support multiple declarators
                AstDeclId* id = *ParseDeclId();

                if (!TryToken(TokenKlass::Semicolon, TokenKlass::RBrace)) {
                    // error recovery
                }
                if (TryToken(TokenKlass::Semicolon)) {
                    ConsumeToken();
                }
                if (TryToken(TokenKlass::RBrace)) {
                    ConsumeToken();
                    break;
                }
            }
            return result;
        }

        // EXPECT: K_struct
        auto ParseStructDefinition() -> AstQualType*
        {
            ConsumeTokenAssert(TokenKlass::K_struct);
            AstDeclId* name = nullptr;
            if (TryToken(TokenKlass::Identifier)) {
                // TODO: parse name
            }

            if (TryToken(TokenKlass::LBrace)) {
                ParseStructBody();
            }

            // error recovery
            GLSLD_NO_IMPL();
        }

        // EXPECT: LBracket
        // PARSE: [N]...
        // ER: continue
        auto ParseArraySpec() -> AstArraySpec*
        {
            std::vector<AstExpr*> sizes;
            while (TryToken(TokenKlass::LBracket)) {
                ConsumeTokenAssert(TokenKlass::LBracket);

                if (TryToken(TokenKlass::RBracket)) {
                    sizes.push_back(nullptr);
                    continue;
                }

                AstExpr* dimSize = ParseExpr();
                sizes.push_back(dimSize);

                if (TryToken(TokenKlass::RBracket)) {
                    ConsumeToken();
                }
                else {
                    ReportError("expecting ]");
                }
            }

            return CreateAstNode<AstArraySpec>({}, std::move(sizes));
        }

        auto ParseType(AstTypeQualifierSeq* quals) -> AstQualType*
        {
            if (TryToken(TokenKlass::K_struct)) {
                return ParseStructDefinition();
            }

            switch (PeekToken().klass) {
            case TokenKlass::K_void:
                ConsumeToken();
                return CreateAstNode<AstQualType>({}, "void", quals);
            case TokenKlass::K_int:
                ConsumeToken();
                return CreateAstNode<AstQualType>({}, "int", quals);
            case TokenKlass::K_float:
                ConsumeToken();
                return CreateAstNode<AstQualType>({}, "float", quals);
            default:
                // error
                return nullptr;
            }
        }
        auto ParseQualType() -> ParseResultType
        {
            auto qualifiers = ParseTypeQualifiers();
            return ParseType(qualifiers);
        }

        // EXPECT: (
        // PARSE: (...)
        // ERROR: skip after ) or before ;
        auto ParseFunctionParamList() -> std::vector<AstParamDecl*>
        {
            std::vector<AstParamDecl*> result;

            ConsumeTokenAssert(TokenKlass::LParen);

            // empty parameter list
            if (TryToken(TokenKlass::RParen)) {
                ConsumeToken();
                return result;
            }

            // parse parameters
            while (PeekToken().klass != TokenKlass::Eof) {
                auto [type, id] = Seq(&ParseContext::ParseQualType, &ParseContext::ParseDeclId);

                if (type && id) {
                    result.push_back(CreateAstNode<AstParamDecl>({}, *type, *id));
                }

                if (TryToken(TokenKlass::Comma)) {
                    ConsumeToken();
                }
                else {
                    break;
                }
            }

            if (TryToken(TokenKlass::RParen)) {
                ConsumeToken();
            }
            else {
                RecoverFromError(RecoveryMode::Paren);
            }

            return result;
        }

        // EXPECT: {
        // PARSE: { ... }
        auto ParseFunctionBody() -> AstCompoundStmt*
        {
            return ParseCompoundStmt();
        }

        // EXPECT: ID
        // PARSE: ID(...) [{...}]
        auto ParseFunctionDecl(AstQualType* returnType) -> AstFunctionDecl*
        {
            // assert, never fail
            auto id = *ParseDeclId();

            //
            auto params = ParseFunctionParamList();

            if (TryToken(TokenKlass::Semicolon)) {
                ConsumeToken();
                return CreateAstNode<AstFunctionDecl>({}, returnType, id, std::move(params));
            }
            else if (TryToken(TokenKlass::LBrace)) {
                auto body = ParseFunctionBody();
                return CreateAstNode<AstFunctionDecl>({}, returnType, id, std::move(params), body);
            }

            // error recovery, return a function anyway
            GLSLD_NO_IMPL();
        }

        // EXPECT: K_precision
        auto ParseTypePrecisionDecl() -> AstDecl*
        {
            GLSLD_NO_IMPL();
        }

        // - global (in/out/uniform)
        //   - type-qual? type-spec;
        //   - type-qual? type-spec id = init;
        //   - type-qual? type-spec id[N] = init;
        //   - type-qual? type-spec id = init, ...;
        // - precision settings
        //   - precision precision-qual type;
        // - other settings
        //   - type-qual;
        //   - type-qual id;
        //   - type-qual id, ...;
        // - block {in/out/uniform/buffer}
        //   - type-qual id { ... };
        //   - type-qual id { ... } id;
        //   - type-qual id { ... } id[N];
        // - struct
        //   - struct id? { ... };
        //   - struct id? { ... } id;
        //   - struct id? { ... } id[N];
        // - function
        //   - type-qual? type-spec id(...);
        //   - type-qual? type-spec id(...) { ... }
        auto DoParseExternalDecl() -> void
        {
            if (TryToken(TokenKlass::Semicolon)) {
                // empty decl
                GLSLD_NO_IMPL();
            }

            if (TryToken(TokenKlass::K_precision)) {
                // precision decl
                GLSLD_NO_IMPL();
                // return ParseTypePrecisionDecl();
            }

            auto quals = ParseTypeQualifiers();
            if (TryToken(TokenKlass::Semicolon)) {
                // early return
                // for example, "layout(...) in;"
                GLSLD_NO_IMPL();
            }

            if (TryToken(TokenKlass::LBrace, 1) || TryToken(TokenKlass::LBrace)) {
                // interface blocks
                // for example, uniform UBO { ... }
                if (!TryToken(TokenKlass::Identifier)) {
                    // need a block name
                }
                GLSLD_NO_IMPL();
            }
            else {
                auto type = ParseType(quals);
                if (TryToken(TokenKlass::LParen, 1)) {
                    // function decl
                    externalDecls.push_back(ParseFunctionDecl(type));
                }
                else {
                    // variable decl
                    auto decl = CreateAstNode<AstVariableDecl>({}, type, ParseVariableDeclarators());
                    externalDecls.push_back(decl);
                }
            }

            // if error
            if (false) {
                RecoverFromError(RecoveryMode::Semi);
            }

            if (TryToken(TokenKlass::Semicolon)) {
                ConsumeToken();
            }
            else {
                // TODO: sometimes ; is not needed, like function definition
            }
        }

        auto DoParseTranslationUnit() -> void
        {
            while (!Eof()) {
                DoParseExternalDecl();
            }
        }

        auto ParseExpr() -> AstExpr*
        {
            auto lhs = ParseAssignmentExpr();

            while (TryToken(TokenKlass::Comma)) {
                ConsumeToken();
                auto rhs = ParseAssignmentExpr();

                lhs = CreateAstNode<AstBinaryExpr>({}, ExprOp::Comma, lhs, rhs);
            }

            return lhs;
        }
        auto ParseAssignmentExpr() -> AstExpr*
        {
            auto lhs = ParseUnaryExpr();

            auto opDesc = GetAssignmentOpDesc(PeekToken().klass);
            if (opDesc) {
                ConsumeToken();
                return CreateAstNode<AstBinaryExpr>({}, *opDesc, lhs, ParseAssignmentExpr());
            }
            else {
                return ParseConditionalExpr(lhs);
            }
        }

        auto ParseConditionalExpr(AstExpr* firstTerm) -> AstExpr*
        {
            auto predicateOrExpr = ParseBinaryExpr(firstTerm, 0);

            if (PeekToken().klass != TokenKlass::Question) {
                return predicateOrExpr;
            }

            auto positveExpr = ParseExpr();
            if (PeekToken().klass != TokenKlass::Colon) {
                // emit error
            }
            auto negativeExpr = ParseAssignmentExpr();

            return CreateAstNode<AstSelectExpr>({}, predicateOrExpr, positveExpr, negativeExpr);
        }

        auto GetPrefixUnaryOpDesc(TokenKlass klass) -> std::optional<ExprOp>
        {
            switch (PeekToken().klass) {
            case TokenKlass::Plus:
                return ExprOp::Plus;
            case TokenKlass::Dash:
                return ExprOp::Minus;
            case TokenKlass::Tilde:
                return ExprOp::BitwiseNot;
            case TokenKlass::Bang:
                return ExprOp::LogicalNot;
            case TokenKlass::Increment:
                return ExprOp::PrefixInc;
            case TokenKlass::Decrement:
                return ExprOp::PrefixDec;
            default:
                return std::nullopt;
            }
        }

        // These are all left-associative operators
        struct BinaryOpDesc
        {
            ExprOp op;
            int precedence;
        };
        auto GetBinaryOpDesc(TokenKlass tok) -> std::optional<BinaryOpDesc>
        {
            switch (tok) {
            case TokenKlass::Or:
                return BinaryOpDesc{ExprOp::LogicalOr, 0};
            case TokenKlass::Xor:
                return BinaryOpDesc{ExprOp::LogicalXor, 1};
            case TokenKlass::And:
                return BinaryOpDesc{ExprOp::LogicalAnd, 2};
            case TokenKlass::VerticalBar:
                return BinaryOpDesc{ExprOp::BitwiseOr, 3};
            case TokenKlass::Caret:
                return BinaryOpDesc{ExprOp::BitwiseXor, 4};
            case TokenKlass::Ampersand:
                return BinaryOpDesc{ExprOp::BitwiseAnd, 5};
            case TokenKlass::Equal:
                return BinaryOpDesc{ExprOp::Equal, 6};
            case TokenKlass::NotEqual:
                return BinaryOpDesc{ExprOp::NotEqual, 6};
            case TokenKlass::LAngle:
                return BinaryOpDesc{ExprOp::Less, 7};
            case TokenKlass::RAngle:
                return BinaryOpDesc{ExprOp::Greater, 7};
            case TokenKlass::LessEq:
                return BinaryOpDesc{ExprOp::LessEq, 7};
            case TokenKlass::GreaterEq:
                return BinaryOpDesc{ExprOp::GreaterEq, 7};
            case TokenKlass::LShift:
                return BinaryOpDesc{ExprOp::ShiftLeft, 8};
            case TokenKlass::RShift:
                return BinaryOpDesc{ExprOp::ShiftRight, 8};
            case TokenKlass::Plus:
                return BinaryOpDesc{ExprOp::Plus, 9};
            case TokenKlass::Dash:
                return BinaryOpDesc{ExprOp::Minus, 9};
            case TokenKlass::Star:
                return BinaryOpDesc{ExprOp::Mul, 10};
            case TokenKlass::Slash:
                return BinaryOpDesc{ExprOp::Div, 10};
            case TokenKlass::Percent:
                return BinaryOpDesc{ExprOp::Modulo, 10};
            default:
                return std::nullopt;
            }
        };

        auto GetAssignmentOpDesc(TokenKlass klass) -> std::optional<ExprOp>
        {
            switch (klass) {
            case TokenKlass::Assign:
                return ExprOp::Assign;
            case TokenKlass::MulAssign:
                return ExprOp::MulAssign;
            case TokenKlass::DivAssign:
                return ExprOp::DivAssign;
            case TokenKlass::ModAssign:
                return ExprOp::ModAssign;
            case TokenKlass::AddAssign:
                return ExprOp::AddAssign;
            case TokenKlass::SubAssign:
                return ExprOp::SubAssign;
            case TokenKlass::LShiftAssign:
                return ExprOp::LShiftAssign;
            case TokenKlass::RShiftAssign:
                return ExprOp::RShiftAssign;
            case TokenKlass::AndAssign:
                return ExprOp::AndAssign;
            case TokenKlass::XorAssign:
                return ExprOp::XorAssign;
            case TokenKlass::OrAssign:
                return ExprOp::OrAssign;
            default:
                return std::nullopt;
            }
        }

        // term for this is a unary expression
        auto ParseBinaryExpr(AstExpr* firstTerm, int minPrecedence) -> AstExpr*
        {

            auto lhs = firstTerm;
            while (true) {
                auto opDesc = GetBinaryOpDesc(PeekToken().klass);
                if (!(opDesc.has_value() && opDesc->precedence >= minPrecedence)) {
                    break;
                }

                ConsumeToken();
                auto rhs = ParseUnaryExpr();

                while (true) {
                    auto opDescNext = GetBinaryOpDesc(PeekToken().klass);
                    if (!(opDescNext.has_value() && opDescNext->precedence > opDesc->precedence)) {
                        break;
                    }

                    rhs = ParseBinaryExpr(rhs, opDesc->precedence + 1);
                }

                lhs = CreateAstNode<AstBinaryExpr>({}, opDesc->op, lhs, rhs);
            }

            return lhs;
        }

        // unary-expr:
        // - [unary-op]... postfix_expr
        auto ParseUnaryExpr() -> AstExpr*
        {
            // TODO: avoid recursion
            auto opDesc = GetPrefixUnaryOpDesc(PeekToken().klass);
            if (opDesc) {
                ConsumeToken();
                return CreateAstNode<AstUnaryExpr>({}, *opDesc, ParseUnaryExpr());
            }
            else {
                return ParsePostfixExpr();
            }
        }

        // postfix-expr:
        // - primary_expr [postfix]...
        auto ParsePostfixExpr() -> AstExpr*
        {
            auto result = ParsePrimaryExpr();

            while (true) {
                bool parsedPostfix = false;
                switch (PeekToken().klass) {
                case TokenKlass::LParen:
                    // function call
                    break;
                case TokenKlass::LBracket:
                    // indexing
                    break;
                case TokenKlass::Dot:
                    // access chain
                    break;
                case TokenKlass::Increment:
                case TokenKlass::Decrement:
                    // inc/dec
                    break;
                default:
                    break;
                }

                if (!parsedPostfix) {
                    break;
                }
            }

            return result;
        }

        // primary-expr:
        // - identifier
        // - int-constant
        // - float-constant
        // - bool-constant
        // - paren-wrapped-expr
        auto ParsePrimaryExpr() -> AstExpr*
        {
            auto range = PeekToken().range;
            auto text  = PeekToken().text;

            switch (PeekToken().klass) {
            case TokenKlass::Identifier:
                // variable name
                ConsumeToken();
                return CreateAstNode<AstVarAccessExpr>({}, std::string{text});
            case TokenKlass::IntegerConstant:
            case TokenKlass::FloatConstant:
                // integer/float constant
                ConsumeToken();
                return CreateAstNode<AstConstantExpr>({}, std::string{text});
            case TokenKlass::K_true:
                // bool constant (true)
                return CreateAstNode<AstConstantExpr>(range, "true");
            case TokenKlass::K_false:
                // bool constant (false)
                return CreateAstNode<AstConstantExpr>(range, "false");
            case TokenKlass::LParen:
                // expr in wrapped parens
                return ParseParenWrappedExpr();
            default:
                // error
                return CreateAstNode<AstErrorExpr>({});
            }
        }

        auto ParseParenWrappedExpr() -> AstExpr*
        {
            ConsumeTokenAssert(TokenKlass::LParen);

            auto result = ParseExpr();

            // consume ")"
            if (!ConsumeTokenIf(TokenKlass::RParen)) {
                // emit error
            }

            return result;
        };

        auto ParseCompoundStmt() -> AstCompoundStmt*
        {
            // consume "{"
            ConsumeTokenAssert(TokenKlass::LBrace);

            std::vector<AstExpr*> children;
            while (!Eof()) {
                if (TryToken(TokenKlass::RBrace)) {
                    // consume "}"
                    ConsumeToken();
                    return CreateAstNode<AstCompoundStmt>({}, std::move(children));
                }

                auto stmt = ParseStmt();
                GLSLD_ASSERT(stmt != nullptr);

                children.push_back(stmt);
            }

            // TODO: error handling
            return nullptr;
        }

        auto ParseSelectionStmt() -> AstIfStmt*
        {
            // consume "if"
            ConsumeTokenAssert(TokenKlass::K_if);

            auto predicateExpr = ParseParenWrappedExpr();
            auto positiveStmt  = ParseStmt();

            if (PeekToken().klass != TokenKlass::K_else) {
                return CreateAstNode<AstIfStmt>({}, predicateExpr, positiveStmt);
            }

            ConsumeTokenAssert(TokenKlass::K_else);

            // TODO: check with if?
            auto negativeStmt = ParseStmt();
            return CreateAstNode<AstIfStmt>({}, predicateExpr, positiveStmt, negativeStmt);
        }

        auto ParseForStmt() -> AstForStmt*
        {
            GLSLD_NO_IMPL();
        }

        auto ParseWhileStmt() -> AstWhileStmt*
        {
            ConsumeTokenAssert(TokenKlass::K_while);

            auto predicateExpr = ParseParenWrappedExpr();
            auto bodyStmt      = ParseStmt();

            return CreateAstNode<AstWhileStmt>({}, predicateExpr, bodyStmt);
        }

        auto ParseSwitchStmt() -> AstSwitchStmt*
        {
            ConsumeTokenAssert(TokenKlass::K_while);

            GLSLD_NO_IMPL();
        }

        // TODO: unify jump stmt?
        auto ParseJumpStmt() -> AstExpr*
        {
            auto range = PeekToken().range;

            switch (PeekToken().klass) {
            case TokenKlass::K_break:
            case TokenKlass::K_continue:
            case TokenKlass::K_discard:
                ConsumeToken();
                if (!ConsumeTokenIf(TokenKlass::Semicolon)) {
                    // emit error
                }
                // TODO: break/continue/discard
                return CreateAstNode<AstBreakStmt>(range);
            case TokenKlass::K_return:
                ConsumeToken();
                if (ConsumeTokenIf(TokenKlass::Semicolon)) {
                    return CreateAstNode<AstReturnStmt>(range);
                }
                else {
                    auto returnValue = ParseExpr();
                    if (!ConsumeTokenIf(TokenKlass::Semicolon)) {
                        // emit error
                    }

                    return CreateAstNode<AstReturnStmt>(range, returnValue);
                }
            default:
                GLSLD_UNREACHABLE();
            }
        }

        auto ParseExprStmt() -> AstStmt*
        {
            auto expr = ParseExpr();
            if (!ConsumeTokenIf(TokenKlass::Semicolon)) {
                // emit error
            }

            return CreateAstNode<AstExprStmt>({}, expr);
        }

        auto ParseStmt() -> AstExpr*
        {
            auto range = PeekToken().range;

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
            case TokenKlass::Semicolon:
                // expression stmt (empty)
                ConsumeToken();
                return CreateAstNode<AstExprStmt>(range, nullptr);
            case TokenKlass::K_break:
            case TokenKlass::K_continue:
            case TokenKlass::K_discard:
            case TokenKlass::K_return:
                // jump stmt
                return ParseJumpStmt();
            default:
                // expression/declaration stmt
                return ParseExprStmt();
            }
        }

        enum class RecoveryMode
        {
            Paren   = static_cast<int>(TokenKlass::RParen),
            Bracket = static_cast<int>(TokenKlass::RBracket),
            Brace   = static_cast<int>(TokenKlass::RBrace),
            Semi    = static_cast<int>(TokenKlass::Semicolon),
        };

        auto RecoverFromError(RecoveryMode mode) -> int
        {
            auto desiredToken = static_cast<TokenKlass>(mode);

            // skip tokens until
            // 1. next ';' in the same or outer scope
            // 2. next '}' that end the current scope
            std::vector<TokenKlass> pendingClose;
            while (PeekToken().klass != TokenKlass::Eof) {
                switch (PeekToken().klass) {
                case TokenKlass::LParen:
                    pendingClose.push_back(TokenKlass::RParen);
                    break;
                case TokenKlass::LBracket:
                    pendingClose.push_back(TokenKlass::RBracket);
                    break;
                case TokenKlass::LBrace:
                    pendingClose.push_back(TokenKlass::RBrace);
                    break;
                case TokenKlass::RParen:
                case TokenKlass::RBracket:
                case TokenKlass::RBrace:
                    if (pendingClose.empty() && PeekToken().klass == desiredToken) {
                        return 1;
                    }
                    break;
                case TokenKlass::Semicolon:
                    if (pendingClose.empty()) {
                        return desiredToken == TokenKlass::Semicolon ? 1 : 0;
                    }
                    break;
                default:
                    break;
                }
            }

            return 0;
        }

        auto TryToken(TokenKlass klass, int lookahead = 0) -> bool
        {
            const auto& tok = PeekToken(lookahead);
            if (tok.klass == klass) {
                return true;
            }
            else {
                return false;
            }
        }
        auto TryToken(TokenKlass klass1, TokenKlass klass2, int lookahead = 0) -> bool
        {
            const auto& tok = PeekToken(lookahead);
            if (tok.klass == klass1 || tok.klass == klass2) {
                return true;
            }
            else {
                return false;
            }
        }
        auto TryToken(TokenKlass klass1, TokenKlass klass2, TokenKlass klass3, int lookahead = 0) -> bool
        {
            const auto& tok = PeekToken(lookahead);
            if (tok.klass == klass1 || tok.klass == klass2 || tok.klass == klass3) {
                return true;
            }
            else {
                return false;
            }
        }

        auto ReportError(size_t tokenIndexBegin, std::string message) -> void
        {
            SyntaxRange range{
                .begin = tokenStream[tokenIndexBegin].range.begin,
                .end   = tokenStream[currentTokIndex].range.end,
            };

            diagManger->ReportError(range, std::move(message));
        }
        auto ReportError(std::string message) -> void
        {
            SyntaxRange range{
                .begin = tokenStream[currentTokIndex].range.begin,
                .end   = tokenStream[currentTokIndex].range.end,
            };

            diagManger->ReportError(range, std::move(message));
        }

        auto GetScannerLocation() -> size_t
        {
            return currentTokIndex;
        }

        auto RestoreScannerLocation(size_t index) -> size_t
        {
            currentTokIndex = index;
        }

        auto PeekToken(size_t lookahead = 0) -> const SyntaxToken&
        {
            auto index = currentTokIndex + lookahead;
            if (index < tokenStream.size()) {
                return tokenStream[index];
            }
            else {
                return tokenStream.back();
            }
        }

        auto ConsumeToken() -> void
        {
            currentTokIndex += 1;
        }

        auto ConsumeTokenAssert(TokenKlass klass) -> void
        {
            GLSLD_ASSERT(PeekToken().klass == klass);
            ConsumeToken();
        }

        // TODO: error recovery
        auto ConsumeTokenIf(TokenKlass klass) -> bool
        {
            if (PeekToken().klass == klass) {
                ConsumeToken();
                return true;
            }
            else {
                return false;
            }
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<SyntaxNode, T>
        auto CreateAstNode(const SyntaxRange& range, Args&&... args) -> T*
        {
            auto result = new T{std::forward<Args>(args)...};
            result->UpdateRange(range);
            return result;
        }

    private:
        size_t currentTokIndex = 0;
        std::vector<SyntaxToken> tokenStream;

        std::vector<AstDecl*> externalDecls;
        std::vector<AstDecl*> variableDecls;
        std::vector<AstDecl*> functionDecls;

        std::shared_ptr<DiagnosticManager> diagManger;
    };

} // namespace glsld
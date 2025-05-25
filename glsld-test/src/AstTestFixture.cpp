#include "CompilerTestFixture.h"

namespace glsld
{

#pragma region Misc Matcher
    auto CompilerTestFixture::NullAst() -> AstMatcher*
    {
        return CreateAstMatcher("NullNode", [](const AstNode* node) -> AstMatchResult {
            return node ? AstMatchResult::Failure(node) : AstMatchResult::Success();
        });
    }

    auto CompilerTestFixture::AnyAst() -> AstMatcher*
    {
        return CreateAstMatcher("AnyNode", [](const AstNode* node) { return AstMatchResult::Success(); });
    }

    auto CompilerTestFixture::AnyInitializer() -> AstMatcher*
    {
        return CreateAstMatcher("AnyInitializer", [](const AstNode* node) {
            return (node && node->Is<AstInitializer>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::AnyExpr() -> AstMatcher*
    {
        return CreateAstMatcher("AnyExpr", [](const AstNode* node) {
            return (node && node->Is<AstExpr>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::AnyStmt() -> AstMatcher*
    {
        return CreateAstMatcher("AnyStmt", [](const AstNode* node) {
            return (node && node->Is<AstStmt>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::AnyDecl() -> AstMatcher*
    {
        return CreateAstMatcher("AnyDecl", [](const AstNode* node) {
            return (node && node->Is<AstDecl>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::AnyQual() -> AstMatcher*
    {
        return CreateAstMatcher("AnyQual", [](const AstNode* node) {
            return (!node || node->Is<AstTypeQualifierSeq>()) ? AstMatchResult::Success()
                                                              : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::AnyQualType() -> AstMatcher*
    {
        return CreateAstMatcher("AnyQualType", [](const AstNode* node) {
            return (node && node->Is<AstQualType>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::QualType(AstMatcher* qualMatcher, AstMatcher* structDeclMatcher,
                                       AstMatcher* arraySpecMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("QualType", [qualMatcher, structDeclMatcher, arraySpecMatcher](const AstNode* node) {
            auto qualType = node ? node->As<AstQualType>() : nullptr;
            if (!qualType) {
                return AstMatchResult::Failure(node);
            }

            if (auto result = qualMatcher->Match(qualType->GetQualifiers()); !result.IsSuccess()) {
                return result;
            }

            if (qualType->GetStructDecl() == nullptr) {
                return AstMatchResult::Failure(node);
            }
            if (auto result = structDeclMatcher->Match(qualType->GetStructDecl()); !result.IsSuccess()) {
                return result;
            }

            if (auto result = arraySpecMatcher->Match(qualType->GetArraySpec()); !result.IsSuccess()) {
                return result;
            }

            return AstMatchResult::Success();
        });
    }
    auto CompilerTestFixture::QualType(AstMatcher* qualMatcher, TokenMatcher* typeNameMatcher,
                                       AstMatcher* arraySpecMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("QualType", [qualMatcher, typeNameMatcher, arraySpecMatcher](const AstNode* node) {
            auto qualType = node ? node->As<AstQualType>() : nullptr;
            if (!qualType) {
                return AstMatchResult::Failure(node);
            }

            if (auto result = qualMatcher->Match(qualType->GetQualifiers()); !result.IsSuccess()) {
                return result;
            }

            if (qualType->GetStructDecl() != nullptr) {
                return AstMatchResult::Failure(node);
            }
            if (!typeNameMatcher->Match(qualType->GetTypeNameTok())) {
                return AstMatchResult::Failure(node);
            }

            if (auto result = arraySpecMatcher->Match(qualType->GetArraySpec()); !result.IsSuccess()) {
                return result;
            }

            return AstMatchResult::Success();
        });
    }
    auto CompilerTestFixture::NamedType(TokenKlass keywordKlass) -> AstMatcher*
    {
        return QualType(AnyQual(), KeywordTok(keywordKlass), NullAst());
    }
    auto CompilerTestFixture::NamedType(StringView name) -> AstMatcher*
    {
        return QualType(AnyQual(), IdTok(name), NullAst());
    }
    auto CompilerTestFixture::StructType(AstMatcher* structDeclMatcher) -> AstMatcher*
    {
        return QualType(AnyQual(), structDeclMatcher, NullAst());
    }
    auto CompilerTestFixture::ArraySpec(std::vector<AstMatcher*> sizeMatchers) -> AstMatcher*
    {
        return CreateAstMatcher("ArraySpec",
                                [indexMatchers = std::move(sizeMatchers)](const AstNode* node) -> AstMatchResult {
                                    auto arraySpec = node ? node->As<AstArraySpec>() : nullptr;
                                    if (!arraySpec || arraySpec->GetSizeList().size() != indexMatchers.size()) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    return MatchAll(arraySpec->GetSizeList(), indexMatchers);
                                });
    }
#pragma endregion

#pragma region Expr Matcher
    auto CompilerTestFixture::InitializerList(std::vector<AstMatcher*> itemMatchers) -> AstMatcher*
    {
        return CreateAstMatcher("InitializerList",
                                [itemMatchers = std::move(itemMatchers)](const AstNode* node) -> AstMatchResult {
                                    auto initList = node ? node->As<AstInitializerList>() : nullptr;
                                    if (!initList || initList->GetItems().size() != itemMatchers.size()) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    return MatchAll(initList->GetItems(), itemMatchers);
                                });
    }
    auto CompilerTestFixture::ErrorExpr() -> AstMatcher*
    {
        return CreateAstMatcher("ErrorExpr", [](const AstNode* node) -> AstMatchResult {
            return (node && node->Is<AstErrorExpr>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::LiteralExpr(ConstValue value) -> AstMatcher*
    {
        return CreateAstMatcher("LiteralExpr", [value = std::move(value)](const AstNode* node) -> AstMatchResult {
            auto expr = node->As<AstLiteralExpr>();
            if (!expr || expr->GetValue() != value) {
                return AstMatchResult::Failure(node);
            }

            return AstMatchResult::Success();
        });
    }
    auto CompilerTestFixture::NameAccessExpr(StringView name) -> AstMatcher*
    {
        return CreateAstMatcher("NameAccessExpr", [name = name.Str()](const AstNode* node) -> AstMatchResult {
            auto expr = node ? node->As<AstNameAccessExpr>() : nullptr;
            if (!expr || expr->GetNameToken().text != name) {
                return AstMatchResult::Failure(node);
            }

            return AstMatchResult::Success();
        });
    }
    auto CompilerTestFixture::FieldAccessExpr(AstMatcher* lhsMatcher, StringView name) -> AstMatcher*
    {
        return CreateAstMatcher("FieldAccessExpr",
                                [lhsMatcher, name = name.Str()](const AstNode* node) -> AstMatchResult {
                                    auto expr = node ? node->As<AstFieldAccessExpr>() : nullptr;
                                    if (!expr || expr->GetNameToken().text != name) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    return MatchAll({{expr->GetBaseExpr(), lhsMatcher}});
                                });
    }
    auto CompilerTestFixture::SwizzleAccessExpr(AstMatcher* lhsMatcher, StringView swizzle) -> AstMatcher*
    {
        return CreateAstMatcher("SwizzleAccessExpr",
                                [lhsMatcher, swizzle = swizzle.Str()](const AstNode* node) -> AstMatchResult {
                                    auto expr = node ? node->As<AstSwizzleAccessExpr>() : nullptr;
                                    if (!expr || expr->GetSwizzleDesc().ToString() != swizzle) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    return MatchAll({{expr->GetBaseExpr(), lhsMatcher}});
                                });
    }
    auto CompilerTestFixture::IndexAccessExpr(AstMatcher* lhsMatcher, AstMatcher* indexMatcher) -> AstMatcher*
    {
        return CreateAstMatcher(
            "IndexAccessExpr",
            [lhsMatcher, indexMatchers = std::move(indexMatcher)](const AstNode* node) -> AstMatchResult {
                auto expr = node ? node->As<AstIndexAccessExpr>() : nullptr;
                if (!expr) {
                    return AstMatchResult::Failure(node);
                }

                return MatchAll({{expr->GetBaseExpr(), lhsMatcher}, {expr->GetIndexExpr(), indexMatchers}});
            });
    }
    auto CompilerTestFixture::UnaryExpr(UnaryOp op, AstMatcher* operandMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("UnaryExpr", [op, operandMatcher](const AstNode* node) -> AstMatchResult {
            auto expr = node ? node->As<AstUnaryExpr>() : nullptr;
            if (!expr || expr->GetOpcode() != op) {
                return AstMatchResult::Failure(node);
            }

            return operandMatcher->Match(expr->GetOperand());
        });
    }
    auto CompilerTestFixture::BinaryExpr(BinaryOp op, AstMatcher* lhsMatcher, AstMatcher* rhsMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("BinaryExpr", [op, lhsMatcher, rhsMatcher](const AstNode* node) -> AstMatchResult {
            auto expr = node ? node->As<AstBinaryExpr>() : nullptr;
            if (!expr || expr->GetOpcode() != op) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll({{expr->GetLhsOperand(), lhsMatcher}, {expr->GetRhsOperand(), rhsMatcher}});
        });
    }
    auto CompilerTestFixture::SelectExpr(AstMatcher* condMatcher, AstMatcher* trueExprMatcher,
                                         AstMatcher* falseExprMatcher) -> AstMatcher*
    {
        return CreateAstMatcher(
            "SelectExpr", [condMatcher, trueExprMatcher, falseExprMatcher](const AstNode* node) -> AstMatchResult {
                auto expr = node ? node->As<AstSelectExpr>() : nullptr;
                if (!expr) {
                    return AstMatchResult::Failure(node);
                }

                return MatchAll({{expr->GetCondition(), condMatcher},
                                 {expr->GetTrueExpr(), trueExprMatcher},
                                 {expr->GetFalseExpr(), falseExprMatcher}});
            });
    }
    auto CompilerTestFixture::ImplicitCastExpr(AstMatcher* matchOperand) -> AstMatcher*
    {
        return CreateAstMatcher("ImplicitCastExpr", [matchOperand](const AstNode* node) -> AstMatchResult {
            auto expr = node ? node->As<AstImplicitCastExpr>() : nullptr;
            if (!expr) {
                return AstMatchResult::Failure(node);
            }
            return matchOperand->Match(expr->GetOperand());
        });
    }
    auto CompilerTestFixture::FunctionCallExpr(TokenMatcher* nameMatcher, std::vector<AstMatcher*> argMatchers)
        -> AstMatcher*
    {
        return CreateAstMatcher(
            "FunctionCallExpr",
            [nameMatcher, argMatchers = std::move(argMatchers)](const AstNode* node) -> AstMatchResult {
                auto expr = node ? node->As<AstFunctionCallExpr>() : nullptr;
                if (!expr || !nameMatcher->Match(expr->GetNameToken()) ||
                    expr->GetArgs().size() != argMatchers.size()) {
                    return AstMatchResult::Failure(node);
                }

                return MatchAll(expr->GetArgs(), argMatchers);
            });
    }
    auto CompilerTestFixture::FunctionCallExpr(StringView name, std::vector<AstMatcher*> argMatchers) -> AstMatcher*
    {
        return FunctionCallExpr(IdTok(name), std::move(argMatchers));
    }
    auto CompilerTestFixture::ConstructorCallExpr(AstMatcher* typeMatcher, std::vector<AstMatcher*> argMatchers)
        -> AstMatcher*
    {
        return CreateAstMatcher(
            "ConstructorCallExpr",
            [typeMatcher, argMatchers = std::move(argMatchers)](const AstNode* node) -> AstMatchResult {
                auto expr = node ? node->As<AstConstructorCallExpr>() : nullptr;
                if (!expr || expr->GetArgs().size() != argMatchers.size()) {
                    return AstMatchResult::Failure(node);
                }
                if (auto typeResult = typeMatcher->Match(expr->GetConstructedType()); !typeResult.IsSuccess()) {
                    return typeResult;
                }

                return MatchAll(expr->GetArgs(), argMatchers);
            });
    }
#pragma endregion

#pragma region Stmt Matcher
    auto CompilerTestFixture::ErrorStmt() -> AstMatcher*
    {
        return CreateAstMatcher("ErrorStmt", [](const AstNode* node) -> AstMatchResult {
            return (node && node->Is<AstErrorStmt>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::EmptyStmt() -> AstMatcher*
    {
        return CreateAstMatcher("EmptyStmt", [](const AstNode* node) -> AstMatchResult {
            return (node && node->Is<AstEmptyStmt>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::CompoundStmt(std::vector<AstMatcher*> stmtMatchers) -> AstMatcher*
    {
        return CreateAstMatcher("CompoundStmt", [stmtMatchers](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstCompoundStmt>() : nullptr;
            if (!stmt || stmt->GetChildren().size() != stmtMatchers.size()) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll(stmt->GetChildren(), stmtMatchers);
        });
    }
    auto CompilerTestFixture::ExprStmt(AstMatcher* exprMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("ExprStmt", [exprMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstExprStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }
            return exprMatcher->Match(stmt->GetExpr());
        });
    }
    auto CompilerTestFixture::DeclStmt(AstMatcher* declMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("DeclStmt", [declMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstDeclStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }
            return declMatcher->Match(stmt->GetDecl());
        });
    }
    auto CompilerTestFixture::IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher) -> AstMatcher*
    {
        return IfStmt(condMatcher, thenStmtMatcher, NullAst());
    }
    auto CompilerTestFixture::IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher, AstMatcher* elseStmtMatcher)
        -> AstMatcher*
    {
        return CreateAstMatcher("IfStmt",
                                [condMatcher, thenStmtMatcher, elseStmtMatcher](const AstNode* node) -> AstMatchResult {
                                    auto stmt = node ? node->As<AstIfStmt>() : nullptr;
                                    if (!stmt) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    return MatchAll({{stmt->GetConditionExpr(), condMatcher},
                                                     {stmt->GetThenStmt(), thenStmtMatcher},
                                                     {stmt->GetElseStmt(), elseStmtMatcher}});
                                });
    }
    auto CompilerTestFixture::ForStmt(AstMatcher* initExprMatcher, AstMatcher* condExprMatcher,
                                      AstMatcher* iterExprMatcher, AstMatcher* bodyMatcher) -> AstMatcher*
    {
        return CreateAstMatcher(
            "ForStmt",
            [initExprMatcher, condExprMatcher, iterExprMatcher, bodyMatcher](const AstNode* node) -> AstMatchResult {
                auto stmt = node ? node->As<AstForStmt>() : nullptr;
                if (!stmt) {
                    return AstMatchResult::Failure(node);
                }

                return MatchAll({{stmt->GetInitStmt(), initExprMatcher},
                                 {stmt->GetConditionExpr(), condExprMatcher},
                                 {stmt->GetIterExpr(), iterExprMatcher},
                                 {stmt->GetBody(), bodyMatcher}});
            });
    }
    auto CompilerTestFixture::WhileStmt(AstMatcher* condMatcher, AstMatcher* bodyMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("WhileStmt", [condMatcher, bodyMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstWhileStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll({{stmt->GetConditionExpr(), condMatcher}, {stmt->GetBody(), bodyMatcher}});
        });
    }
    auto CompilerTestFixture::DoWhileStmt(AstMatcher* bodyMatcher, AstMatcher* condMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("DoWhileStmt", [bodyMatcher, condMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstDoWhileStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll({{stmt->GetBody(), bodyMatcher}, {stmt->GetConditionExpr(), condMatcher}});
        });
    }
    auto CompilerTestFixture::CaseLabelStmt(AstMatcher* exprMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("CaseLabelStmt", [exprMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstLabelStmt>() : nullptr;
            if (!stmt || stmt->GetCaseExpr() == nullptr) {
                return AstMatchResult::Failure(node);
            }

            return exprMatcher->Match(stmt->GetCaseExpr());
        });
    }
    auto CompilerTestFixture::DefaultLabelStmt() -> AstMatcher*
    {
        return CreateAstMatcher("DefaultLabelStmt", [](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstLabelStmt>() : nullptr;
            return stmt && stmt->GetCaseExpr() == nullptr ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::SwitchStmt(AstMatcher* testExprMatcher, AstMatcher* bodyMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("SwitchStmt", [testExprMatcher, bodyMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstSwitchStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll({{stmt->GetTestExpr(), testExprMatcher}, {stmt->GetBody(), bodyMatcher}});
        });
    }
    auto CompilerTestFixture::BreakStmt() -> AstMatcher*
    {
        return CreateAstMatcher("BreakStmt", [](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstJumpStmt>() : nullptr;
            return stmt && stmt->GetJumpType() == JumpType::Break ? AstMatchResult::Success()
                                                                  : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::ContinueStmt() -> AstMatcher*
    {
        return CreateAstMatcher("ContinueStmt", [](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstJumpStmt>() : nullptr;
            return stmt && stmt->GetJumpType() == JumpType::Continue ? AstMatchResult::Success()
                                                                     : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::ReturnStmt() -> AstMatcher*
    {
        return CreateAstMatcher("ReturnStmt", [](const AstNode* node) -> AstMatchResult {
            return (node && node->Is<AstReturnStmt>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::ReturnStmt(AstMatcher* exprMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("ReturnStmt", [exprMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstReturnStmt>() : nullptr;
            if (!stmt || stmt->GetExpr() == nullptr) {
                return AstMatchResult::Failure(node);
            }
            return exprMatcher->Match(stmt->GetExpr());
        });
    }
#pragma endregion

#pragma region Decl Matcher
    auto CompilerTestFixture::ErrorDecl() -> AstMatcher*
    {
        return CreateAstMatcher("ErrorDecl", [](const AstNode* node) -> AstMatchResult {
            return (node && node->Is<AstErrorDecl>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto CompilerTestFixture::EmptyDecl() -> AstMatcher*
    {
        return CreateAstMatcher("EmptyDecl", [](const AstNode* node) -> AstMatchResult {
            if (!node || !node->Is<AstEmptyDecl>()) {
                return AstMatchResult::Failure(node);
            }
            return AstMatchResult::Success();
        });
    }
    auto CompilerTestFixture::PrecisionDecl(AstMatcher* typeMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("PrecisionDecl", [typeMatcher](const AstNode* node) -> AstMatchResult {
            auto decl = node ? node->As<AstPrecisionDecl>() : nullptr;
            if (!decl) {
                return AstMatchResult::Failure(node);
            }

            return typeMatcher->Match(decl->GetType());
        });
    }
    auto CompilerTestFixture::BlockDecl(AstMatcher* qualMatcher, TokenMatcher* blockNameMatcher,
                                        std::vector<AstMatcher*> fieldMatchers) -> AstMatcher*
    {
        return CreateAstMatcher("BlockDecl",
                                [qualMatcher, blockNameMatcher,
                                 fieldMatchers = std::move(fieldMatchers)](const AstNode* node) -> AstMatchResult {
                                    auto decl = node ? node->As<AstInterfaceBlockDecl>() : nullptr;
                                    if (!decl || !blockNameMatcher->Match(decl->GetNameToken()) ||
                                        decl->GetMembers().size() != fieldMatchers.size()) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    return MatchAll(decl->GetMembers(), fieldMatchers);
                                });
    }
    auto CompilerTestFixture::BlockDecl(AstMatcher* qualMatcher, TokenMatcher* blockNameMatcher,
                                        std::vector<AstMatcher*> fieldMatchers, TokenMatcher* instanceNameMatcher,
                                        AstMatcher* instanceArraySpecMatcher) -> AstMatcher*
    {
        return CreateAstMatcher("BlockDecl",
                                [qualMatcher, blockNameMatcher, fieldMatchers = std::move(fieldMatchers),
                                 instanceNameMatcher, instanceArraySpecMatcher](const AstNode* node) -> AstMatchResult {
                                    auto decl = node ? node->As<AstInterfaceBlockDecl>() : nullptr;
                                    if (!decl || !blockNameMatcher->Match(decl->GetNameToken()) ||
                                        decl->GetMembers().size() != fieldMatchers.size()) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    if (decl->GetDeclarator()) {
                                        if (!instanceNameMatcher->Match(decl->GetDeclarator()->nameToken)) {
                                            return AstMatchResult::Failure(node);
                                        }

                                        if (auto result =
                                                instanceArraySpecMatcher->Match(decl->GetDeclarator()->arraySpec);
                                            !result.IsSuccess()) {
                                            return result;
                                        }
                                    }

                                    return MatchAll(decl->GetMembers(), fieldMatchers);
                                });
    }
    auto CompilerTestFixture::BlockFieldDecl(AstMatcher* qualTypeMatcher,
                                             std::vector<DeclaratorMatcher> declaratorMatchers) -> AstMatcher*
    {
        return CreateAstMatcher("BlockFieldDecl",
                                [qualTypeMatcher, declaratorMatchers = std::move(declaratorMatchers)](
                                    const AstNode* node) -> AstMatchResult {
                                    auto decl = node ? node->As<AstBlockFieldDecl>() : nullptr;
                                    if (!decl || decl->GetDeclarators().size() != declaratorMatchers.size()) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    for (size_t i = 0; i < decl->GetDeclarators().size(); ++i) {
                                        const auto& declarator = decl->GetDeclarators()[i];
                                        const auto& matcher    = declaratorMatchers[i];

                                        if (!matcher.nameMatcher->Match(declarator.nameToken)) {
                                            return AstMatchResult::Failure(node);
                                        }

                                        if (auto result = declaratorMatchers[i].Match(decl, declarator);
                                            !result.IsSuccess()) {
                                            return result;
                                        }
                                    }

                                    return AstMatchResult::Success();
                                });
    }
    auto CompilerTestFixture::BlockFieldDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher) -> AstMatcher*
    {
        return BlockFieldDecl(qualTypeMatcher, {{nameMatcher, NullAst(), NullAst()}});
    }
    auto CompilerTestFixture::BlockFieldDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher,
                                             AstMatcher* arraySpecMatcher) -> AstMatcher*
    {
        return BlockFieldDecl(qualTypeMatcher, {{nameMatcher, arraySpecMatcher, NullAst()}});
    }
    auto CompilerTestFixture::VariableDecl(AstMatcher* qualTypeMatcher,
                                           std::vector<DeclaratorMatcher> declaratorMatchers) -> AstMatcher*
    {
        return CreateAstMatcher(
            "VariableDecl",
            [qualTypeMatcher,
             declaratorMatchers = std::move(declaratorMatchers)](const AstNode* node) -> AstMatchResult {
                auto decl = node ? node->As<AstVariableDecl>() : nullptr;
                if (!decl || decl->GetDeclarators().size() != declaratorMatchers.size()) {
                    return AstMatchResult::Failure(node);
                }

                if (auto result = qualTypeMatcher->Match(decl->GetQualType()); !result.IsSuccess()) {
                    return result;
                }

                for (const auto& [declarator, matcher] : std::views::zip(decl->GetDeclarators(), declaratorMatchers)) {
                    if (auto result = matcher.Match(decl, declarator); !result.IsSuccess()) {
                        return result;
                    }
                }

                return AstMatchResult::Success();
            });
    }
    auto CompilerTestFixture::VariableDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher,
                                           AstMatcher* arraySpecMatcher, AstMatcher* initializerMatcher) -> AstMatcher*
    {
        return VariableDecl(qualTypeMatcher, {{nameMatcher, arraySpecMatcher, initializerMatcher}});
    }
    auto CompilerTestFixture::StructFieldDecl(AstMatcher* qualTypeMatcher,
                                              std::vector<DeclaratorMatcher> declaratorMatchers) -> AstMatcher*
    {
        return CreateAstMatcher(
            "StructFieldDecl",
            [qualTypeMatcher,
             declaratorMatchers = std::move(declaratorMatchers)](const AstNode* node) -> AstMatchResult {
                auto decl = node ? node->As<AstStructFieldDecl>() : nullptr;
                if (!decl || decl->GetDeclarators().size() != declaratorMatchers.size()) {
                    return AstMatchResult::Failure(node);
                }

                for (size_t i = 0; i < decl->GetDeclarators().size(); ++i) {
                    const auto& declarator = decl->GetDeclarators()[i];
                    const auto& matcher    = declaratorMatchers[i];

                    if (!matcher.nameMatcher->Match(declarator.nameToken)) {
                        return AstMatchResult::Failure(node);
                    }

                    if (auto result = MatchAll({{declarator.arraySpec, matcher.arraySpecMatcher},
                                                {declarator.initializer, matcher.initializerMatcher}});
                        !result.IsSuccess()) {
                        return result;
                    }
                }

                return AstMatchResult::Success();
            });
    }
    auto CompilerTestFixture::StructFieldDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher,
                                              AstMatcher* arraySpecMatcher) -> AstMatcher*
    {
        return StructFieldDecl(qualTypeMatcher, {{nameMatcher, arraySpecMatcher, NullAst()}});
    }
    auto CompilerTestFixture::StructDecl(TokenMatcher* nameMatcher, std::vector<AstMatcher*> fieldMatchers)
        -> AstMatcher*
    {
        return CreateAstMatcher(
            "StructDecl",
            [nameMatcher, fieldMatchers = std::move(fieldMatchers)](const AstNode* node) -> AstMatchResult {
                auto decl = node ? node->As<AstStructDecl>() : nullptr;
                if (!decl || !nameMatcher->Match(decl->GetNameToken() ? *decl->GetNameToken() : AstSyntaxToken{}) ||
                    decl->GetMembers().size() != fieldMatchers.size()) {
                    return AstMatchResult::Failure(node);
                }

                return MatchAll(decl->GetMembers(), fieldMatchers);
            });
    }
    auto CompilerTestFixture::ParamDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher,
                                        AstMatcher* arraySpecMatcher) -> AstMatcher*
    {
        return CreateAstMatcher(
            "ParamDecl", [qualTypeMatcher, nameMatcher, arraySpecMatcher](const AstNode* node) -> AstMatchResult {
                auto decl = node ? node->As<AstParamDecl>() : nullptr;
                if (!decl ||
                    !nameMatcher->Match(decl->GetDeclarator() ? decl->GetDeclarator()->nameToken : AstSyntaxToken{})) {
                    return AstMatchResult::Failure(node);
                }

                return MatchAll({
                    {decl->GetQualType(), qualTypeMatcher},
                    {decl->GetDeclarator() ? decl->GetDeclarator()->arraySpec : nullptr, arraySpecMatcher},
                });
            });
    }
    auto CompilerTestFixture::FunctionDecl(AstMatcher* returnTypeMatcher, TokenMatcher* nameMatcher,
                                           std::vector<AstMatcher*> paramMatchers, AstMatcher* bodyMatcher)
        -> AstMatcher*
    {
        return CreateAstMatcher("FunctionDecl",
                                [returnTypeMatcher, nameMatcher, paramMatchers = std::move(paramMatchers),
                                 bodyMatcher](const AstNode* node) -> AstMatchResult {
                                    if (!node) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    auto decl = node->As<AstFunctionDecl>();
                                    if (!decl || !nameMatcher->Match(decl->GetNameToken()) ||
                                        decl->GetParams().size() != paramMatchers.size()) {
                                        return AstMatchResult::Failure(node);
                                    }
                                    if (auto paramResult = MatchAll(decl->GetParams(), paramMatchers);
                                        !paramResult.IsSuccess()) {
                                        return paramResult;
                                    }

                                    return MatchAll({
                                        {decl->GetReturnType(), returnTypeMatcher},
                                        {decl->GetBody(), bodyMatcher},
                                    });
                                });
    }
#pragma endregion

    auto CompilerTestFixture::TranslationUnit(std::vector<AstMatcher*> declMatchers) -> AstMatcher*
    {
        return CreateAstMatcher("TranslationUnit",
                                [declMatchers = std::move(declMatchers)](const AstNode* node) -> AstMatchResult {
                                    if (!node) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    auto tu = node->As<AstTranslationUnit>();
                                    if (!tu || tu->GetGlobalDecls().size() != declMatchers.size()) {
                                        return AstMatchResult::Failure(node);
                                    }

                                    return MatchAll(tu->GetGlobalDecls(), declMatchers);
                                });
    }
} // namespace glsld
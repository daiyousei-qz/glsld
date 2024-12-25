#include "AstTestFixture.h"

namespace glsld
{
    auto AstTestFixture::NullAst() -> AstMatcher*
    {
        return CreateMatcher("NullNode", [](const AstNode* node) -> AstMatchResult {
            return node ? AstMatchResult::Failure(node) : AstMatchResult::Success();
        });
    }

    auto AstTestFixture::AnyAst() -> AstMatcher*
    {
        return CreateMatcher("AnyNode", [](const AstNode* node) { return AstMatchResult::Success(); });
    }

    auto AstTestFixture::AnyInitializer() -> AstMatcher*
    {
        return CreateMatcher("AnyInitializer", [](const AstNode* node) {
            return (node && node->Is<AstInitializer>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::AnyExpr() -> AstMatcher*
    {
        return CreateMatcher("AnyExpr", [](const AstNode* node) {
            return (node && node->Is<AstExpr>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::AnyStmt() -> AstMatcher*
    {
        return CreateMatcher("AnyStmt", [](const AstNode* node) {
            return (node && node->Is<AstStmt>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::AnyDecl() -> AstMatcher*
    {
        return CreateMatcher("AnyDecl", [](const AstNode* node) {
            return (node && node->Is<AstDecl>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::AnyQualType() -> AstMatcher*
    {
        return CreateMatcher("AnyQualType", [](const AstNode* node) {
            return (node && node->Is<AstQualType>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::BuiltinType(GlslBuiltinType type) -> AstMatcher*
    {
        return CreateMatcher("BuiltinType", [type](const AstNode* node) -> AstMatchResult {
            auto qualType = node ? node->As<AstQualType>() : nullptr;
            if (!qualType || qualType->GetResolvedType() != Type::GetBuiltinType(type)) {
                return AstMatchResult::Failure(node);
            }

            return AstMatchResult::Success();
        });
    }
    auto AstTestFixture::NamedType(TokenMatcher nameMatcher) -> AstMatcher*
    {
        return CreateMatcher("NamedType",
                             [nameMatcher = std::move(nameMatcher)](const AstNode* node) -> AstMatchResult {
                                 auto qualType = node ? node->As<AstQualType>() : nullptr;
                                 if (!qualType || !nameMatcher.Match(qualType->GetTypeNameTok())) {
                                     return AstMatchResult::Failure(node);
                                 }

                                 return AstMatchResult::Success();
                             });
    }
    auto AstTestFixture::NamedType(StringView name) -> AstMatcher*
    {
        return NamedType(IdTok(name));
    }
    auto AstTestFixture::ArraySpec(std::vector<AstMatcher*> indexMatchers) -> AstMatcher*
    {
        return CreateMatcher("ArraySpec",
                             [indexMatchers = std::move(indexMatchers)](const AstNode* node) -> AstMatchResult {
                                 auto arraySpec = node ? node->As<AstArraySpec>() : nullptr;
                                 if (!arraySpec || arraySpec->GetSizeList().size() != indexMatchers.size()) {
                                     return AstMatchResult::Failure(node);
                                 }

                                 return MatchAll(arraySpec->GetSizeList(), indexMatchers);
                             });
    }
    auto AstTestFixture::InitializerList(std::vector<AstMatcher*> itemMatchers) -> AstMatcher*
    {
        return CreateMatcher("InitializerList",
                             [itemMatchers = std::move(itemMatchers)](const AstNode* node) -> AstMatchResult {
                                 auto initList = node ? node->As<AstInitializerList>() : nullptr;
                                 if (!initList || initList->GetItems().size() != itemMatchers.size()) {
                                     return AstMatchResult::Failure(node);
                                 }

                                 return MatchAll(initList->GetItems(), itemMatchers);
                             });
    }
    auto AstTestFixture::ErrorExpr() -> AstMatcher*
    {
        return CreateMatcher("ErrorExpr", [](const AstNode* node) -> AstMatchResult {
            return (node && node->Is<AstErrorExpr>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::NameAccessExpr(StringView name) -> AstMatcher*
    {
        return CreateMatcher("NameAccessExpr", [name = name.Str()](const AstNode* node) -> AstMatchResult {
            auto expr = node ? node->As<AstNameAccessExpr>() : nullptr;
            if (!expr || expr->GetAccessName().text != name) {
                return AstMatchResult::Failure(node);
            }

            return AstMatchResult::Success();
        });
    }
    auto AstTestFixture::FieldAccessExpr(AstMatcher* lhsMatcher, StringView name) -> AstMatcher*
    {
        return CreateMatcher("FieldAccessExpr", [lhsMatcher, name = name.Str()](const AstNode* node) -> AstMatchResult {
            auto expr = node ? node->As<AstFieldAccessExpr>() : nullptr;
            if (!expr || expr->GetAccessName().text != name) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll({{expr->GetLhsExpr(), lhsMatcher}});
        });
    }
    auto AstTestFixture::SwizzleAccessExpr(AstMatcher* lhsMatcher, StringView swizzle) -> AstMatcher*
    {
        return CreateMatcher("SwizzleAccessExpr",
                             [lhsMatcher, swizzle = swizzle.Str()](const AstNode* node) -> AstMatchResult {
                                 auto expr = node ? node->As<AstSwizzleAccessExpr>() : nullptr;
                                 if (!expr || expr->GetSwizzleDesc().ToString() != swizzle) {
                                     return AstMatchResult::Failure(node);
                                 }

                                 return MatchAll({{expr->GetLhsExpr(), lhsMatcher}});
                             });
    }
    auto AstTestFixture::IndexAccessExpr(AstMatcher* lhsMatcher, std::vector<AstMatcher*> indexMatchers) -> AstMatcher*
    {
        return CreateMatcher(
            "IndexAccessExpr",
            [lhsMatcher, indexMatchers = std::move(indexMatchers)](const AstNode* node) -> AstMatchResult {
                auto expr = node ? node->As<AstIndexAccessExpr>() : nullptr;
                if (!expr || expr->GetIndices()->GetSizeList().size() != indexMatchers.size()) {
                    return AstMatchResult::Failure(node);
                }
                if (auto lhsResult = lhsMatcher->Match(expr->GetBaseExpr()); !lhsResult.IsSuccess()) {
                    return lhsResult;
                }

                return MatchAll(expr->GetIndices()->GetSizeList(), indexMatchers);
            });
    }
    auto AstTestFixture::UnaryExpr(UnaryOp op, AstMatcher* operandMatcher) -> AstMatcher*
    {
        return CreateMatcher("UnaryExpr", [op, operandMatcher](const AstNode* node) -> AstMatchResult {
            auto expr = node ? node->As<AstUnaryExpr>() : nullptr;
            if (!expr || expr->GetOpcode() != op) {
                return AstMatchResult::Failure(node);
            }

            return operandMatcher->Match(expr->GetOperand());
        });
    }
    auto AstTestFixture::BinaryExpr(BinaryOp op, AstMatcher* lhsMatcher, AstMatcher* rhsMatcher) -> AstMatcher*
    {
        return CreateMatcher("BinaryExpr", [op, lhsMatcher, rhsMatcher](const AstNode* node) -> AstMatchResult {
            auto expr = node ? node->As<AstBinaryExpr>() : nullptr;
            if (!expr || expr->GetOpcode() != op) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll({{expr->GetLhsOperand(), lhsMatcher}, {expr->GetRhsOperand(), rhsMatcher}});
        });
    }
    auto AstTestFixture::SelectExpr(AstMatcher* condMatcher, AstMatcher* trueExprMatcher, AstMatcher* falseExprMatcher)
        -> AstMatcher*
    {
        return CreateMatcher("SelectExpr",
                             [condMatcher, trueExprMatcher, falseExprMatcher](const AstNode* node) -> AstMatchResult {
                                 auto expr = node ? node->As<AstSelectExpr>() : nullptr;
                                 if (!expr) {
                                     return AstMatchResult::Failure(node);
                                 }

                                 return MatchAll({{expr->GetCondition(), condMatcher},
                                                  {expr->GetTrueExpr(), trueExprMatcher},
                                                  {expr->GetFalseExpr(), falseExprMatcher}});
                             });
    }
    auto AstTestFixture::ImplicitCastExpr(AstMatcher* matchOperand) -> AstMatcher*
    {
        return CreateMatcher("ImplicitCastExpr", [matchOperand](const AstNode* node) -> AstMatchResult {
            auto expr = node ? node->As<AstImplicitCastExpr>() : nullptr;
            if (!expr) {
                return AstMatchResult::Failure(node);
            }
            return matchOperand->Match(expr->GetOperand());
        });
    }
    auto AstTestFixture::FunctionCallExpr(TokenMatcher nameMatcher, std::vector<AstMatcher*> argMatchers) -> AstMatcher*
    {
        return CreateMatcher("FunctionCallExpr",
                             [nameMatcher = std::move(nameMatcher),
                              argMatchers = std::move(argMatchers)](const AstNode* node) -> AstMatchResult {
                                 auto expr = node ? node->As<AstFunctionCallExpr>() : nullptr;
                                 if (!expr || !nameMatcher.Match(expr->GetFunctionName()) ||
                                     expr->GetArgs().size() != argMatchers.size()) {
                                     return AstMatchResult::Failure(node);
                                 }

                                 return MatchAll(expr->GetArgs(), argMatchers);
                             });
    }
    auto AstTestFixture::FunctionCallExpr(StringView name, std::vector<AstMatcher*> argMatchers) -> AstMatcher*
    {
        return FunctionCallExpr(IdTok(name), std::move(argMatchers));
    }
    auto AstTestFixture::ConstructorCallExpr(AstMatcher* typeMatcher, std::vector<AstMatcher*> argMatchers)
        -> AstMatcher*
    {
        return CreateMatcher(
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
    auto AstTestFixture::ErrorStmt() -> AstMatcher*
    {
        return CreateMatcher("ErrorStmt", [](const AstNode* node) -> AstMatchResult {
            return (node && node->Is<AstErrorStmt>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::EmptyStmt() -> AstMatcher*
    {
        return CreateMatcher("EmptyStmt", [](const AstNode* node) -> AstMatchResult {
            return (node && node->Is<AstEmptyStmt>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::CompoundStmt(std::vector<AstMatcher*> stmtMatchers) -> AstMatcher*
    {
        return CreateMatcher("CompoundStmt", [stmtMatchers](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstCompoundStmt>() : nullptr;
            if (!stmt || stmt->GetChildren().size() != stmtMatchers.size()) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll(stmt->GetChildren(), stmtMatchers);
        });
    }
    auto AstTestFixture::ExprStmt(AstMatcher* exprMatcher) -> AstMatcher*
    {
        return CreateMatcher("ExprStmt", [exprMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstExprStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }
            return exprMatcher->Match(stmt->GetExpr());
        });
    }
    auto AstTestFixture::DeclStmt(AstMatcher* declMatcher) -> AstMatcher*
    {
        return CreateMatcher("DeclStmt", [declMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstDeclStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }
            return declMatcher->Match(stmt->GetDecl());
        });
    }
    auto AstTestFixture::IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher) -> AstMatcher*
    {
        return IfStmt(condMatcher, thenStmtMatcher, NullAst());
    }
    auto AstTestFixture::IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher, AstMatcher* elseStmtMatcher)
        -> AstMatcher*
    {
        return CreateMatcher("IfStmt",
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
    auto AstTestFixture::ForStmt(AstMatcher* initExprMatcher, AstMatcher* condExprMatcher, AstMatcher* iterExprMatcher,
                                 AstMatcher* bodyMatcher) -> AstMatcher*
    {
        return CreateMatcher(
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
    auto AstTestFixture::WhileStmt(AstMatcher* condMatcher, AstMatcher* bodyMatcher) -> AstMatcher*
    {
        return CreateMatcher("WhileStmt", [condMatcher, bodyMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstWhileStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll({{stmt->GetConditionExpr(), condMatcher}, {stmt->GetBody(), bodyMatcher}});
        });
    }
    auto AstTestFixture::DoWhileStmt(AstMatcher* bodyMatcher, AstMatcher* condMatcher) -> AstMatcher*
    {
        return CreateMatcher("DoWhileStmt", [bodyMatcher, condMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstDoWhileStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll({{stmt->GetBody(), bodyMatcher}, {stmt->GetConditionExpr(), condMatcher}});
        });
    }
    auto AstTestFixture::CaseLabelStmt(AstMatcher* exprMatcher) -> AstMatcher*
    {
        return CreateMatcher("CaseLabelStmt", [exprMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstLabelStmt>() : nullptr;
            if (!stmt || stmt->GetCaseExpr() == nullptr) {
                return AstMatchResult::Failure(node);
            }

            return exprMatcher->Match(stmt->GetCaseExpr());
        });
    }
    auto AstTestFixture::DefaultLabelStmt() -> AstMatcher*
    {
        return CreateMatcher("DefaultLabelStmt", [](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstLabelStmt>() : nullptr;
            return stmt && stmt->GetCaseExpr() == nullptr ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::SwitchStmt(AstMatcher* testExprMatcher, AstMatcher* bodyMatcher) -> AstMatcher*
    {
        return CreateMatcher("SwitchStmt", [testExprMatcher, bodyMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstSwitchStmt>() : nullptr;
            if (!stmt) {
                return AstMatchResult::Failure(node);
            }

            return MatchAll({{stmt->GetTestExpr(), testExprMatcher}, {stmt->GetBody(), bodyMatcher}});
        });
    }
    auto AstTestFixture::BreakStmt() -> AstMatcher*
    {
        return CreateMatcher("BreakStmt", [](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstJumpStmt>() : nullptr;
            return stmt && stmt->GetJumpType() == JumpType::Break ? AstMatchResult::Success()
                                                                  : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::ContinueStmt() -> AstMatcher*
    {
        return CreateMatcher("ContinueStmt", [](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstJumpStmt>() : nullptr;
            return stmt && stmt->GetJumpType() == JumpType::Continue ? AstMatchResult::Success()
                                                                     : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::ReturnStmt() -> AstMatcher*
    {
        return CreateMatcher("ReturnStmt", [](const AstNode* node) -> AstMatchResult {
            return (node && node->Is<AstReturnStmt>()) ? AstMatchResult::Success() : AstMatchResult::Failure(node);
        });
    }
    auto AstTestFixture::ReturnStmt(AstMatcher* exprMatcher) -> AstMatcher*
    {
        return CreateMatcher("ReturnStmt", [exprMatcher](const AstNode* node) -> AstMatchResult {
            auto stmt = node ? node->As<AstReturnStmt>() : nullptr;
            if (!stmt || stmt->GetExpr() == nullptr) {
                return AstMatchResult::Failure(node);
            }
            return exprMatcher->Match(stmt->GetExpr());
        });
    }
} // namespace glsld
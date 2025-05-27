#include "CompilerTestFixture.h"

namespace glsld
{

    class AstMatchPipeline
    {
    private:
        const AstNode* node   = nullptr;
        AstMatchResult result = AstMatchResult::Success();

    public:
        AstMatchPipeline(const AstNode* node) : node(node)
        {
        }

        auto Try(bool test, StringView errorMessage) -> AstMatchPipeline&
        {
            if (result.IsSuccess() && !test) {
                result = AstMatchResult::Failure(node, "{}", errorMessage);
            }

            return *this;
        }

        template <typename T, typename U>
        auto TryEqual(const T& expected, const U& actual, fmt::format_string<const T&, const U&> fmtError)
            -> AstMatchPipeline&
        {
            if (result.IsSuccess() && expected != actual) {
                result = AstMatchResult::Failure(node, std::move(fmtError), expected, actual);
            }

            return *this;
        }

        auto TryMatchToken(AstSyntaxToken token, TokenMatcher* matcher) -> AstMatchPipeline&
        {
            if (result.IsSuccess() && !matcher->Match(token)) {
                result = AstMatchResult::Failure(node, "Unexpected token, expecting {}, got {}", matcher->Describe(),
                                                 token.text.StrView());
            }

            return *this;
        }

        auto TryMatch(const AstNode* child, AstMatcher* matcher) -> AstMatchPipeline&
        {
            if (result.IsSuccess()) {
                result = matcher->Match(child);
            }

            return *this;
        }

        template <std::derived_from<AstNode> AstNodeType>
        auto TryMatchAll(ArrayView<const AstNodeType*> children, ArrayView<AstMatcher*> matchers) -> AstMatchPipeline&
        {
            if (result.IsSuccess()) {
                if (children.size() != matchers.size()) {
                    result = AstMatchResult::Failure(node, "Unexpected child count: expected {}, got {}",
                                                     matchers.size(), children.size());
                    return *this;
                }

                for (const auto& [child, matcher] : std::views::zip(children, matchers)) {
                    result = matcher->Match(child);
                    if (!result.IsSuccess()) {
                        return *this;
                    }
                }
            }

            return *this;
        }

        auto TryMatchDeclarators(ArrayView<Declarator> declarators, ArrayView<DeclaratorMatcher> declaratorMatchers)
            -> AstMatchPipeline&
        {
            if (result.IsSuccess()) {
                if (declarators.size() != declaratorMatchers.size()) {
                    result = AstMatchResult::Failure(node, "Unexpected declarator count: expected {}, got {}",
                                                     declaratorMatchers.size(), declarators.size());
                    return *this;
                }

                for (const auto& [declarator, matcher] : std::views::zip(declarators, declaratorMatchers)) {
                    result = matcher.Match(node->As<AstDecl>(), declarator);
                    if (!result.IsSuccess()) {
                        return *this;
                    }
                }
            }

            return *this;
        }

        auto Finish() -> AstMatchResult
        {
            return std::move(result);
        }
    };

#pragma region Misc Matcher
    auto CompilerTestFixture::NullAst() -> AstMatcher*
    {
        return CreateAstMatcher(__func__, [](const AstNode* node) -> AstMatchResult {
            return node ? AstMatchResult::FailWithUnexpectedAstNode(node, "null") : AstMatchResult::Success();
        });
    }

    auto CompilerTestFixture::AnyAst() -> AstMatcher*
    {
        return CreateAstMatcher(__func__, [](const AstNode* node) { return AstMatchResult::Success(); });
    }

    auto CompilerTestFixture::AnyInitializer() -> AstMatcher*
    {
        return CreateAstMatcher(__func__, [](const AstNode* node) {
            return (node && node->Is<AstInitializer>())
                       ? AstMatchResult::Success()
                       : AstMatchResult::FailWithUnexpectedAstNode(node, AstNodeTrait<AstInitializer>::name);
        });
    }
    auto CompilerTestFixture::AnyExpr() -> AstMatcher*
    {
        return CreateAstMatcher(__func__, [](const AstNode* node) {
            return (node && node->Is<AstExpr>())
                       ? AstMatchResult::Success()
                       : AstMatchResult::FailWithUnexpectedAstNode(node, AstNodeTrait<AstExpr>::name);
        });
    }
    auto CompilerTestFixture::AnyStmt() -> AstMatcher*
    {
        return CreateAstMatcher(__func__, [](const AstNode* node) {
            return (node && node->Is<AstStmt>())
                       ? AstMatchResult::Success()
                       : AstMatchResult::FailWithUnexpectedAstNode(node, AstNodeTrait<AstStmt>::name);
        });
    }
    auto CompilerTestFixture::AnyDecl() -> AstMatcher*
    {
        return CreateAstMatcher(__func__, [](const AstNode* node) {
            return (node && node->Is<AstDecl>())
                       ? AstMatchResult::Success()
                       : AstMatchResult::FailWithUnexpectedAstNode(node, AstNodeTrait<AstDecl>::name);
        });
    }
    auto CompilerTestFixture::AnyQual() -> AstMatcher*
    {
        return CreateAstMatcher(__func__, [](const AstNode* node) {
            return (!node || node->Is<AstTypeQualifierSeq>())
                       ? AstMatchResult::Success()
                       : AstMatchResult::FailWithUnexpectedAstNode(node, AstNodeTrait<AstTypeQualifierSeq>::name);
        });
    }
    auto CompilerTestFixture::AnyQualType() -> AstMatcher*
    {
        return CreateAstMatcher(__func__, [](const AstNode* node) {
            return (node && node->Is<AstQualType>())
                       ? AstMatchResult::Success()
                       : AstMatchResult::FailWithUnexpectedAstNode(node, AstNodeTrait<AstQualType>::name);
        });
    }
    auto CompilerTestFixture::NamedQual(std::vector<TokenKlass> keyword) -> AstMatcher*
    {
        auto expectedQualGroup = QualifierGroup{};
        for (auto k : keyword) {
            switch (k) {
            case TokenKlass::K_highp:
                expectedQualGroup.qHighp = true;
                break;
            case TokenKlass::K_mediump:
                expectedQualGroup.qMediump = true;
                break;
            case TokenKlass::K_lowp:
                expectedQualGroup.qLowp = true;
                break;

            case TokenKlass::K_const:
                expectedQualGroup.qConst = true;
                break;
            case TokenKlass::K_in:
                expectedQualGroup.qIn = true;
                break;
            case TokenKlass::K_out:
                expectedQualGroup.qOut = true;
                break;
            case TokenKlass::K_inout:
                expectedQualGroup.qInout = true;
                break;
            case TokenKlass::K_attribute:
                expectedQualGroup.qAttribute = true;
                break;
            case TokenKlass::K_uniform:
                expectedQualGroup.qUniform = true;
                break;
            case TokenKlass::K_varying:
                expectedQualGroup.qVarying = true;
                break;
            case TokenKlass::K_buffer:
                expectedQualGroup.qBuffer = true;
                break;
            case TokenKlass::K_shared:
                expectedQualGroup.qShared = true;
                break;

            case TokenKlass::K_centroid:
                expectedQualGroup.qCentroid = true;
                break;
            case TokenKlass::K_sample:
                expectedQualGroup.qSample = true;
                break;
            case TokenKlass::K_patch:
                expectedQualGroup.qPatch = true;
                break;

            case TokenKlass::K_smooth:
                expectedQualGroup.qSmooth = true;
                break;
            case TokenKlass::K_flat:
                expectedQualGroup.qFlat = true;
                break;
            case TokenKlass::K_noperspective:
                expectedQualGroup.qNoperspective = true;
                break;

            case TokenKlass::K_invariant:
                expectedQualGroup.qInvariant = true;
                break;

            case TokenKlass::K_precise:
                expectedQualGroup.qPrecise = true;
                break;

            case TokenKlass::K_coherent:
                expectedQualGroup.qCoherent = true;
                break;
            case TokenKlass::K_volatile:
                expectedQualGroup.qVolatile = true;
                break;
            case TokenKlass::K_restrict:
                expectedQualGroup.qRestrict = true;
                break;
            case TokenKlass::K_readonly:
                expectedQualGroup.qReadonly = true;
                break;
            case TokenKlass::K_writeonly:
                expectedQualGroup.qWriteonly = true;
                break;

            case TokenKlass::K_rayPayloadNV:
            case TokenKlass::K_rayPayloadEXT:
                expectedQualGroup.qRayPayloadEXT = true;
                break;
            case TokenKlass::K_rayPayloadInNV:
            case TokenKlass::K_rayPayloadInEXT:
                expectedQualGroup.qRayPayloadInEXT = true;
                break;
            case TokenKlass::K_hitAttributeNV:
            case TokenKlass::K_hitAttributeEXT:
                expectedQualGroup.qHitAttributeEXT = true;
                break;
            case TokenKlass::K_callableDataNV:
            case TokenKlass::K_callableDataEXT:
                expectedQualGroup.qCallableDataEXT = true;
                break;
            case TokenKlass::K_callableDataInNV:
            case TokenKlass::K_callableDataInEXT:
                expectedQualGroup.qCallableDataInEXT = true;
                break;
            case TokenKlass::K_perprimitiveNV:
            case TokenKlass::K_perprimitiveEXT:
                expectedQualGroup.qPerprimitiveNV = true;
                break;
            case TokenKlass::K_perviewNV:
                expectedQualGroup.qPerviewNV = true;
                break;
            case TokenKlass::K_taskNV:
                expectedQualGroup.qTaskNV = true;
                break;
            case TokenKlass::K_taskPayloadSharedEXT:
                expectedQualGroup.qTaskPayloadSharedEXT = true;
                break;
            default:
                GLSLD_ASSERT(false && "Unexpected token");
                break;
            }
        }

        return CreateAstMatcher<AstTypeQualifierSeq>(
            __func__, [expectedQualGroup](const AstTypeQualifierSeq* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryEqual(expectedQualGroup, node->GetQualGroup(), "Unexpected qualifiers: expected {}, got {}")
                    .Finish();
            });
    }
    auto CompilerTestFixture::QualType(AstMatcher* qualMatcher, AstMatcher* structDeclMatcher,
                                       AstMatcher* arraySpecMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstQualType>(
            __func__, [qualMatcher, structDeclMatcher, arraySpecMatcher](const AstQualType* node) {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetQualifiers(), qualMatcher)
                    .Try(node->GetStructDecl() != nullptr, "Expecting qual-type with struct declaration, got null")
                    .TryMatch(node->GetStructDecl(), structDeclMatcher)
                    .TryMatch(node->GetArraySpec(), arraySpecMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::QualType(AstMatcher* qualMatcher, TokenMatcher* typeNameMatcher,
                                       AstMatcher* arraySpecMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstQualType>(__func__, [qualMatcher, typeNameMatcher,
                                                        arraySpecMatcher](const AstQualType* node) {
            return AstMatchPipeline{node}
                .TryMatch(node->GetQualifiers(), qualMatcher)
                .Try(node->GetStructDecl() == nullptr, "Expecting qual-type with no struct declaration, but got one")
                .TryMatchToken(node->GetTypeNameTok(), typeNameMatcher)
                .TryMatch(node->GetArraySpec(), arraySpecMatcher)
                .Finish();
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
        return CreateAstMatcher<AstArraySpec>(
            __func__, [indexMatchers = std::move(sizeMatchers)](const AstArraySpec* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryEqual(indexMatchers.size(), node->GetSizeList().size(),
                              "Unexpected array spec dimension: expected {}, got {}")
                    .TryMatchAll(node->GetSizeList(), indexMatchers)
                    .Finish();
            });
    }
#pragma endregion

#pragma region Expr Matcher
    auto CompilerTestFixture::InitializerList(std::vector<AstMatcher*> itemMatchers) -> AstMatcher*
    {
        return CreateAstMatcher<AstInitializerList>(
            __func__, [itemMatchers = std::move(itemMatchers)](const AstInitializerList* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryEqual(itemMatchers.size(), node->GetItems().size(),
                              "Unexpected initializer list size: expected {}, got {}")
                    .TryMatchAll(node->GetItems(), itemMatchers)
                    .Finish();
            });
    }
    auto CompilerTestFixture::ErrorExpr() -> AstMatcher*
    {
        return CreateAstMatcher<AstErrorExpr>(
            __func__, [](const AstErrorExpr* node) -> AstMatchResult { return AstMatchResult::Success(); });
    }
    auto CompilerTestFixture::LiteralExpr(ConstValue value) -> AstMatcher*
    {
        return CreateAstMatcher<AstLiteralExpr>(
            __func__, [value = std::move(value)](const AstLiteralExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryEqual(value, node->GetValue(), "Unexpected literal value: expected {}, got {}")
                    .Finish();
            });
    }
    auto CompilerTestFixture::NameAccessExpr(StringView name) -> AstMatcher*
    {
        return CreateAstMatcher<AstNameAccessExpr>(
            __func__, [name = name.Str()](const AstNameAccessExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryEqual(name, node->GetNameToken().text.StrView(), "Unexpected name access: expected {}, got {}")
                    .Finish();
            });
    }
    auto CompilerTestFixture::FieldAccessExpr(AstMatcher* lhsMatcher, StringView name) -> AstMatcher*
    {
        return CreateAstMatcher<AstFieldAccessExpr>(
            __func__, [lhsMatcher, name = name.Str()](const AstFieldAccessExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryEqual(name, node->GetNameToken().text.StrView(), "Unexpected field access: expected {}, got {}")
                    .TryMatch(node->GetBaseExpr(), lhsMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::SwizzleAccessExpr(AstMatcher* lhsMatcher, StringView swizzle) -> AstMatcher*
    {
        return CreateAstMatcher<AstSwizzleAccessExpr>(
            __func__, [lhsMatcher, swizzle = swizzle.Str()](const AstSwizzleAccessExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryEqual(swizzle, node->GetSwizzleDesc().ToString(),
                              "Unexpected swizzle access: expected {}, got {}")
                    .TryMatch(node->GetBaseExpr(), lhsMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::IndexAccessExpr(AstMatcher* lhsMatcher, AstMatcher* indexMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstIndexAccessExpr>(
            __func__, [lhsMatcher, indexMatcher](const AstIndexAccessExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetBaseExpr(), lhsMatcher)
                    .TryMatch(node->GetIndexExpr(), indexMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::UnaryExpr(UnaryOp op, AstMatcher* operandMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstUnaryExpr>(__func__,
                                              [op, operandMatcher](const AstUnaryExpr* node) -> AstMatchResult {
                                                  return AstMatchPipeline{node}
                                                      .TryEqual(UnaryOpToString(op), UnaryOpToString(node->GetOpcode()),
                                                                "Unexpected unary operator: expected {}, got {}")
                                                      .TryMatch(node->GetOperand(), operandMatcher)
                                                      .Finish();
                                              });
    }
    auto CompilerTestFixture::BinaryExpr(BinaryOp op, AstMatcher* lhsMatcher, AstMatcher* rhsMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstBinaryExpr>(
            __func__, [op, lhsMatcher, rhsMatcher](const AstBinaryExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryEqual(BinaryOpToString(op), BinaryOpToString(node->GetOpcode()),
                              "Unexpected binary operator: expected {}, got {}")
                    .TryMatch(node->GetLhsOperand(), lhsMatcher)
                    .TryMatch(node->GetRhsOperand(), rhsMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::SelectExpr(AstMatcher* condMatcher, AstMatcher* trueExprMatcher,
                                         AstMatcher* falseExprMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstSelectExpr>(
            __func__, [condMatcher, trueExprMatcher, falseExprMatcher](const AstSelectExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetCondition(), condMatcher)
                    .TryMatch(node->GetTrueExpr(), trueExprMatcher)
                    .TryMatch(node->GetFalseExpr(), falseExprMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::ImplicitCastExpr(AstMatcher* operandMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstImplicitCastExpr>(
            __func__, [operandMatcher](const AstImplicitCastExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}.TryMatch(node->GetOperand(), operandMatcher).Finish();
            });
    }
    auto CompilerTestFixture::FunctionCallExpr(TokenMatcher* nameMatcher, std::vector<AstMatcher*> argMatchers)
        -> AstMatcher*
    {
        return CreateAstMatcher<AstFunctionCallExpr>(
            __func__,
            [nameMatcher, argMatchers = std::move(argMatchers)](const AstFunctionCallExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatchToken(node->GetNameToken(), nameMatcher)
                    .TryEqual(argMatchers.size(), node->GetArgs().size(),
                              "Unexpected function call argument count: expected {}, got {}")
                    .TryMatchAll(node->GetArgs(), argMatchers)
                    .Finish();
            });
    }
    auto CompilerTestFixture::FunctionCallExpr(StringView name, std::vector<AstMatcher*> argMatchers) -> AstMatcher*
    {
        return FunctionCallExpr(IdTok(name), std::move(argMatchers));
    }
    auto CompilerTestFixture::ConstructorCallExpr(AstMatcher* typeMatcher, std::vector<AstMatcher*> argMatchers)
        -> AstMatcher*
    {
        return CreateAstMatcher<AstConstructorCallExpr>(
            __func__,
            [typeMatcher, argMatchers = std::move(argMatchers)](const AstConstructorCallExpr* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetConstructedType(), typeMatcher)
                    .TryEqual(argMatchers.size(), node->GetArgs().size(),
                              "Unexpected constructor call argument count: expected {}, got {}")
                    .TryMatchAll(node->GetArgs(), argMatchers)
                    .Finish();
            });
    }
#pragma endregion

#pragma region Stmt Matcher
    auto CompilerTestFixture::ErrorStmt() -> AstMatcher*
    {
        return CreateAstMatcher<AstErrorStmt>(
            __func__, [](const AstErrorStmt* node) -> AstMatchResult { return AstMatchResult::Success(); });
    }
    auto CompilerTestFixture::EmptyStmt() -> AstMatcher*
    {
        return CreateAstMatcher<AstEmptyStmt>(
            __func__, [](const AstEmptyStmt* node) -> AstMatchResult { return AstMatchResult::Success(); });
    }
    auto CompilerTestFixture::CompoundStmt(std::vector<AstMatcher*> stmtMatchers) -> AstMatcher*
    {
        return CreateAstMatcher<AstCompoundStmt>(__func__,
                                                 [stmtMatchers](const AstCompoundStmt* node) -> AstMatchResult {
                                                     return AstMatchPipeline{node}
                                                         .TryEqual(stmtMatchers.size(), node->GetChildren().size(),
                                                                   "Unexpected statement count: expected {}, got {}")
                                                         .TryMatchAll(node->GetChildren(), stmtMatchers)
                                                         .Finish();
                                                 });
    }
    auto CompilerTestFixture::ExprStmt(AstMatcher* exprMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstExprStmt>(__func__, [exprMatcher](const AstExprStmt* node) -> AstMatchResult {
            return AstMatchPipeline{node}.TryMatch(node->GetExpr(), exprMatcher).Finish();
        });
    }
    auto CompilerTestFixture::DeclStmt(AstMatcher* declMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstDeclStmt>(__func__, [declMatcher](const AstDeclStmt* node) -> AstMatchResult {
            return AstMatchPipeline{node}.TryMatch(node->GetDecl(), declMatcher).Finish();
        });
    }
    auto CompilerTestFixture::IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher, AstMatcher* elseStmtMatcher)
        -> AstMatcher*
    {
        return CreateAstMatcher<AstIfStmt>(
            __func__, [condMatcher, thenStmtMatcher, elseStmtMatcher](const AstIfStmt* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetConditionExpr(), condMatcher)
                    .TryMatch(node->GetThenStmt(), thenStmtMatcher)
                    .TryMatch(node->GetElseStmt(), elseStmtMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher) -> AstMatcher*
    {
        return IfStmt(condMatcher, thenStmtMatcher, NullAst());
    }
    auto CompilerTestFixture::ForStmt(AstMatcher* initExprMatcher, AstMatcher* condExprMatcher,
                                      AstMatcher* iterExprMatcher, AstMatcher* bodyMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstForStmt>(
            __func__,
            [initExprMatcher, condExprMatcher, iterExprMatcher, bodyMatcher](const AstForStmt* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetInitStmt(), initExprMatcher)
                    .TryMatch(node->GetConditionExpr(), condExprMatcher)
                    .TryMatch(node->GetIterExpr(), iterExprMatcher)
                    .TryMatch(node->GetBody(), bodyMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::WhileStmt(AstMatcher* condMatcher, AstMatcher* bodyMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstWhileStmt>(__func__,
                                              [condMatcher, bodyMatcher](const AstWhileStmt* node) -> AstMatchResult {
                                                  return AstMatchPipeline{node}
                                                      .TryMatch(node->GetConditionExpr(), condMatcher)
                                                      .TryMatch(node->GetBody(), bodyMatcher)
                                                      .Finish();
                                              });
    }
    auto CompilerTestFixture::DoWhileStmt(AstMatcher* bodyMatcher, AstMatcher* condMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstDoWhileStmt>(
            __func__, [bodyMatcher, condMatcher](const AstDoWhileStmt* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetBody(), bodyMatcher)
                    .TryMatch(node->GetConditionExpr(), condMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::CaseLabelStmt(AstMatcher* exprMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstLabelStmt>(__func__, [exprMatcher](const AstLabelStmt* node) -> AstMatchResult {
            return AstMatchPipeline{node}.TryMatch(node->GetCaseExpr(), exprMatcher).Finish();
        });
    }
    auto CompilerTestFixture::DefaultLabelStmt() -> AstMatcher*
    {
        return CaseLabelStmt(NullAst());
    }
    auto CompilerTestFixture::SwitchStmt(AstMatcher* testExprMatcher, AstMatcher* bodyMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstSwitchStmt>(
            __func__, [testExprMatcher, bodyMatcher](const AstSwitchStmt* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetTestExpr(), testExprMatcher)
                    .TryMatch(node->GetBody(), bodyMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::BreakStmt() -> AstMatcher*
    {
        return CreateAstMatcher<AstJumpStmt>(__func__, [](const AstJumpStmt* node) -> AstMatchResult {
            return AstMatchPipeline{node}.Try(node->GetJumpType() == JumpType::Break, "Unexpected jump type").Finish();
        });
    }
    auto CompilerTestFixture::ContinueStmt() -> AstMatcher*
    {
        return CreateAstMatcher<AstJumpStmt>(__func__, [](const AstJumpStmt* node) -> AstMatchResult {
            return AstMatchPipeline{node}
                .Try(node->GetJumpType() == JumpType::Continue, "Unexpected jump type")
                .Finish();
        });
    }
    auto CompilerTestFixture::DiscardStmt() -> AstMatcher*
    {
        return CreateAstMatcher<AstJumpStmt>(__func__, [](const AstJumpStmt* node) -> AstMatchResult {
            return AstMatchPipeline{node}
                .Try(node->GetJumpType() == JumpType::Discard, "Unexpected jump type")
                .Finish();
        });
    }
    auto CompilerTestFixture::ReturnStmt(AstMatcher* exprMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstReturnStmt>(__func__, [exprMatcher](const AstReturnStmt* node) -> AstMatchResult {
            return AstMatchPipeline{node}.TryMatch(node->GetExpr(), exprMatcher).Finish();
        });
    }
    auto CompilerTestFixture::ReturnStmt() -> AstMatcher*
    {
        return ReturnStmt(NullAst());
    }
#pragma endregion

#pragma region Decl Matcher
    auto CompilerTestFixture::ErrorDecl() -> AstMatcher*
    {
        return CreateAstMatcher<AstErrorDecl>(
            __func__, [](const AstErrorDecl* node) -> AstMatchResult { return AstMatchResult::Success(); });
    }
    auto CompilerTestFixture::EmptyDecl() -> AstMatcher*
    {
        return CreateAstMatcher<AstEmptyDecl>(
            __func__, [](const AstEmptyDecl* node) -> AstMatchResult { return AstMatchResult::Success(); });
    }
    auto CompilerTestFixture::PrecisionDecl(AstMatcher* typeMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstPrecisionDecl>(
            __func__, [typeMatcher](const AstPrecisionDecl* node) -> AstMatchResult {
                return AstMatchPipeline{node}.TryMatch(node->GetType(), typeMatcher).Finish();
            });
    }
    auto CompilerTestFixture::BlockDecl(AstMatcher* qualMatcher, TokenMatcher* blockNameMatcher,
                                        std::vector<AstMatcher*> fieldMatchers) -> AstMatcher*
    {
        return CreateAstMatcher<AstInterfaceBlockDecl>(
            __func__,
            [qualMatcher, blockNameMatcher,
             fieldMatchers = std::move(fieldMatchers)](const AstInterfaceBlockDecl* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetQuals(), qualMatcher)
                    .TryMatchToken(node->GetNameToken(), blockNameMatcher)
                    .TryEqual(fieldMatchers.size(), node->GetMembers().size(),
                              "Unexpected block member count: expected {}, got {}")
                    .TryMatchAll(node->GetMembers(), fieldMatchers)
                    // TODO: match instance
                    .Finish();
            });
    }
    auto CompilerTestFixture::BlockDecl(AstMatcher* qualMatcher, TokenMatcher* blockNameMatcher,
                                        std::vector<AstMatcher*> fieldMatchers, TokenMatcher* instanceNameMatcher,
                                        AstMatcher* instanceArraySpecMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstInterfaceBlockDecl>(
            __func__,
            [qualMatcher, blockNameMatcher, fieldMatchers = std::move(fieldMatchers), instanceNameMatcher,
             instanceArraySpecMatcher](const AstInterfaceBlockDecl* node) -> AstMatchResult {
                // FIXME: use AstMatchPipeline
                if (node->GetDeclarator()) {
                    if (!instanceNameMatcher->Match(node->GetDeclarator()->nameToken)) {
                        return AstMatchResult::FailWithUnknown(node);
                    }

                    if (auto result = instanceArraySpecMatcher->Match(node->GetDeclarator()->arraySpec);
                        !result.IsSuccess()) {
                        return result;
                    }
                }

                return AstMatchPipeline{node}
                    .TryMatch(node->GetQuals(), qualMatcher)
                    .TryMatchToken(node->GetNameToken(), blockNameMatcher)
                    .TryEqual(fieldMatchers.size(), node->GetMembers().size(),
                              "Unexpected block member count: expected {}, got {}")
                    .TryMatchAll(node->GetMembers(), fieldMatchers)
                    // TODO: match instance
                    .Finish();
            });
    }
    auto CompilerTestFixture::BlockFieldDecl(AstMatcher* qualTypeMatcher,
                                             std::vector<DeclaratorMatcher> declaratorMatchers) -> AstMatcher*
    {
        return CreateAstMatcher<AstBlockFieldDecl>(
            __func__,
            [qualTypeMatcher,
             declaratorMatchers = std::move(declaratorMatchers)](const AstBlockFieldDecl* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetQualType(), qualTypeMatcher)
                    .TryMatchDeclarators(node->GetDeclarators(), declaratorMatchers)
                    .Finish();
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
        return CreateAstMatcher<AstVariableDecl>(__func__,
                                                 [qualTypeMatcher, declaratorMatchers = std::move(declaratorMatchers)](
                                                     const AstVariableDecl* node) -> AstMatchResult {
                                                     return AstMatchPipeline{node}
                                                         .TryMatch(node->GetQualType(), qualTypeMatcher)
                                                         .TryMatchDeclarators(node->GetDeclarators(),
                                                                              declaratorMatchers)
                                                         .Finish();
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
        return CreateAstMatcher<AstStructFieldDecl>(
            __func__,
            [qualTypeMatcher,
             declaratorMatchers = std::move(declaratorMatchers)](const AstStructFieldDecl* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetQualType(), qualTypeMatcher)
                    .TryMatchDeclarators(node->GetDeclarators(), declaratorMatchers)
                    .Finish();
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
        return CreateAstMatcher<AstStructDecl>(
            __func__,
            [nameMatcher, fieldMatchers = std::move(fieldMatchers)](const AstStructDecl* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatchToken(node->GetNameToken() ? *node->GetNameToken() : AstSyntaxToken{}, nameMatcher)
                    .TryEqual(fieldMatchers.size(), node->GetMembers().size(),
                              "Unexpected struct member count: expected {}, got {}")
                    .TryMatchAll(node->GetMembers(), fieldMatchers)
                    .Finish();
            });
    }
    auto CompilerTestFixture::ParamDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher,
                                        AstMatcher* arraySpecMatcher) -> AstMatcher*
    {
        return CreateAstMatcher<AstParamDecl>(
            __func__, [qualTypeMatcher, nameMatcher, arraySpecMatcher](const AstParamDecl* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryMatch(node->GetQualType(), qualTypeMatcher)
                    .TryMatchToken(node->GetDeclarator() ? node->GetDeclarator()->nameToken : AstSyntaxToken{},
                                   nameMatcher)
                    .TryMatch(node->GetDeclarator() ? node->GetDeclarator()->arraySpec : nullptr, arraySpecMatcher)
                    .Finish();
            });
    }
    auto CompilerTestFixture::FunctionDecl(AstMatcher* returnTypeMatcher, TokenMatcher* nameMatcher,
                                           std::vector<AstMatcher*> paramMatchers, AstMatcher* bodyMatcher)
        -> AstMatcher*
    {
        return CreateAstMatcher<AstFunctionDecl>(__func__,
                                                 [returnTypeMatcher, nameMatcher,
                                                  paramMatchers = std::move(paramMatchers),
                                                  bodyMatcher](const AstFunctionDecl* node) -> AstMatchResult {
                                                     return AstMatchPipeline{node}
                                                         .TryMatch(node->GetReturnType(), returnTypeMatcher)
                                                         .TryMatchToken(node->GetNameToken(), nameMatcher)
                                                         .TryEqual(paramMatchers.size(), node->GetParams().size(),
                                                                   "Unexpected parameter count: expected {}, got {}")
                                                         .TryMatchAll(node->GetParams(), paramMatchers)
                                                         .TryMatch(node->GetBody(), bodyMatcher)
                                                         .Finish();
                                                 });
    }
#pragma endregion

    auto CompilerTestFixture::TranslationUnit(std::vector<AstMatcher*> declMatchers) -> AstMatcher*
    {
        return CreateAstMatcher<AstTranslationUnit>(
            __func__, [declMatchers = std::move(declMatchers)](const AstTranslationUnit* node) -> AstMatchResult {
                return AstMatchPipeline{node}
                    .TryEqual(declMatchers.size(), node->GetGlobalDecls().size(),
                              "Unexpected global declaration count: expected {}, got {}")
                    .TryMatchAll(node->GetGlobalDecls(), declMatchers)
                    .Finish();
            });
    }
} // namespace glsld
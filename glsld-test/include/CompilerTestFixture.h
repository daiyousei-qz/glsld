#pragma once

#include "AstMatcher.h"

#include "Basic/StringView.h"
#include "Compiler/CompilerInvocation.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <tuple>

namespace glsld
{
    class CompilerTestFixture
    {
    private:
        std::string sourceTemplate;
        std::move_only_function<AstMatcher*(AstMatcher*)> matcherTemplate;

        // Matchers are managed by the test fixture
        std::vector<std::unique_ptr<TokenMatcher>> tokenMatchers;
        std::vector<std::unique_ptr<AstMatcher>> astMatchers;

        auto CreateTokenMatcher(std::string name, std::optional<TokenKlass> klass, std::optional<std::string> text)
            -> TokenMatcher*
        {
            tokenMatchers.push_back(std::make_unique<TokenMatcher>(name, klass, text));
            return tokenMatchers.back().get();
        }

        auto CreateAstMatcher(std::string name,
                              std::move_only_function<auto(const AstNode*)->AstMatchResult> matchCallback)
            -> AstMatcher*
        {
            astMatchers.push_back(std::make_unique<AstMatcher>(name, std::move(matchCallback)));
            return astMatchers.back().get();
        }

        template <std::derived_from<AstNode> AstNodeType>
        auto CreateAstMatcher(std::string name,
                              std::move_only_function<auto(const AstNodeType*)->AstMatchResult> matchCallback)
            -> AstMatcher*
        {
            return CreateAstMatcher(name, [innerCallback = std::move(matchCallback)](const AstNode* node) mutable {
                const AstNodeType* typedNode = node ? node->As<AstNodeType>() : nullptr;
                if (!typedNode) {
                    return AstMatchResult::FailWithUnexpectedAstNode(node, AstNodeTrait<AstNodeType>::name);
                }

                return innerCallback(typedNode);
            });
        }

        auto CreateAstMatcher(std::string desc, AstMatcher* finder, AstMatcher* matcher) -> AstMatcher*
        {
            astMatchers.push_back(std::make_unique<AstMatcher>(desc, finder, matcher));
            return astMatchers.back().get();
        }

        template <std::derived_from<AstNode> AstType>
        static auto MatchAll(ArrayView<const AstType*> nodes, ArrayView<AstMatcher*> matchers) -> AstMatchResult
        {
            GLSLD_REQUIRE(nodes.size() == matchers.size());

            for (size_t i = 0; i < nodes.size(); ++i) {
                if (AstMatchResult result = matchers[i]->Match(nodes[i]); !result.IsSuccess()) {
                    return result;
                }
            }

            return AstMatchResult::Success();
        }

        static auto MatchAll(std::initializer_list<std::pair<const AstNode*, AstMatcher*>> requests) -> AstMatchResult
        {
            for (const auto& [node, matcher] : requests) {
                if (auto result = matcher->Match(node); !result.IsSuccess()) {
                    return result;
                }
            }

            return AstMatchResult::Success();
        }

    public:
        auto SetTestTemplate(StringView sourceTemplate,
                             std::move_only_function<auto(AstMatcher*)->AstMatcher*> matcherTemplate = std::identity())
            -> void
        {
            this->sourceTemplate  = sourceTemplate.Str();
            this->matcherTemplate = std::move(matcherTemplate);
        }

        template <typename... Args>
        auto WrapSource(std::tuple<Args...> sourcePieces) -> std::string
        {
            return std::apply(
                [this](const auto&... args) { return fmt::format(fmt::runtime(sourceTemplate), args...); },
                sourcePieces);
        }
        auto WrapSource(StringView sourceText) -> std::string
        {
            return WrapSource(std::make_tuple(sourceText));
        }

        auto WrapMatcher(AstMatcher* matcher) -> AstMatcher*
        {
            return matcherTemplate(matcher);
        }

        auto Compile(StringView sourceText, CompileMode compileMode) const -> std::unique_ptr<CompilerResult>
        {
            auto compiler = std::make_unique<CompilerInvocation>();
            compiler->SetNoStdlib(true);
            compiler->SetMainFileFromBuffer(sourceText);
            return compiler->CompileMainFile(nullptr, compileMode);
        }

#pragma region Token Matchers
        auto AnyTok() -> TokenMatcher*
        {
            return CreateTokenMatcher("AnyToken", std::nullopt, std::nullopt);
        }
        auto EofTok() -> TokenMatcher*
        {
            return CreateTokenMatcher("EofToken", TokenKlass::Eof, std::nullopt);
        }
        auto InvalidTok() -> TokenMatcher*
        {
            return CreateTokenMatcher("InvalidToken", TokenKlass::Invalid, std::nullopt);
        }
        auto IdTok(StringView identifier) -> TokenMatcher*
        {
            return CreateTokenMatcher(fmt::format("Identifier[{}]", identifier), TokenKlass::Identifier,
                                      identifier.Str());
        }
        auto KeywordTok(TokenKlass keyword) -> TokenMatcher*
        {
            return CreateTokenMatcher(fmt::format("Keyword[{}]", TokenKlassToString(keyword)), keyword, std::nullopt);
        }
        auto IntTok(StringView value) -> TokenMatcher*
        {
            return CreateTokenMatcher(fmt::format("IntegerConstant[{}]", value), TokenKlass::IntegerConstant,
                                      value.Str());
        }
        auto FloatTok(StringView value) -> TokenMatcher*
        {
            return CreateTokenMatcher(fmt::format("FloatConstant[{}]", value), TokenKlass::FloatConstant, value.Str());
        }
#pragma endregion

#pragma region Ast Matchers
        auto NullAst() -> AstMatcher*;

        auto AnyAst() -> AstMatcher*;
        auto AnyInitializer() -> AstMatcher*;
        auto AnyExpr() -> AstMatcher*;
        auto AnyStmt() -> AstMatcher*;
        auto AnyDecl() -> AstMatcher*;
        auto AnyQual() -> AstMatcher*;
        auto AnyQualType() -> AstMatcher*;

        auto NamedQual(std::vector<TokenKlass> keyword) -> AstMatcher*;

        auto QualType(AstMatcher* qualMatcher, AstMatcher* structDeclMatcher, AstMatcher* arraySpecMatcher)
            -> AstMatcher*;
        auto QualType(AstMatcher* qualMatcher, TokenMatcher* typeNameMatcher, AstMatcher* arraySpecMatcher)
            -> AstMatcher*;
        auto NamedType(TokenKlass keywordKlass) -> AstMatcher*;
        auto NamedType(StringView name) -> AstMatcher*;
        auto StructType(AstMatcher* structDeclMatcher) -> AstMatcher*;

        auto ArraySpec(std::vector<AstMatcher*> sizeMatchers) -> AstMatcher*;

        auto InitializerList(std::vector<AstMatcher*> itemMatchers) -> AstMatcher*;
        auto ErrorExpr() -> AstMatcher*;
        auto LiteralExpr(ConstValue value) -> AstMatcher*;

        // Scalar literal
        template <typename T>
        auto LiteralExpr(T scalarValue) -> AstMatcher*
        {
            return LiteralExpr(ConstValue::CreateScalar(scalarValue));
        }

        auto NameAccessExpr(StringView name) -> AstMatcher*;
        auto FieldAccessExpr(AstMatcher* lhsMatcher, StringView name) -> AstMatcher*;
        auto SwizzleAccessExpr(AstMatcher* lhsMatcher, StringView swizzle) -> AstMatcher*;
        auto IndexAccessExpr(AstMatcher* lhsMatcher, AstMatcher* indexMatcher) -> AstMatcher*;
        auto UnaryExpr(UnaryOp op, AstMatcher* operandMatcher) -> AstMatcher*;
        auto BinaryExpr(BinaryOp op, AstMatcher* lhsMatcher, AstMatcher* rhsMatcher) -> AstMatcher*;
        auto SelectExpr(AstMatcher* condMatcher, AstMatcher* trueExprMatcher, AstMatcher* falseExprMatcher)
            -> AstMatcher*;
        auto ImplicitCastExpr(AstMatcher* matchOperand) -> AstMatcher*;
        auto FunctionCallExpr(TokenMatcher* nameMatcher, std::vector<AstMatcher*> argMatchers) -> AstMatcher*;
        auto FunctionCallExpr(StringView name, std::vector<AstMatcher*> argMatchers) -> AstMatcher*;
        auto ConstructorCallExpr(AstMatcher* typeMatcher, std::vector<AstMatcher*> argMatchers) -> AstMatcher*;

        auto ErrorStmt() -> AstMatcher*;
        auto EmptyStmt() -> AstMatcher*;
        auto CompoundStmt(std::vector<AstMatcher*> stmtMatchers) -> AstMatcher*;
        auto ExprStmt(AstMatcher* exprMatcher) -> AstMatcher*;
        auto DeclStmt(AstMatcher* declMatcher) -> AstMatcher*;
        auto IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher, AstMatcher* elseStmtMatcher) -> AstMatcher*;
        auto IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher) -> AstMatcher*;
        auto ForStmt(AstMatcher* initExprMatcher, AstMatcher* condExprMatcher, AstMatcher* iterExprMatcher,
                     AstMatcher* bodyMatcher) -> AstMatcher*;
        auto WhileStmt(AstMatcher* condMatcher, AstMatcher* bodyMatcher) -> AstMatcher*;
        auto DoWhileStmt(AstMatcher* bodyMatcher, AstMatcher* condMatcher) -> AstMatcher*;
        auto CaseLabelStmt(AstMatcher* exprMatcher) -> AstMatcher*;
        auto DefaultLabelStmt() -> AstMatcher*;
        auto SwitchStmt(AstMatcher* testExprMatcher, AstMatcher* bodyMatcher) -> AstMatcher*;
        auto BreakStmt() -> AstMatcher*;
        auto ContinueStmt() -> AstMatcher*;
        auto DiscardStmt() -> AstMatcher*;
        auto ReturnStmt() -> AstMatcher*;
        auto ReturnStmt(AstMatcher* exprMatcher) -> AstMatcher*;

        auto ErrorDecl() -> AstMatcher*;
        auto EmptyDecl() -> AstMatcher*;
        auto PrecisionDecl(AstMatcher* typeMatcher) -> AstMatcher*;
        auto BlockDecl(AstMatcher* qualMatcher, TokenMatcher* blockNameMatcher, std::vector<AstMatcher*> fieldMatchers)
            -> AstMatcher*;
        auto BlockDecl(AstMatcher* qualMatcher, TokenMatcher* blockNameMatcher, std::vector<AstMatcher*> fieldMatchers,
                       TokenMatcher* instanceNameMatcher, AstMatcher* instanceArraySpecMatcher) -> AstMatcher*;
        auto BlockFieldDecl(AstMatcher* qualTypeMatcher, std::vector<DeclaratorMatcher> declaratorMatchers)
            -> AstMatcher*;
        auto BlockFieldDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher) -> AstMatcher*;
        auto BlockFieldDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher, AstMatcher* arraySpecMatcher)
            -> AstMatcher*;
        auto VariableDecl(AstMatcher* qualTypeMatcher, std::vector<DeclaratorMatcher> declaratorMatchers)
            -> AstMatcher*;
        auto VariableDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher, AstMatcher* arraySpecMatcher,
                          AstMatcher* initializerMatcher) -> AstMatcher*;
        auto StructFieldDecl(AstMatcher* qualTypeMatcher, std::vector<DeclaratorMatcher> declaratorMatchers)
            -> AstMatcher*;
        auto StructFieldDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher, AstMatcher* arraySpecMatcher)
            -> AstMatcher*;
        auto StructDecl(TokenMatcher* nameMatcher, std::vector<AstMatcher*> fieldMatchers) -> AstMatcher*;
        auto ParamDecl(AstMatcher* qualTypeMatcher, TokenMatcher* nameMatcher, AstMatcher* arraySpecMatcher)
            -> AstMatcher*;
        auto FunctionDecl(AstMatcher* returnTypeMatcher, TokenMatcher* nameMatcher,
                          std::vector<AstMatcher*> paramMatchers, AstMatcher* bodyMatcher) -> AstMatcher*;

        auto TranslationUnit(std::vector<AstMatcher*> declMatchers) -> AstMatcher*;

        auto FindMatch(AstMatcher* finder, AstMatcher* matcher) -> AstMatcher*
        {
            return CreateAstMatcher("FindMatch", finder, matcher);
        }

#pragma endregion
    };

    class LexingTestCatchMatcher : public Catch::Matchers::MatcherBase<StringView>
    {
    private:
        const CompilerTestFixture& fixture;
        std::vector<TokenMatcher*> matchers;
        StringView matcherDesc;

    public:
        LexingTestCatchMatcher(const CompilerTestFixture& fixture, std::vector<TokenMatcher*> matchers,
                               StringView matcherDesc)
            : fixture(fixture), matchers(std::move(matchers)), matcherDesc(matcherDesc)
        {
        }

        auto match(const StringView& sourceText) const -> bool override
        {
            auto compilerResult = fixture.Compile(sourceText, CompileMode::PreprocessOnly);

            auto tokens = compilerResult->GetUserFileArtifacts().GetTokens();
            if (tokens.size() != matchers.size()) {
                UNSCOPED_INFO(fmt::format("Token stream match failed: expected {} tokens, got {} tokens",
                                          matchers.size(), tokens.size()));
                return false;
            }

            for (uint32_t i = 0; i < tokens.size(); ++i) {
                auto token = AstSyntaxToken{
                    .id    = SyntaxTokenID{TranslationUnitID::UserFile, i},
                    .klass = tokens[i].klass,
                    .text  = tokens[i].text,
                };
                if (!matchers[i]->Match(token)) {
                    UNSCOPED_INFO(fmt::format("Token stream match failed: mismatch at index {}", i));
                    return false;
                }
            }

            return true;
        }

        auto describe() const -> std::string override
        {
            return matcherDesc.Str();
        }
    };

    class AstTestCatchMatcher : public Catch::Matchers::MatcherBase<StringView>
    {
    private:
        CompilerTestFixture& fixture;
        AstMatcher* matcher;
        StringView matcherDesc;

    public:
        AstTestCatchMatcher(CompilerTestFixture& fixture, AstMatcher* matcher, StringView matcherDesc)
            : fixture(fixture), matcher(fixture.WrapMatcher(matcher)), matcherDesc(matcherDesc)
        {
        }

        auto match(const StringView& sourceText) const -> bool override
        {
            auto compilerResult = fixture.Compile(sourceText, CompileMode::ParseOnly);

            auto matchResult = matcher->Match(compilerResult->GetUserFileArtifacts().GetAst());
            if (matchResult.IsSuccess()) {
                return true;
            }
            else {
                UNSCOPED_INFO("AST match failed, see AST:\n" +
                              (matchResult.GetFailedNode() ? matchResult.GetFailedNode()->ToString() : "null") +
                              "\nSee error trace:\n" + matchResult.GetErrorTrace());
                return false;
            }
        }

        auto describe() const -> std::string override
        {
            return "AstMatcher";
        }
    };

} // namespace glsld

#define SOURCE_PIECES(...) std::make_tuple(__VA_ARGS__)

#define GLSLD_CHECK_TOKENS(SRC, ...)                                                                                   \
    do {                                                                                                               \
        auto matchers__ =                                                                                              \
            ::glsld::LexingTestCatchMatcher{*this, ::std::vector<::glsld::TokenMatcher*>{__VA_ARGS__}, #__VA_ARGS__};  \
        auto sourceText__ = WrapSource((SRC));                                                                         \
        CHECK_THAT(sourceText__, matchers__);                                                                          \
    } while (false)

#define GLSLD_CHECK_AST(SRC, ...)                                                                                      \
    do {                                                                                                               \
        auto matcher__    = ::glsld::AstTestCatchMatcher{*this, __VA_ARGS__, #__VA_ARGS__};                            \
        auto sourceText__ = WrapSource((SRC));                                                                         \
        CHECK_THAT(sourceText__, matcher__);                                                                           \
    } while (false)
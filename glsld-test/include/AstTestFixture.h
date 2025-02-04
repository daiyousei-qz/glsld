#pragma once

#include "AstMatcher.h"

#include "Basic/StringView.h"
#include "Compiler/CompilerInvocation.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <tuple>

namespace glsld
{

    class AstTestFixture
    {
    private:
        std::string sourceTemplate;
        std::move_only_function<AstMatcher*(AstMatcher*)> matcherTemplate;

        // Matchers are managed by the test fixture
        std::vector<std::unique_ptr<AstMatcher>> matchers;

        auto CreateMatcher(std::string desc,
                           std::move_only_function<auto(const AstNode*)->AstMatchResult> matchCallback) -> AstMatcher*
        {
            matchers.push_back(std::make_unique<AstMatcher>(desc, std::move(matchCallback)));
            return matchers.back().get();
        }

        auto CreateMatcher(std::string desc, AstMatcher* finder, AstMatcher* matcher) -> AstMatcher*
        {
            matchers.push_back(std::make_unique<AstMatcher>(desc, finder, matcher));
            return matchers.back().get();
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
                             std::move_only_function<auto(AstMatcher*)->AstMatcher*> matcherTemplate) -> void
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

        auto Compile(StringView sourceText) const -> std::unique_ptr<CompilerResult>
        {
            auto compiler = std::make_unique<CompilerInvocation>();
            compiler->SetNoStdlib(true);
            compiler->SetMainFileFromBuffer(sourceText);
            return compiler->CompileMainFile(nullptr, CompileMode::ParseOnly);
        }

#pragma region Matchers
        auto NullAst() -> AstMatcher*;

        auto AnyAst() -> AstMatcher*;
        auto AnyInitializer() -> AstMatcher*;
        auto AnyExpr() -> AstMatcher*;
        auto AnyStmt() -> AstMatcher*;
        auto AnyDecl() -> AstMatcher*;
        auto AnyQualType() -> AstMatcher*;

        auto BuiltinType(GlslBuiltinType type) -> AstMatcher*;
        auto NamedType(TokenMatcher nameMatcher) -> AstMatcher*;
        auto NamedType(StringView name) -> AstMatcher*;

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
        auto FunctionCallExpr(TokenMatcher nameMatcher, std::vector<AstMatcher*> argMatchers) -> AstMatcher*;
        auto FunctionCallExpr(StringView name, std::vector<AstMatcher*> argMatchers) -> AstMatcher*;
        auto ConstructorCallExpr(AstMatcher* typeMatcher, std::vector<AstMatcher*> argMatchers) -> AstMatcher*;

        auto ErrorStmt() -> AstMatcher*;
        auto EmptyStmt() -> AstMatcher*;
        auto CompoundStmt(std::vector<AstMatcher*> stmtMatchers) -> AstMatcher*;
        auto ExprStmt(AstMatcher* exprMatcher) -> AstMatcher*;
        auto DeclStmt(AstMatcher* declMatcher) -> AstMatcher*;
        auto IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher) -> AstMatcher*;
        auto IfStmt(AstMatcher* condMatcher, AstMatcher* thenStmtMatcher, AstMatcher* elseStmtMatcher) -> AstMatcher*;
        auto ForStmt(AstMatcher* initExprMatcher, AstMatcher* condExprMatcher, AstMatcher* iterExprMatcher,
                     AstMatcher* bodyMatcher) -> AstMatcher*;
        auto WhileStmt(AstMatcher* condMatcher, AstMatcher* bodyMatcher) -> AstMatcher*;
        auto DoWhileStmt(AstMatcher* bodyMatcher, AstMatcher* condMatcher) -> AstMatcher*;
        auto CaseLabelStmt(AstMatcher* exprMatcher) -> AstMatcher*;
        auto DefaultLabelStmt() -> AstMatcher*;
        auto SwitchStmt(AstMatcher* testExprMatcher, AstMatcher* bodyMatcher) -> AstMatcher*;
        auto BreakStmt() -> AstMatcher*;
        auto ContinueStmt() -> AstMatcher*;
        auto ReturnStmt() -> AstMatcher*;
        auto ReturnStmt(AstMatcher* exprMatcher) -> AstMatcher*;

        auto ErrorDecl() -> AstMatcher*;
        auto EmptyDecl() -> AstMatcher*;
        auto PrecisionDecl(AstMatcher* typeMatcher) -> AstMatcher*;
        auto VariableDecl(AstMatcher* qualTypeMatcher, std::vector<DeclaratorMatcher> declaratorMatchers)
            -> AstMatcher*;
        auto VariableDecl(AstMatcher* qualTypeMatcher, TokenMatcher nameMatcher, AstMatcher* arraySpecMatcher,
                          AstMatcher* initializerMatcher) -> AstMatcher*;
        auto FieldDecl(AstMatcher* qualTypeMatcher, std::vector<DeclaratorMatcher> declaratorMatchers) -> AstMatcher*;
        auto FieldDecl(AstMatcher* qualTypeMatcher, TokenMatcher nameMatcher, AstMatcher* arraySpecMatcher)
            -> AstMatcher*;
        auto StructDecl(TokenMatcher nameMatcher, std::vector<AstMatcher*> fieldMatchers) -> AstMatcher*;
        auto ParamDecl(AstMatcher* qualTypeMatcher, TokenMatcher nameMatcher, AstMatcher* arraySpecMatcher)
            -> AstMatcher*;
        auto FunctionDecl(AstMatcher* returnTypeMatcher, TokenMatcher nameMatcher,
                          std::vector<AstMatcher*> paramMatchers, AstMatcher* bodyMatcher) -> AstMatcher*;

        auto TranslationUnit(std::vector<AstMatcher*> declMatchers) -> AstMatcher*;

        auto FindMatch(AstMatcher* finder, AstMatcher* matcher) -> AstMatcher*
        {
            return CreateMatcher("FindMatch", finder, matcher);
        }

#pragma endregion
    };

    class AstTestCatchMatcher : public Catch::Matchers::MatcherBase<StringView>
    {
    private:
        AstTestFixture& fixture;
        AstMatcher* matcher;
        StringView matcherDesc;

    public:
        AstTestCatchMatcher(AstTestFixture& fixture, AstMatcher* matcher, StringView matcherDesc)
            : fixture(fixture), matcher(fixture.WrapMatcher(matcher)), matcherDesc(matcherDesc)
        {
        }

        auto match(const StringView& sourceText) const -> bool override
        {
            auto compilerResult = fixture.Compile(sourceText);

            auto matchResult = matcher->Match(compilerResult->GetUserFileArtifacts().GetAst());
            if (matchResult.IsSuccess()) {
                return true;
            }
            else {
                UNSCOPED_INFO(matchResult.GetFailedNode()->ToString());
                UNSCOPED_INFO(matchResult.GetErrorTrace());
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

#define GLSLD_CHECK_AST(SRC, ...)                                                                                      \
    do {                                                                                                               \
        auto matcher    = ::glsld::AstTestCatchMatcher{*this, __VA_ARGS__, #__VA_ARGS__};                              \
        auto sourceText = WrapSource((SRC));                                                                           \
        CHECK_THAT(sourceText, matcher);                                                                               \
    } while (false)
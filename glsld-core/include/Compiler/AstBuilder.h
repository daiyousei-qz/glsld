#pragma once
#include "Ast/Expr.h"
#include "Ast/Stmt.h"
#include "Ast/Decl.h"
#include "Ast/Misc.h"
#include "Compiler/AstContext.h"
#include "Compiler/CompilerInvocationState.h"
#include "Compiler/SymbolTable.h"

namespace glsld
{
    // A stateful builder to construct AST from parsed nodes. AST nodes leaving this builder should be immutable.
    // This class also computes and fills payload, including:
    // - Name resolution
    // - Type deduction
    // - .etc
    // However, we don't do sophisticated type checking here.
    class AstBuilder
    {
    private:
        AstContext& astContext;

        MemoryArena& arena;

        SymbolTable& symbolTable;

        // Return type of the current function. `nullptr` if we are not in a function.
        const Type* returnType;

    public:
        AstBuilder(CompilerInvocationState& compiler)
            : astContext(compiler.GetAstContext()), arena(compiler.GetAstContext().GetArena()),
              symbolTable(compiler.GetSymbolTable())
        {
        }

        auto GetAstContext() -> AstContext&
        {
            return astContext;
        }

        auto IsStructName(StringView name) const -> bool
        {
            auto symbol = symbolTable.FindSymbol(name);
            return symbol.IsValid() && symbol.GetDecl()->Is<AstStructDecl>();
        }

        auto EnterFunctionScope(const Type* returnType) -> void
        {
            this->returnType = returnType;
            symbolTable.PushLevel(DeclScope::Function);
        }
        auto LeaveFunctionScope() -> void
        {
            this->returnType = nullptr;
            symbolTable.PopLevel();
        }
        auto EnterLexicalBlockScope() -> void
        {
            symbolTable.PushLevel(DeclScope::Block);
        }
        auto LeaveLexicalBlockScope() -> void
        {
            symbolTable.PopLevel();
        }

        auto GetCurrentScope() const -> DeclScope
        {
            return symbolTable.GetCurrentLevel()->GetScope();
        }

#pragma region Misc

        auto BuildTranslationUnit(AstSyntaxRange range, std::vector<AstDecl*> decls) -> AstTranslationUnit*;

        auto BuildArraySpec(AstSyntaxRange range, std::vector<AstExpr*> sizes) -> AstArraySpec*;

        auto BuildTypeQualifierSeq(AstSyntaxRange range, QualifierGroup quals, std::vector<LayoutItem> layoutQuals)
            -> AstTypeQualifierSeq*;

        auto BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, AstSyntaxToken typeName,
                           AstArraySpec* arraySpec) -> AstQualType*;

        auto BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, AstStructDecl* structDecl,
                           AstArraySpec* arraySpec) -> AstQualType*;

        auto BuildInitializerList(AstSyntaxRange range, std::vector<AstInitializer*> initializers,
                                  const Type* contextType) -> AstInitializerList*;

#pragma endregion

#pragma region Expr

        auto BuildErrorExpr(AstSyntaxRange range) -> AstErrorExpr*;

        auto BuildLiteralExpr(AstSyntaxRange range, ConstValue value) -> AstLiteralExpr*;

        auto BuildNameAccessExpr(AstSyntaxRange range, AstSyntaxToken idToken) -> AstNameAccessExpr*;

        // Build either a swizzle or a field access expression
        auto BuildDotAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, AstSyntaxToken idToken) -> AstExpr*;

        auto BuildIndexAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, AstExpr* index) -> AstIndexAccessExpr*;

        auto BuildUnaryExpr(AstSyntaxRange range, AstExpr* operand, UnaryOp opcode) -> AstUnaryExpr*;

        auto BuildBinaryExpr(AstSyntaxRange range, AstExpr* lhs, AstExpr* rhs, BinaryOp opcode) -> AstBinaryExpr*;

        auto BuildSelectExpr(AstSyntaxRange range, AstExpr* condExpr, AstExpr* trueExpr, AstExpr* falseExpr)
            -> AstSelectExpr*;

        auto BuildImplicitCastExpr(AstSyntaxRange range, AstExpr* expr, const Type* castType) -> AstImplicitCastExpr*;

        auto BuildFuntionCallExpr(AstSyntaxRange range, AstSyntaxToken functionName, std::vector<AstExpr*> args)
            -> AstFunctionCallExpr*;

        auto BuildConstructorCallExpr(AstSyntaxRange range, AstQualType* qualType, std::vector<AstExpr*> args)
            -> AstConstructorCallExpr*;

#pragma endregion

#pragma region Stmt

        auto BuildErrorStmt(AstSyntaxRange range) -> AstErrorStmt*;

        auto BuildEmptyStmt(AstSyntaxRange range) -> AstEmptyStmt*;

        auto BuildCompoundStmt(AstSyntaxRange range, std::vector<AstStmt*> stmts) -> AstCompoundStmt*;

        auto BuildExprStmt(AstSyntaxRange range, AstExpr* expr) -> AstExprStmt*;

        auto BuildDeclStmt(AstSyntaxRange range, AstDecl* decl) -> AstDeclStmt*;

        auto BuildIfStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* thenStmt) -> AstIfStmt*;

        auto BuildIfStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* thenStmt, AstStmt* elseStmt) -> AstIfStmt*;

        auto BuildForStmt(AstSyntaxRange range, AstStmt* initStmt, AstExpr* condExpr, AstExpr* iterExpr,
                          AstStmt* bodyStmt) -> AstForStmt*;

        auto BuildWhileStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* bodyStmt) -> AstWhileStmt*;

        auto BuildDoWhileStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* bodyStmt) -> AstDoWhileStmt*;

        auto BuildLabelStmt(AstSyntaxRange range, AstExpr* caseExpr) -> AstLabelStmt*;

        auto BuildSwitchStmt(AstSyntaxRange range, AstExpr* testExpr, AstStmt* bodyStmt) -> AstSwitchStmt*;

        auto BuildJumpStmt(AstSyntaxRange range, JumpType jumpType) -> AstJumpStmt*;

        auto BuildReturnStmt(AstSyntaxRange range, AstExpr* expr) -> AstReturnStmt*;

#pragma endregion

#pragma region Decl

        auto BuildErrorDecl(AstSyntaxRange range) -> AstErrorDecl*;

        auto BuildEmptyDecl(AstSyntaxRange range) -> AstEmptyDecl*;

        auto BuildPrecisionDecl(AstSyntaxRange range, AstQualType* type) -> AstPrecisionDecl*;

        auto BuildVariableDecl(AstSyntaxRange range, AstQualType* qualType, std::vector<Declarator> declarators)
            -> AstVariableDecl*;

        auto BuildStructFieldDecl(AstSyntaxRange range, AstQualType* qualType, std::vector<Declarator> declarators)
            -> AstStructFieldDecl*;

        auto BuildBlockFieldDecl(AstSyntaxRange range, AstQualType* qualType, std::vector<Declarator> declarators)
            -> AstBlockFieldDecl*;

        auto BuildStructDecl(AstSyntaxRange range, std::optional<AstSyntaxToken> declTok,
                             std::vector<AstStructFieldDecl*> members) -> AstStructDecl*;

        auto BuildInterfaceBlockDecl(AstSyntaxRange range, AstTypeQualifierSeq* quals, AstSyntaxToken declTok,
                                     std::vector<AstBlockFieldDecl*> members, std::optional<Declarator> declarator)
            -> AstInterfaceBlockDecl*;

        auto BuildParamDecl(AstSyntaxRange range, AstQualType* qualType, std::optional<Declarator> declarator)
            -> AstParamDecl*;

        auto BuildFunctionDecl(AstSyntaxRange range, AstQualType* returnType, AstSyntaxToken declTok,
                               std::vector<AstParamDecl*> params, AstStmt* body) -> AstFunctionDecl*;

#pragma endregion

    private:
        template <typename T>
        auto CopyArray(const std::vector<T>& array) -> ArrayView<T>
        {
            T* buffer = arena.Construct<T[]>(array.size());
            std::ranges::copy(array, buffer);
            return ArrayView<T>(buffer, array.size());
        }

        template <AstNodeT T, typename... Args>
        auto CreateAstNode(AstSyntaxRange range, Args&&... args) -> T*
        {
            auto result = arena.Construct<T>(std::forward<Args>(args)...);
            result->Initialize(AstNodeTrait<T>::tag, range);
            return result;
        }

        // Try to implicitly cast an expression to the context type.
        auto TryMakeImplicitCast(AstExpr* expr, const Type* contextType) -> AstExpr*
        {
            if (expr->GetDeducedType()->IsSameWith(contextType) || expr->GetDeducedType()->IsError()) {
                return expr;
            }

            return BuildImplicitCastExpr(expr->GetSyntaxRange(), expr, contextType);
        }
    };
} // namespace glsld
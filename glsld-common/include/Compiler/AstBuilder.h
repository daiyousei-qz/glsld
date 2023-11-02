#pragma once
#include "Ast/Expr.h"
#include "Ast/Stmt.h"
#include "Ast/Decl.h"
#include "Ast/Misc.h"
#include "Compiler/CompilerObject.h"
#include "Compiler/AstContext.h"
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
        CompilerObject& compilerObject;

        SymbolTable& symbolTable;

        // Return type of the current function. `nullptr` if we are not in a function.
        const Type* returnType;

    public:
        AstBuilder(CompilerObject& compilerObject)
            : compilerObject(compilerObject), symbolTable(compilerObject.GetAstContext().GetSymbolTable())
        {
        }

        auto IsTypeName(StringView name) const
        {
            auto symbol = symbolTable.FindSymbol(name.Str());
            return symbol.IsValid() && symbol.GetDecl()->Is<AstStructDecl>();
        }

        auto EnterFunction(const Type* returnType) -> void
        {
            this->returnType = returnType;
            symbolTable.PushLevel(DeclScope::Function);
        }
        auto LeaveFunction() -> void
        {
            this->returnType = nullptr;
            symbolTable.PopLevel();
        }
        auto EnterLexicalBlock() -> void
        {
            symbolTable.PushLevel(DeclScope::Block);
        }
        auto LeaveLexicalBlock() -> void
        {
            symbolTable.PopLevel();
        }

#pragma region Misc

        auto BuildTranslationUnit(AstSyntaxRange range, std::vector<AstDecl*> decls) -> AstTranslationUnit*;

        auto BuildArraySpec(AstSyntaxRange range, std::vector<AstExpr*> sizes) -> AstArraySpec*;

        auto BuildTypeQualifierSeq(AstSyntaxRange range, QualifierGroup quals, std::vector<LayoutItem> layoutQuals)
            -> AstTypeQualifierSeq*;

        auto BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, SyntaxToken typeName,
                           AstArraySpec* arraySpec) -> AstQualType*;

        auto BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, AstStructDecl* structDecl,
                           AstArraySpec* arraySpec) -> AstQualType*;

        auto BuildInitializerList(AstSyntaxRange range, std::vector<AstInitializer*> initializers)
            -> AstInitializerList*;

#pragma endregion

#pragma region Expr

        auto BuildErrorExpr(AstSyntaxRange range) -> AstErrorExpr*;

        auto BuildLiteralExpr(AstSyntaxRange range, ConstValue value) -> AstLiteralExpr*;

        auto BuildNameAccessExpr(AstSyntaxRange range, SyntaxToken idToken) -> AstNameAccessExpr*;

        // Build either a swizzle or a field access expression
        auto BuildDotAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, SyntaxToken idToken) -> AstExpr*;

        auto BuildIndexAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, AstArraySpec* indices)
            -> AstIndexAccessExpr*;

        auto BuildUnaryExpr(AstSyntaxRange range, AstExpr* operand, UnaryOp opcode) -> AstUnaryExpr*;

        auto BuildBinaryExpr(AstSyntaxRange range, AstExpr* lhs, AstExpr* rhs, BinaryOp opcode) -> AstBinaryExpr*;

        auto BuildSelectExpr(AstSyntaxRange range, AstExpr* condExpr, AstExpr* trueExpr, AstExpr* falseExpr)
            -> AstSelectExpr*;

        auto BuildImplicitCastExpr(AstSyntaxRange range, AstExpr* expr, const Type* castType) -> AstImplicitCastExpr*;

        auto BuildFuntionCallExpr(AstSyntaxRange range, SyntaxToken functionName, std::vector<AstExpr*> args)
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

        auto BuildVariableDecl(AstSyntaxRange range, AstQualType* qualType, std::vector<Declarator> declarators)
            -> AstVariableDecl*;

        auto BuildFieldDecl(AstSyntaxRange range, AstQualType* qualType, std::vector<Declarator> declarators)
            -> AstFieldDecl*;

        auto BuildStructDecl(AstSyntaxRange range, std::optional<SyntaxToken> declTok,
                             std::vector<AstFieldDecl*> members) -> AstStructDecl*;

        auto BuildParamDecl(AstSyntaxRange range, AstQualType* qualType, std::optional<Declarator> declarator)
            -> AstParamDecl*;

        auto BuildFunctionDecl(AstSyntaxRange range, AstQualType* returnType, SyntaxToken declTok,
                               std::vector<AstParamDecl*> params, AstStmt* body) -> AstFunctionDecl*;

        auto BuildInterfaceBlockDecl(AstSyntaxRange range, AstTypeQualifierSeq* quals, SyntaxToken declTok,
                                     std::vector<AstFieldDecl*> members, std::optional<Declarator> declarator)
            -> AstInterfaceBlockDecl*;

#pragma endregion

    private:
        auto TryMakeImplicitCast(AstExpr* expr, const Type* contextType) -> AstExpr*
        {
            if (expr->GetDeducedType()->IsSameWith(contextType) || expr->GetDeducedType()->IsError()) {
                return expr;
            }

            GLSLD_ASSERT(!contextType->IsError());
            return BuildImplicitCastExpr(expr->GetSyntaxRange(), expr, contextType);
        }

        template <AstNodeT T, typename... Args>
        auto CreateAstNode(AstSyntaxRange range, Args&&... args) -> T*
        {
            return compilerObject.GetAstContext().CreateAstNode<T>(range, std::forward<Args>(args)...);
        }
    };
} // namespace glsld
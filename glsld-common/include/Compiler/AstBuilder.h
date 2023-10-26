#pragma once
#include "Ast/Expr.h"
#include "Ast/Stmt.h"
#include "Ast/Decl.h"
#include "Ast/Misc.h"
#include "Compiler/CompilerObject.h"
#include "Compiler/AstContext.h"
#include "Compiler/TypeContext.h"
#include "Compiler/SymbolTable.h"

namespace glsld
{
    inline auto DeduceUnaryExprType(UnaryOp op, const Type* operandType) -> const Type*
    {
        switch (op) {
        case UnaryOp::Identity:
            return operandType;
        case UnaryOp::Negate:
            if (operandType->IsSameWith(GlslBuiltinType::Ty_int) || operandType->IsSameWith(GlslBuiltinType::Ty_uint) ||
                operandType->IsSameWith(GlslBuiltinType::Ty_float) ||
                operandType->IsSameWith(GlslBuiltinType::Ty_double)) {
                return operandType;
            }
            else {
                return Type::GetErrorType();
            }
        case UnaryOp::BitwiseNot:
            if (operandType->IsSameWith(GlslBuiltinType::Ty_int) || operandType->IsSameWith(GlslBuiltinType::Ty_uint)) {
                return operandType;
            }
            else {
                return Type::GetErrorType();
            }
        case UnaryOp::LogicalNot:
            if (operandType->IsSameWith(GlslBuiltinType::Ty_bool)) {
                return operandType;
            }
            else {
                return Type::GetErrorType();
            }
        case UnaryOp::PrefixInc:
        case UnaryOp::PrefixDec:
        case UnaryOp::PostfixInc:
        case UnaryOp::PostfixDec:
            if (operandType->IsSameWith(GlslBuiltinType::Ty_int) || operandType->IsSameWith(GlslBuiltinType::Ty_uint)) {
                return operandType;
            }
            else {
                return Type::GetErrorType();
            }
        case UnaryOp::Length:
            // FIXME: test if the type has length operation
            return Type::GetBuiltinType(GlslBuiltinType::Ty_int);
        }

        GLSLD_UNREACHABLE();
    }

    inline auto IsUnaryExprConst(UnaryOp op, const Type* operandType) -> bool
    {
        return false;
    }

    inline auto DeduceBinaryExprType(BinaryOp op, const Type* lhsType, const Type* rhsType) -> const Type*
    {
        // FIXME: implement this
        if (lhsType->IsSameWith(rhsType)) {
            return lhsType;
        }
        else {
            return Type::GetErrorType();
        }
    }

    inline auto IsBinaryExprConst(BinaryOp op, const Type* lhsType, const Type* rhsType) -> bool
    {
        return false;
    }

    struct SymbolReferenceInfo
    {
        const Type* type;

        bool isConst;
    };

    inline auto ComputeSymbolReferenceInfo(DeclView declView) -> std::optional<SymbolReferenceInfo>
    {
        if (!declView.IsValid()) {
            return std::nullopt;
        }

        if (auto variableDecl = declView.GetDecl()->As<AstVariableDecl>()) {
            // TODO: should we also check if initializer is const?
            return SymbolReferenceInfo{.type    = variableDecl->GetResolvedTypes()[declView.GetIndex()],
                                       .isConst = variableDecl->GetQualType()->GetQualifiers()->GetQualGroup().qConst};
        }
        else if (auto paramDecl = declView.GetDecl()->As<AstParamDecl>()) {
            return SymbolReferenceInfo{
                .type    = paramDecl->GetResolvedType(),
                .isConst = false,
            };
        }
        else if (auto interfaceBlockDecl = declView.GetDecl()->As<AstInterfaceBlockDecl>()) {
            GLSLD_ASSERT(interfaceBlockDecl->GetDeclarator().has_value());
            return SymbolReferenceInfo{
                .type    = interfaceBlockDecl->GetResolvedInstanceType(),
                .isConst = false,
            };
        }
        else if (auto structDecl = declView.GetDecl()->As<AstStructDecl>()) {
            return SymbolReferenceInfo{
                .type    = Type::GetErrorType(),
                .isConst = false,
            };
        }
        else {
            return std::nullopt;
        }
    }

    // Construct AST from parsed nodes. This class also computes and fills payload, including:
    // - Name resolution
    // - Type deduction
    // - .etc
    // However, we don't do sophisticated type checking here.
    class AstBuilder
    {
    private:
        CompilerObject& compilerObject;

        SymbolTable symbolTable;

        std::vector<AstNode*> astNodes;

    public:
        AstBuilder(CompilerObject& compilerObject) : compilerObject(compilerObject)
        {
        }

        auto IsTypeName(StringView name) const
        {
            auto symbol = symbolTable.FindSymbol(name.Str());
            return symbol.IsValid() && symbol.GetDecl()->Is<AstStructDecl>();
        }

        auto EnterFunction() -> void
        {
            symbolTable.PushLevel(DeclScope::Function);
        }
        auto LeaveFunction() -> void
        {
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
        auto EnterInitializeList() -> void;
        auto LeaveInitializeList() -> void;

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

        auto BuildMemberNameAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, SyntaxToken idToken)
            -> AstMemberNameAccessExpr*;

        auto BuildIndexAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, AstArraySpec* indices)
            -> AstIndexAccessExpr*;

        auto BuildUnaryExpr(AstSyntaxRange range, AstExpr* operand, UnaryOp opcode) -> AstUnaryExpr*;

        auto BuildBinaryExpr(AstSyntaxRange range, AstExpr* lhs, AstExpr* rhs, BinaryOp opcode) -> AstBinaryExpr*;

        auto BuildSelectExpr(AstSyntaxRange range, AstExpr* condExpr, AstExpr* trueExpr, AstExpr* falseExpr)
            -> AstSelectExpr*;

        auto BuildInvokeExpr(AstSyntaxRange range, SyntaxToken functionName, std::vector<AstExpr*> args)
            -> AstFunctionCallExpr*;

        auto BuildConstructorExpr(AstSyntaxRange range, AstQualType* qualType, std::vector<AstExpr*> args)
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

        auto BuildVariableDecl(AstSyntaxRange range, AstQualType* qualType, std::vector<VariableDeclarator> declarators)
            -> AstVariableDecl*;

        auto BuildStructDecl(AstSyntaxRange range, std::optional<SyntaxToken> declTok,
                             std::vector<AstVariableDecl*> members) -> AstStructDecl*;

        auto BuildParamDecl(AstSyntaxRange range, AstQualType* qualType, VariableDeclarator declarator)
            -> AstParamDecl*;

        auto BuildFunctionDecl(AstSyntaxRange range, AstQualType* returnType, SyntaxToken declTok,
                               std::vector<AstParamDecl*> params, AstStmt* body) -> AstFunctionDecl*;

        auto BuildInterfaceBlockDecl(AstSyntaxRange range, AstTypeQualifierSeq* quals, SyntaxToken declTok,
                                     std::vector<AstVariableDecl*> members,
                                     std::optional<VariableDeclarator> declarator) -> AstInterfaceBlockDecl*;

#pragma endregion

    private:
        template <AstNodeT T, typename... Args>
        auto CreateAstNode(AstSyntaxRange range, Args&&... args) -> T*
        {
            return compilerObject.GetAstContext().CreateAstNode<T>(range, std::forward<Args>(args)...);
        }
    };
} // namespace glsld
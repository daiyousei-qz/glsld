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

    struct VariableReferenceInfo
    {
        const Type* type;

        bool isConst;
    };

    inline auto ComputeVariableReferenceInfo(DeclView declView) -> std::optional<VariableReferenceInfo>
    {
        if (!declView.IsValid()) {
            return std::nullopt;
        }

        if (auto variableDecl = declView.GetDecl()->As<AstVariableDecl>()) {
            // TODO: should we also check if initializer is const?
            return VariableReferenceInfo{.type = variableDecl->GetResolvedTypes()[declView.GetIndex()],
                                         .isConst =
                                             variableDecl->GetQualType()->GetQualifiers()->GetQualGroup().qConst};
        }
        else if (auto paramDecl = declView.GetDecl()->As<AstParamDecl>()) {
            return VariableReferenceInfo{
                .type    = paramDecl->GetResolvedType(),
                .isConst = false,
            };
        }
        else if (auto interfaceBlockDecl = declView.GetDecl()->As<AstInterfaceBlockDecl>()) {
            GLSLD_ASSERT(interfaceBlockDecl->GetDeclarator().has_value());
            return VariableReferenceInfo{
                .type    = interfaceBlockDecl->GetResolvedInstanceType(),
                .isConst = false,
            };
        }
        else {
            return std::nullopt;
        }
    }

    class AstBuilder
    {
    private:
        CompilerObject& compilerObject;

        SymbolTable symbolTable;

        DeclScope currentScope = DeclScope::Global;

        // We are currently in a context of parsing possibly function name.
        bool expectFunctionName = false;

    public:
        AstBuilder(CompilerObject& compilerObject) : compilerObject(compilerObject)
        {
        }

        auto EnterFunction() -> void;
        auto LeaveFunction() -> void;
        auto EnterLexicalBlock() -> void;
        auto LeaveLexicalBlock() -> void;
        auto EnterInitializeList() -> void;
        auto LeaveInitializeList() -> void;

#pragma region Misc

        auto BuildArraySpec(AstSyntaxRange range, std::vector<AstExpr*> sizes) -> AstArraySpec*
        {
            return CreateAstNode<AstArraySpec>(range, std::move(sizes));
        }

        auto BuildTypeQualifierSeq(AstSyntaxRange range, QualifierGroup quals, std::vector<LayoutItem> layoutQuals)
            -> AstTypeQualifierSeq*
        {
            return CreateAstNode<AstTypeQualifierSeq>(range, quals, std::move(layoutQuals));
        }

        auto BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, SyntaxToken typeName,
                           AstArraySpec* arraySpec) -> AstQualType*
        {
            auto result = CreateAstNode<AstQualType>(range, qualifiers, typeName, arraySpec);

            // Set up a good default first
            result->SetResolvedType(Type::GetErrorType());

            if (auto glslType = GetGlslBuiltinType(typeName.klass)) {
                result->SetResolvedType(
                    compilerObject.GetTypeContext().GetArrayType(Type::GetBuiltinType(*glslType), arraySpec));
            }
            else if (typeName.IsIdentifier()) {
                auto symbol = symbolTable.FindSymbol(typeName.text.Str());
                if (symbol.IsValid()) {
                    if (auto structDecl = symbol.GetDecl()->As<AstStructDecl>()) {
                        result->SetResolvedType(
                            compilerObject.GetTypeContext().GetArrayType(structDecl->GetDeclaredType(), arraySpec));
                    }
                }
            }
            return result;
        }

        auto BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, AstStructDecl* structDecl,
                           AstArraySpec* arraySpec)
        {
            auto result = CreateAstNode<AstQualType>(range, qualifiers, structDecl, arraySpec);

            result->SetResolvedType(
                compilerObject.GetTypeContext().GetArrayType(structDecl->GetDeclaredType(), arraySpec));
            return result;
        }

        auto BuildInitializerList(AstSyntaxRange range, std::vector<AstInitializer*> initializers)
            -> AstInitializerList*
        {
            return CreateAstNode<AstInitializerList>(range, std::move(initializers));
        }

#pragma endregion

#pragma region Expr
        auto BuildErrorExpr(AstSyntaxRange range) -> AstErrorExpr*
        {
            auto result = CreateAstNode<AstErrorExpr>(range);

            result->SetConst(false);
            result->SetDeducedType(Type::GetErrorType());
            return result;
        }

        auto BuildLiteralExpr(AstSyntaxRange range, ConstValue value) -> AstLiteralExpr*
        {
            auto result = CreateAstNode<AstLiteralExpr>(range, std::move(value));

            result->SetConst(true);
            if (auto glslType = result->GetValue().GetGlslType()) {
                result->SetDeducedType(Type::GetBuiltinType(*glslType));
            }
            else {
                result->SetDeducedType(Type::GetErrorType());
            }
            return result;
        }
        auto BuildNameAccessExpr(AstSyntaxRange range, SyntaxToken idToken) -> AstNameAccessExpr*
        {
            auto result = CreateAstNode<AstNameAccessExpr>(range, idToken);

            // Set a good default since the logic is complicated.
            result->SetConst(false);
            result->SetDeducedType(Type::GetErrorType());
            result->SetResolvedDecl({});

            // Note functions are resolved in `BuildInvokeExpr` because of the need of overload resolution.
            if (!expectFunctionName && idToken.IsIdentifier()) {
                auto symbol = symbolTable.FindSymbol(idToken.text.Str());
                if (auto refInfo = ComputeVariableReferenceInfo(symbol)) {
                    GLSLD_ASSERT(symbol.IsValid());
                    result->SetResolvedDecl(symbol);
                    result->SetDeducedType(refInfo->type);
                    result->SetConst(refInfo->isConst);
                }
            }

            return result;
        }
        auto BuildMemberNameAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, SyntaxToken idToken)
            -> AstMemberNameAccessExpr*
        {
            auto result = CreateAstNode<AstMemberNameAccessExpr>(range, baseExpr, idToken);

            // FIXME: implement this
            result->SetConst(false);
            result->SetDeducedType(Type::GetErrorType());
            return result;
        }
        auto BuildIndexAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, AstArraySpec* indices) -> AstIndexAccessExpr*
        {
            auto result = CreateAstNode<AstIndexAccessExpr>(range, baseExpr, indices);

            // FIXME: implement this
            result->SetConst(false);
            result->SetDeducedType(Type::GetErrorType());
            return result;
        }

        auto BuildUnaryExpr(AstSyntaxRange range, AstExpr* operand, UnaryOp opcode) -> AstUnaryExpr*
        {
            auto result = CreateAstNode<AstUnaryExpr>(range, operand, opcode);

            if (operand->IsConst() && IsUnaryExprConst(opcode, operand->GetDeducedType())) {
                result->SetConst(true);
            }
            else {
                result->SetConst(false);
            }
            result->SetDeducedType(DeduceUnaryExprType(opcode, operand->GetDeducedType()));
            return result;
        }
        auto BuildBinaryExpr(AstSyntaxRange range, AstExpr* lhs, AstExpr* rhs, BinaryOp opcode) -> AstBinaryExpr*
        {
            auto result = CreateAstNode<AstBinaryExpr>(range, lhs, rhs, opcode);

            if (lhs->IsConst() && rhs->IsConst() &&
                IsBinaryExprConst(opcode, lhs->GetDeducedType(), rhs->GetDeducedType())) {
                result->SetConst(true);
            }
            else {
                result->SetConst(false);
            }
            result->SetDeducedType(DeduceBinaryExprType(opcode, lhs->GetDeducedType(), rhs->GetDeducedType()));
            return result;
        }
        auto BuildSelectExpr(AstSyntaxRange range, AstExpr* condExpr, AstExpr* trueExpr, AstExpr* falseExpr)
            -> AstSelectExpr*
        {
            auto result = CreateAstNode<AstSelectExpr>(range, condExpr, trueExpr, falseExpr);

            if (condExpr->IsConst() && trueExpr->IsConst() && falseExpr->IsConst()) {
                result->SetConst(true);
            }
            else {
                result->SetConst(false);
            }
            if (trueExpr->GetDeducedType()->IsSameWith(falseExpr->GetDeducedType())) {
                result->SetDeducedType(trueExpr->GetDeducedType());
            }
            else {
                result->SetDeducedType(Type::GetErrorType());
            }
            return result;
        }
        auto BuildInvokeExpr(AstSyntaxRange range, AstExpr* invokedExpr, std::vector<AstExpr*> args) -> AstInvokeExpr*
        {
            // FIXME: do we need fix up AST for constructor call?
            auto result = CreateAstNode<AstInvokeExpr>(range, invokedExpr, std::move(args));

            // Set a good default since the logic is complicated.
            result->SetConst(false);
            result->SetDeducedType(Type::GetErrorType());

            // We need to resolve the called function first
            if (auto invokedNameExpr = invokedExpr->As<AstNameAccessExpr>()) {
                // Case 1: function/constructor call `IDENTIFIER(...)`

                std::vector<const Type*> argTypes;
                for (auto arg : result->GetArgs()) {
                    argTypes.push_back(arg->GetDeducedType());
                }

                // TODO: if the function name could uniquely identify a function, we can skip overload resolution
                //       this could help make the resolution more resilient to errors while typing
                auto function = symbolTable.FindFunction(invokedNameExpr->GetAccessName().text.Str(), argTypes);
                if (function) {
                    invokedNameExpr->SetResolvedDecl(function);
                    // invokedNameExpr->SetDeducedType(function->GetDeducedType());

                    // result->SetDeducedType(function->GetReturnType().)
                    // TODO: result of some function call can be const
                    result->SetConst(false);
                }
                else {
                    // This could also be a constructor call
                }
            }
            else if (auto invokedIndexExpr = invokedExpr->As<AstIndexAccessExpr>();
                     invokedIndexExpr && invokedIndexExpr->GetBaseExpr()->Is<AstNameAccessExpr>()) {
                // Case 2: constructor call `TYPE[...](...)`

                // result->SetDeducedType();
                result->SetConst(false);
            }
            else {
                // FIXME: Are there other cases? Perhaps need to report error for invalid invocation.
                // Note: we treat `IDENTIFIER.length()` as a unary expression, not a function call.
            }

            return result;
        }
#pragma endregion

#pragma region Stmt
        auto BuildErrorStmt(AstSyntaxRange range) -> AstErrorStmt*
        {
            return CreateAstNode<AstErrorStmt>(range);
        }

        auto BuildEmptyStmt(AstSyntaxRange range) -> AstEmptyStmt*
        {
            return CreateAstNode<AstEmptyStmt>(range);
        }

        auto BuildCompoundStmt(AstSyntaxRange range, std::vector<AstStmt*> stmts) -> AstCompoundStmt*
        {
            return CreateAstNode<AstCompoundStmt>(range, std::move(stmts));
        }

        auto BuildExprStmt(AstSyntaxRange range, AstExpr* expr) -> AstExprStmt*
        {
            return CreateAstNode<AstExprStmt>(range, expr);
        }

        auto BuildDeclStmt(AstSyntaxRange range, AstDecl* decl) -> AstDeclStmt*
        {
            return CreateAstNode<AstDeclStmt>(range, decl);
        }

        auto BuildIfStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* thenStmt) -> AstIfStmt*
        {
            return CreateAstNode<AstIfStmt>(range, condExpr, thenStmt, nullptr);
        }
        auto BuildIfStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* thenStmt, AstStmt* elseStmt) -> AstIfStmt*
        {
            return CreateAstNode<AstIfStmt>(range, condExpr, thenStmt, elseStmt);
        }

        auto BuildForStmt(AstSyntaxRange range, AstStmt* initStmt, AstExpr* condExpr, AstExpr* iterExpr,
                          AstStmt* bodyStmt) -> AstForStmt*
        {
            return CreateAstNode<AstForStmt>(range, initStmt, condExpr, iterExpr, bodyStmt);
        }

        auto BuildWhileStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* bodyStmt) -> AstWhileStmt*
        {
            return CreateAstNode<AstWhileStmt>(range, condExpr, bodyStmt);
        }

        auto BuildDoWhileStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* bodyStmt) -> AstDoWhileStmt*
        {
            return CreateAstNode<AstDoWhileStmt>(range, condExpr, bodyStmt);
        }

        auto BuildLabelStmt(AstSyntaxRange range, AstExpr* caseExpr) -> AstLabelStmt*
        {
            return CreateAstNode<AstLabelStmt>(range, caseExpr);
        }
        auto BuildSwitchStmt(AstSyntaxRange range, AstExpr* testExpr, AstStmt* bodyStmt) -> AstSwitchStmt*
        {
            return CreateAstNode<AstSwitchStmt>(range, testExpr, bodyStmt);
        }

        auto BuildJumpStmt(AstSyntaxRange range, JumpType jumpType) -> AstJumpStmt*
        {
            return CreateAstNode<AstJumpStmt>(range, jumpType);
        }

        auto BuildReturnStmt(AstSyntaxRange range, AstExpr* expr) -> AstReturnStmt*
        {
            return CreateAstNode<AstReturnStmt>(range, expr);
        }
#pragma endregion

#pragma region Decl

        auto BuildErrorDecl(AstSyntaxRange range) -> AstErrorDecl*
        {
            return CreateAstNode<AstErrorDecl>(range);
        }

        auto BuildEmptyDecl(AstSyntaxRange range) -> AstEmptyDecl*
        {
            return CreateAstNode<AstEmptyDecl>(range);
        }

        auto BuildVariableDecl(AstSyntaxRange range, AstQualType* qualType, std::vector<VariableDeclarator> declarators)
            -> AstVariableDecl*
        {
            auto result = CreateAstNode<AstVariableDecl>(range, qualType, std::move(declarators));

            std::vector<const Type*> variableTypes;
            for (const auto& declarator : result->GetDeclarators()) {
                variableTypes.push_back(
                    compilerObject.GetTypeContext().GetArrayType(qualType->GetResolvedType(), declarator.arraySize));
            }
            result->SetResolvedTypes(std::move(variableTypes));

            symbolTable.AddVariableDecl(*result);
            return result;
        }

        auto BuildStructDecl(AstSyntaxRange range, std::optional<SyntaxToken> declTok,
                             std::vector<AstVariableDecl*> members) -> AstStructDecl*
        {
            auto result = CreateAstNode<AstStructDecl>(range, declTok, std::move(members));

            result->SetDeclaredType(compilerObject.GetTypeContext().CreateStructType(*result));

            symbolTable.AddStructDecl(*result);
            return result;
        }

        auto BuildParamDecl(AstSyntaxRange range, AstQualType* qualType, VariableDeclarator declarator) -> AstParamDecl*
        {
            auto result = CreateAstNode<AstParamDecl>(range, qualType, declarator);

            result->SetResolvedType(
                compilerObject.GetTypeContext().GetArrayType(qualType->GetResolvedType(), declarator.arraySize));

            symbolTable.AddParamDecl(*result);
            return result;
        }
        auto BuildFunctionDecl(AstSyntaxRange range, AstQualType* returnType, SyntaxToken declTok,
                               std::vector<AstParamDecl*> params, AstStmt* body) -> AstFunctionDecl*
        {
            auto result = CreateAstNode<AstFunctionDecl>(range, returnType, declTok, std::move(params), body);

            symbolTable.AddFunctionDecl(*result);
            return result;
        }

        auto BuildInterfaceBlockDecl(AstSyntaxRange range, AstTypeQualifierSeq* quals, SyntaxToken declTok,
                                     std::vector<AstVariableDecl*> members,
                                     std::optional<VariableDeclarator> declarator) -> AstInterfaceBlockDecl*
        {
            auto result =
                CreateAstNode<AstInterfaceBlockDecl>(range, quals, declTok, std::move(members), std::move(declarator));

            auto blockType = compilerObject.GetTypeContext().CreateInterfaceBlockType(*result);
            result->SetResolvedBlockType(blockType);
            if (declarator) {
                result->SetResolvedInstanceType(
                    compilerObject.GetTypeContext().GetArrayType(blockType, declarator->arraySize));
            }
            else {
                result->SetResolvedInstanceType(Type::GetErrorType());
            }

            symbolTable.AddInterfaceBlockDecl(*result);
            return result;
        }

#pragma endregion

    private:
        template <typename T, typename... Args>
            requires std::is_base_of_v<AstNode, T>
        auto CreateAstNode(AstSyntaxRange range, Args&&... args) -> T*
        {
            return compilerObject.GetAstContext().CreateAstNode<T>(compilerObject.GetId(), range,
                                                                   std::forward<Args>(args)...);
        }
    };
} // namespace glsld
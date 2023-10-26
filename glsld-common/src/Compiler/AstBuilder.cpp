#include "Compiler/AstBuilder.h"

namespace glsld
{
#pragma region Misc
    auto AstBuilder::BuildTranslationUnit(AstSyntaxRange range, std::vector<AstDecl*> decls) -> AstTranslationUnit*
    {
        return CreateAstNode<AstTranslationUnit>(range, std::move(decls));
    }

    auto AstBuilder::BuildArraySpec(AstSyntaxRange range, std::vector<AstExpr*> sizes) -> AstArraySpec*
    {
        return CreateAstNode<AstArraySpec>(range, std::move(sizes));
    }

    auto AstBuilder::BuildTypeQualifierSeq(AstSyntaxRange range, QualifierGroup quals,
                                           std::vector<LayoutItem> layoutQuals) -> AstTypeQualifierSeq*
    {
        return CreateAstNode<AstTypeQualifierSeq>(range, quals, std::move(layoutQuals));
    }

    auto AstBuilder::BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, SyntaxToken typeName,
                                   AstArraySpec* arraySpec) -> AstQualType*
    {
        auto result = CreateAstNode<AstQualType>(range, qualifiers, typeName, arraySpec);

        // Set a good default as a fallback.
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

    auto AstBuilder::BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, AstStructDecl* structDecl,
                                   AstArraySpec* arraySpec) -> AstQualType*
    {
        auto result = CreateAstNode<AstQualType>(range, qualifiers, structDecl, arraySpec);

        result->SetResolvedType(compilerObject.GetTypeContext().GetArrayType(structDecl->GetDeclaredType(), arraySpec));
        return result;
    }

    auto AstBuilder::BuildInitializerList(AstSyntaxRange range, std::vector<AstInitializer*> initializers)
        -> AstInitializerList*
    {
        return CreateAstNode<AstInitializerList>(range, std::move(initializers));
    }

#pragma endregion

#pragma region Expr
    auto AstBuilder::BuildErrorExpr(AstSyntaxRange range) -> AstErrorExpr*
    {
        auto result = CreateAstNode<AstErrorExpr>(range);

        result->SetConst(false);
        result->SetDeducedType(Type::GetErrorType());
        return result;
    }

    auto AstBuilder::BuildLiteralExpr(AstSyntaxRange range, ConstValue value) -> AstLiteralExpr*
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
    auto AstBuilder::BuildNameAccessExpr(AstSyntaxRange range, SyntaxToken idToken) -> AstNameAccessExpr*
    {
        auto result = CreateAstNode<AstNameAccessExpr>(range, idToken);

        // Set a good default since the logic is complicated.
        result->SetConst(false);
        result->SetDeducedType(Type::GetErrorType());
        result->SetResolvedDecl({});

        if (idToken.IsIdentifier()) {
            auto symbol = symbolTable.FindSymbol(idToken.text.Str());
            if (auto refInfo = ComputeSymbolReferenceInfo(symbol)) {
                GLSLD_ASSERT(symbol.IsValid());
                result->SetResolvedDecl(symbol);
                result->SetDeducedType(refInfo->type);
                result->SetConst(refInfo->isConst);
            }
        }

        return result;
    }

    auto ParseSwizzle(StringView swizzleName) -> SwizzleDesc
    {
        struct SwizzleCharDesc
        {
            int set;
            int index;
        };
        auto translateSwizzleChar = [](char ch) -> SwizzleCharDesc {
            switch (ch) {
            case 'x':
                return {0, 0};
            case 'y':
                return {0, 1};
            case 'z':
                return {0, 2};
            case 'w':
                return {0, 3};
            case 'r':
                return {1, 0};
            case 'g':
                return {1, 1};
            case 'b':
                return {1, 2};
            case 'a':
                return {1, 3};
            case 's':
                return {2, 0};
            case 't':
                return {2, 1};
            case 'p':
                return {2, 2};
            case 'q':
                return {2, 3};
            default:
                return {-1, 0};
            }
        };

        int lastSwizzleSet = -1;
        uint8_t swizzleBuffer[4];

        for (int i = 0; i < swizzleName.Size(); ++i) {
            const auto [set, index] = translateSwizzleChar(swizzleName[i]);
            if (set == -1) {
                // Bad swizzle char
                return {};
            }
            else if (lastSwizzleSet != -1 && set != lastSwizzleSet) {
                // Swizzle set mismatch
                return {};
            }

            lastSwizzleSet   = set;
            swizzleBuffer[i] = static_cast<uint8_t>(index);
        }

        return SwizzleDesc{ArrayView<uint8_t>{swizzleBuffer, swizzleName.Size()}};
    }

    auto DeduceSwizzleType(const Type* baseType, SwizzleDesc swizzleDesc) -> const Type*
    {
        // Note we don't check if the swizzle is valid here.

        size_t swizzleDim = swizzleDesc.GetDimension();
        if (auto scalarDesc = baseType->GetScalarDesc()) {
            return Type::GetVectorType(scalarDesc->type, swizzleDim);
        }
        else if (auto vectorDesc = baseType->GetVectorDesc()) {
            return Type::GetVectorType(vectorDesc->scalarType, swizzleDim);
        }
        else {
            GLSLD_UNREACHABLE();
        }
    }

    auto AstBuilder::BuildMemberNameAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, SyntaxToken idToken)
        -> AstMemberNameAccessExpr*
    {
        auto result = CreateAstNode<AstMemberNameAccessExpr>(range, baseExpr, idToken);

        // Set a good default first
        result->SetConst(false);
        result->SetDeducedType(Type::GetErrorType());
        result->SetResolvedDecl({});
        result->SetSwizzleDesc({});

        if (idToken.IsIdentifier()) {
            auto baseType   = baseExpr->GetDeducedType();
            auto accessName = idToken.text.StrView();

            if (baseType->IsStruct()) {
                // Struct member access
                // FIXME: implement this
                result->SetConst(baseExpr->IsConst());        // FIXME: but types also matter
                result->SetDeducedType(Type::GetErrorType()); // FIXME: fill this
                result->SetResolvedDecl({});
            }
            else if (baseType->IsScalar() || baseType->IsVector()) {
                // Swizzle
                result->SetConst(baseExpr->IsConst());
                SwizzleDesc swizzleDesc = ParseSwizzle(accessName);
                result->SetDeducedType(DeduceSwizzleType(baseType, swizzleDesc));
                result->SetSwizzleDesc(swizzleDesc);
            }
        }

        return result;
    }
    auto AstBuilder::BuildIndexAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, AstArraySpec* indices)
        -> AstIndexAccessExpr*
    {
        auto result = CreateAstNode<AstIndexAccessExpr>(range, baseExpr, indices);

        if (baseExpr->IsConst()) {
            bool isConst = true;
            for (auto indexExpr : indices->GetSizeList()) {
                if (!indexExpr->IsConst()) {
                    isConst = false;
                    break;
                }
            }

            result->SetConst(isConst);
        }
        else {
            result->SetConst(false);
        }

        auto baseType = baseExpr->GetDeducedType();
        if (auto arrayDesc = baseType->GetArrayDesc()) {
            result->SetDeducedType(arrayDesc->elementType);
        }
        else if (auto vectorDesc = baseType->GetVectorDesc()) {
            result->SetDeducedType(Type::GetScalarType(vectorDesc->scalarType));
        }
        else if (auto matrixDesc = baseType->GetMatrixDesc()) {
            // FIXME: row vector? column vector?
            result->SetDeducedType(Type::GetVectorType(matrixDesc->scalarType, matrixDesc->dimCol));
        }
        else {
            result->SetDeducedType(Type::GetErrorType());
        }
        return result;
    }

    auto AstBuilder::BuildUnaryExpr(AstSyntaxRange range, AstExpr* operand, UnaryOp opcode) -> AstUnaryExpr*
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
    auto AstBuilder::BuildBinaryExpr(AstSyntaxRange range, AstExpr* lhs, AstExpr* rhs, BinaryOp opcode)
        -> AstBinaryExpr*
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
    auto AstBuilder::BuildSelectExpr(AstSyntaxRange range, AstExpr* condExpr, AstExpr* trueExpr, AstExpr* falseExpr)
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
    auto AstBuilder::BuildInvokeExpr(AstSyntaxRange range, SyntaxToken functionName, std::vector<AstExpr*> args)
        -> AstFunctionCallExpr*
    {
        // FIXME: do we need fix up AST for constructor call?
        auto result = CreateAstNode<AstFunctionCallExpr>(range, functionName, std::move(args));

        // Set a good default since the logic is complicated.
        result->SetConst(false);
        result->SetDeducedType(Type::GetErrorType());
        result->SetResolvedFunction(nullptr);

        if (functionName.IsIdentifier()) {
            std::vector<const Type*> argTypes;
            for (auto arg : result->GetArgs()) {
                argTypes.push_back(arg->GetDeducedType());
            }

            // TODO: if the function name could uniquely identify a function, we can skip overload resolution
            //       this could help make the resolution more resilient to errors while typing
            auto function = symbolTable.FindFunction(functionName.text.Str(), argTypes);
            if (function) {
                result->SetResolvedFunction(function);
                result->SetDeducedType(function->GetReturnType()->GetResolvedType());
                // TODO: result of some function call can be const
                result->SetConst(false);
            }
        }

        return result;
    }
    auto AstBuilder::BuildConstructorExpr(AstSyntaxRange range, AstQualType* qualType, std::vector<AstExpr*> args)
        -> AstConstructorCallExpr*
    {
        auto result = CreateAstNode<AstConstructorCallExpr>(range, qualType, std::move(args));

        // FIXME: implement this.
        result->SetConst(false);
        result->SetDeducedType(qualType->GetResolvedType());
        return result;
    }
#pragma endregion

#pragma region Stmt
    auto AstBuilder::BuildErrorStmt(AstSyntaxRange range) -> AstErrorStmt*
    {
        return CreateAstNode<AstErrorStmt>(range);
    }

    auto AstBuilder::BuildEmptyStmt(AstSyntaxRange range) -> AstEmptyStmt*
    {
        return CreateAstNode<AstEmptyStmt>(range);
    }

    auto AstBuilder::BuildCompoundStmt(AstSyntaxRange range, std::vector<AstStmt*> stmts) -> AstCompoundStmt*
    {
        return CreateAstNode<AstCompoundStmt>(range, std::move(stmts));
    }

    auto AstBuilder::BuildExprStmt(AstSyntaxRange range, AstExpr* expr) -> AstExprStmt*
    {
        return CreateAstNode<AstExprStmt>(range, expr);
    }

    auto AstBuilder::BuildDeclStmt(AstSyntaxRange range, AstDecl* decl) -> AstDeclStmt*
    {
        return CreateAstNode<AstDeclStmt>(range, decl);
    }

    auto AstBuilder::BuildIfStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* thenStmt) -> AstIfStmt*
    {
        return CreateAstNode<AstIfStmt>(range, condExpr, thenStmt, nullptr);
    }
    auto AstBuilder::BuildIfStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* thenStmt, AstStmt* elseStmt)
        -> AstIfStmt*
    {
        return CreateAstNode<AstIfStmt>(range, condExpr, thenStmt, elseStmt);
    }

    auto AstBuilder::BuildForStmt(AstSyntaxRange range, AstStmt* initStmt, AstExpr* condExpr, AstExpr* iterExpr,
                                  AstStmt* bodyStmt) -> AstForStmt*
    {
        return CreateAstNode<AstForStmt>(range, initStmt, condExpr, iterExpr, bodyStmt);
    }

    auto AstBuilder::BuildWhileStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* bodyStmt) -> AstWhileStmt*
    {
        return CreateAstNode<AstWhileStmt>(range, condExpr, bodyStmt);
    }

    auto AstBuilder::BuildDoWhileStmt(AstSyntaxRange range, AstExpr* condExpr, AstStmt* bodyStmt) -> AstDoWhileStmt*
    {
        return CreateAstNode<AstDoWhileStmt>(range, condExpr, bodyStmt);
    }

    auto AstBuilder::BuildLabelStmt(AstSyntaxRange range, AstExpr* caseExpr) -> AstLabelStmt*
    {
        return CreateAstNode<AstLabelStmt>(range, caseExpr);
    }
    auto AstBuilder::BuildSwitchStmt(AstSyntaxRange range, AstExpr* testExpr, AstStmt* bodyStmt) -> AstSwitchStmt*
    {
        return CreateAstNode<AstSwitchStmt>(range, testExpr, bodyStmt);
    }

    auto AstBuilder::BuildJumpStmt(AstSyntaxRange range, JumpType jumpType) -> AstJumpStmt*
    {
        return CreateAstNode<AstJumpStmt>(range, jumpType);
    }

    auto AstBuilder::BuildReturnStmt(AstSyntaxRange range, AstExpr* expr) -> AstReturnStmt*
    {
        return CreateAstNode<AstReturnStmt>(range, expr);
    }
#pragma endregion

#pragma region Decl

    auto AstBuilder::BuildErrorDecl(AstSyntaxRange range) -> AstErrorDecl*
    {
        return CreateAstNode<AstErrorDecl>(range);
    }

    auto AstBuilder::BuildEmptyDecl(AstSyntaxRange range) -> AstEmptyDecl*
    {
        return CreateAstNode<AstEmptyDecl>(range);
    }

    auto AstBuilder::BuildVariableDecl(AstSyntaxRange range, AstQualType* qualType,
                                       std::vector<VariableDeclarator> declarators) -> AstVariableDecl*
    {
        auto result = CreateAstNode<AstVariableDecl>(range, qualType, std::move(declarators));

        std::vector<const Type*> variableTypes;
        for (const auto& declarator : result->GetDeclarators()) {
            variableTypes.push_back(
                compilerObject.GetTypeContext().GetArrayType(qualType->GetResolvedType(), declarator.arraySize));
        }
        result->SetResolvedTypes(std::move(variableTypes));

        symbolTable.GetCurrentLevel()->AddVariableDecl(*result);
        return result;
    }

    auto AstBuilder::BuildStructDecl(AstSyntaxRange range, std::optional<SyntaxToken> declTok,
                                     std::vector<AstVariableDecl*> members) -> AstStructDecl*
    {
        auto result = CreateAstNode<AstStructDecl>(range, declTok, std::move(members));

        result->SetDeclaredType(compilerObject.GetTypeContext().CreateStructType(*result));

        symbolTable.GetCurrentLevel()->AddStructDecl(*result);
        return result;
    }

    auto AstBuilder::BuildParamDecl(AstSyntaxRange range, AstQualType* qualType, VariableDeclarator declarator)
        -> AstParamDecl*
    {
        auto result = CreateAstNode<AstParamDecl>(range, qualType, declarator);

        result->SetResolvedType(
            compilerObject.GetTypeContext().GetArrayType(qualType->GetResolvedType(), declarator.arraySize));

        symbolTable.GetCurrentLevel()->AddParamDecl(*result);
        return result;
    }
    auto AstBuilder::BuildFunctionDecl(AstSyntaxRange range, AstQualType* returnType, SyntaxToken declTok,
                                       std::vector<AstParamDecl*> params, AstStmt* body) -> AstFunctionDecl*
    {
        auto result = CreateAstNode<AstFunctionDecl>(range, returnType, declTok, std::move(params), body);

        symbolTable.GetCurrentLevel()->AddFunctionDecl(*result);
        return result;
    }

    auto AstBuilder::BuildInterfaceBlockDecl(AstSyntaxRange range, AstTypeQualifierSeq* quals, SyntaxToken declTok,
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

        symbolTable.GetCurrentLevel()->AddInterfaceBlockDecl(*result);
        return result;
    }

#pragma endregion
} // namespace glsld
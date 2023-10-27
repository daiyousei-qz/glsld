#include "Compiler/AstBuilder.h"

namespace glsld
{
    struct SymbolReferenceInfo
    {
        const Type* type;

        bool isConst;
    };

    auto ComputeSymbolReferenceInfo(DeclView declView) -> std::optional<SymbolReferenceInfo>
    {
        if (!declView.IsValid()) {
            return std::nullopt;
        }

        if (auto variableDecl = declView.GetDecl()->As<AstVariableDecl>()) {
            // TODO: should we also check if initializer is const?
            auto type    = variableDecl->GetResolvedTypes()[declView.GetIndex()];
            bool isConst = false;
            if (auto qualifiers = variableDecl->GetQualType()->GetQualifiers()) {
                isConst = qualifiers->GetQualGroup().qConst;
            }

            return SymbolReferenceInfo{.type = type, .isConst = isConst};
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

        // Note anything with error type is not const
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
                result->SetConst(refInfo->isConst);
                result->SetDeducedType(refInfo->type);
                result->SetResolvedDecl(symbol);
            }
        }

        return result;
    }

    auto ParseSwizzle(StringView swizzleName) -> SwizzleDesc
    {
        GLSLD_ASSERT(!swizzleName.Empty());
        if (swizzleName.Size() > 4) {
            // Swizzle name too long
            return {};
        }

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
        GLSLD_ASSERT(baseType->IsScalar() || baseType->IsVector());
        if (!swizzleDesc.IsValid()) {
            return Type::GetErrorType();
        }

        // Note we don't further check if the swizzle is out-of-bound here.
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

    auto AstBuilder::BuildDotAccessExpr(AstSyntaxRange range, AstExpr* lhsExpr, SyntaxToken idToken) -> AstExpr*
    {
        auto baseType = lhsExpr->GetDeducedType();
        if (baseType->IsScalar() || baseType->IsVector()) {
            // This is swizzle access
            auto result = CreateAstNode<AstSwizzleAccessExpr>(range, lhsExpr, idToken);

            auto swizzleDesc = SwizzleDesc{};
            if (idToken.IsIdentifier()) {
                swizzleDesc = ParseSwizzle(idToken.text.StrView());
            }

            result->SetConst(lhsExpr->IsConst());
            result->SetDeducedType(DeduceSwizzleType(baseType, swizzleDesc));
            result->SetSwizzleDesc(swizzleDesc);
            return result;
        }
        else {
            // This is struct member access
            auto result = CreateAstNode<AstFieldAccessExpr>(range, lhsExpr, idToken);

            // Set a good default first
            result->SetConst(false);
            result->SetDeducedType(Type::GetErrorType());
            result->SetResolvedDecl({});

            if (idToken.IsIdentifier()) {
                if (auto structRecord = compilerObject.GetTypeContext().FindStructTypeDecl(baseType)) {
                    if (auto it = structRecord->memberLookup.find(idToken.text.Str());
                        it != structRecord->memberLookup.end()) {
                        auto memberDecl = it->second;
                        GLSLD_ASSERT(memberDecl.IsValid());

                        result->SetConst(lhsExpr->IsConst()); // FIXME: but types also matter
                        result->SetDeducedType(
                            memberDecl.GetDecl()->As<AstVariableDecl>()->GetResolvedTypes()[memberDecl.GetIndex()]);
                        result->SetResolvedDecl(memberDecl);
                    }
                }
            }
            return result;
        }
    }

    static auto DeduceIndexAccessType(const Type* baseType, size_t dimToUnwrap) -> const Type*
    {
        if (baseType->IsError() || dimToUnwrap == 0) {
            return baseType;
        }

        if (auto arrayDesc = baseType->GetArrayDesc()) {
            return DeduceIndexAccessType(arrayDesc->elementType, dimToUnwrap - 1);
        }
        else if (auto vectorDesc = baseType->GetVectorDesc()) {
            if (dimToUnwrap == 1) {
                return Type::GetScalarType(vectorDesc->scalarType);
            }
        }
        else if (auto matrixDesc = baseType->GetMatrixDesc()) {
            if (dimToUnwrap == 2) {
                return Type::GetScalarType(matrixDesc->scalarType);
            }
            else if (dimToUnwrap == 1) {
                // Note we are indexing to a column vector here.
                return Type::GetVectorType(matrixDesc->scalarType, matrixDesc->dimCol);
            }
        }

        return Type::GetErrorType();
    }

    auto AstBuilder::BuildIndexAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, AstArraySpec* indices)
        -> AstIndexAccessExpr*
    {
        auto result = CreateAstNode<AstIndexAccessExpr>(range, baseExpr, indices);

        result->SetDeducedType(DeduceIndexAccessType(baseExpr->GetDeducedType(), indices->GetSizeList().size()));
        if (baseExpr->IsConst() && !result->GetDeducedType()->IsError()) {
            bool isConst = true;
            for (auto indexExpr : indices->GetSizeList()) {
                if (!indexExpr->IsConst()) {
                    isConst = false;
                    break;
                }
            }

            result->SetConst(isConst);
        }
        return result;
    }

    static auto DeduceUnaryExprType(UnaryOp op, const Type* operandType) -> const Type*
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
            if (operandType->IsArray() || operandType->IsVector() || operandType->IsMatrix()) {
                return Type::GetBuiltinType(GlslBuiltinType::Ty_int);
            }
            else {
                // FIXME: are there any other cases?
                return Type::GetErrorType();
            }
        }

        GLSLD_UNREACHABLE();
    }

    static auto IsUnaryExprConst(UnaryOp op, const Type* operandType) -> bool
    {
        // FIXME: implement this.
        return false;
    }

    auto AstBuilder::BuildUnaryExpr(AstSyntaxRange range, AstExpr* operand, UnaryOp opcode) -> AstUnaryExpr*
    {
        auto result = CreateAstNode<AstUnaryExpr>(range, operand, opcode);

        result->SetConst(false);
        result->SetDeducedType(DeduceUnaryExprType(opcode, operand->GetDeducedType()));

        if (!result->GetDeducedType()->IsError()) {
            if (operand->IsConst() && IsUnaryExprConst(opcode, operand->GetDeducedType())) {
                result->SetConst(true);
            }
        }
        return result;
    }

    static auto DeduceBinaryExprType(BinaryOp op, const Type* lhsType, const Type* rhsType) -> const Type*
    {
        // FIXME: implement this
        if (lhsType->IsSameWith(rhsType)) {
            return lhsType;
        }
        else {
            return Type::GetErrorType();
        }
    }

    auto AstBuilder::BuildBinaryExpr(AstSyntaxRange range, AstExpr* lhs, AstExpr* rhs, BinaryOp opcode)
        -> AstBinaryExpr*
    {
        auto result = CreateAstNode<AstBinaryExpr>(range, lhs, rhs, opcode);

        result->SetConst(false);
        result->SetDeducedType(DeduceBinaryExprType(opcode, lhs->GetDeducedType(), rhs->GetDeducedType()));

        if (!result->GetDeducedType()->IsError()) {
            if (lhs->IsConst() && rhs->IsConst() && !(opcode == BinaryOp::Comma || IsAssignmentOp(opcode))) {
                result->SetConst(true);
            }
        }
        return result;
    }

    auto AstBuilder::BuildSelectExpr(AstSyntaxRange range, AstExpr* condExpr, AstExpr* trueExpr, AstExpr* falseExpr)
        -> AstSelectExpr*
    {
        auto result = CreateAstNode<AstSelectExpr>(range, condExpr, trueExpr, falseExpr);

        result->SetConst(condExpr->IsConst() && trueExpr->IsConst() && falseExpr->IsConst());
        if (trueExpr->GetDeducedType()->IsSameWith(falseExpr->GetDeducedType())) {
            result->SetDeducedType(trueExpr->GetDeducedType());
        }
        else {
            result->SetDeducedType(Type::GetErrorType());
        }
        return result;
    }

    auto AstBuilder::BuildFuntionCallExpr(AstSyntaxRange range, SyntaxToken functionName, std::vector<AstExpr*> args)
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

    auto AstBuilder::BuildConstructorCallExpr(AstSyntaxRange range, AstQualType* qualType, std::vector<AstExpr*> args)
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
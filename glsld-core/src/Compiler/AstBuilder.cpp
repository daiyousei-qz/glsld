#include "Compiler/AstBuilder.h"
#include "Ast/Eval.h"
#include "Ast/Expr.h"
#include "Ast/Type.h"
#include "Basic/Common.h"
#include "Language/Typing.h"

#include <algorithm>
#include <cstddef>

namespace glsld
{

#pragma region Misc
    auto AstBuilder::BuildTranslationUnit(AstSyntaxRange range, std::vector<AstDecl*> decls) -> AstTranslationUnit*
    {
        return CreateAstNode<AstTranslationUnit>(range, CopyArray(decls));
    }

    auto AstBuilder::BuildArraySpec(AstSyntaxRange range, std::vector<AstExpr*> sizes) -> AstArraySpec*
    {
        for (auto& sizeExpr : sizes) {
            if (sizeExpr) {
                sizeExpr = TryMakeImplicitCast(sizeExpr, Type::GetBuiltinType(GlslBuiltinType::Ty_int));
            }
        }

        return CreateAstNode<AstArraySpec>(range, CopyArray(sizes));
    }

    auto AstBuilder::BuildTypeQualifierSeq(AstSyntaxRange range, QualifierGroup quals,
                                           std::vector<LayoutItem> layoutQuals) -> AstTypeQualifierSeq*
    {
        return CreateAstNode<AstTypeQualifierSeq>(range, quals, CopyArray(layoutQuals));
    }

    auto AstBuilder::BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, AstSyntaxToken typeName,
                                   AstArraySpec* arraySpec) -> AstQualType*
    {
        auto result = CreateAstNode<AstQualType>(range, qualifiers, typeName, arraySpec);

        // Set a good default as a fallback.
        result->SetResolvedType(Type::GetErrorType());

        if (auto glslType = GetGlslBuiltinType(typeName.klass)) {
            result->SetResolvedType(astContext.GetArrayType(Type::GetBuiltinType(*glslType), arraySpec));
        }
        else if (typeName.IsIdentifier()) {
            if (auto symbolDecl = symbolTable.FindSymbol(typeName.text.Str()); symbolDecl) {
                if (auto structDecl = symbolDecl->As<AstStructDecl>()) {
                    result->SetResolvedType(astContext.GetArrayType(structDecl->GetDeclaredType(), arraySpec));
                }
            }
        }
        return result;
    }

    auto AstBuilder::BuildQualType(AstSyntaxRange range, AstTypeQualifierSeq* qualifiers, AstStructDecl* structDecl,
                                   AstArraySpec* arraySpec) -> AstQualType*
    {
        auto result = CreateAstNode<AstQualType>(range, qualifiers, structDecl, arraySpec);

        result->SetResolvedType(astContext.GetArrayType(structDecl->GetDeclaredType(), arraySpec));
        return result;
    }

    auto AstBuilder::BuildInitializerList(AstSyntaxRange range, std::vector<AstInitializer*> initializers,
                                          const Type* contextType) -> AstInitializerList*
    {
        // Make implicit cast if needed
        if (contextType->IsArithmetic()) {
            // Initializer list initialize an arithmetic type component-wisely
            for (size_t i = 0; i < initializers.size(); ++i) {
                if (initializers[i]->GetDeducedType()->IsArithmetic()) {
                    initializers[i] = TryMakeImplicitCast(
                        initializers[i], Type::GetArithmeticType(*contextType->GetElementScalarKind(),
                                                                 initializers[i]->GetDeducedType()->GetDimension()));
                }
            }
        }
        else if (contextType->IsStruct() || contextType->IsArray()) {
            for (size_t i = 0; i < initializers.size(); ++i) {
                initializers[i] = TryMakeImplicitCast(initializers[i], contextType->GetComponentType(i));
            }
        }

        // Infer implicit array size if needed
        // Notably, for implicitly sized array, only outermost dimension could be unsized.
        const Type* resolvedType = contextType;
        if (auto arrayDesc = resolvedType->GetArrayDesc(); arrayDesc && arrayDesc->dimSize == 0) {
            resolvedType = astContext.GetArrayType(arrayDesc->elementType, initializers.size());
        }

        auto result = CreateAstNode<AstInitializerList>(range, CopyArray(initializers));
        result->SetConst(std::ranges::all_of(initializers, [](const AstInitializer* init) { return init->IsConst(); }));
        result->SetDeducedType(resolvedType);
        return result;
    }

#pragma endregion

#pragma region Expr
    auto AstBuilder::BuildErrorExpr(AstSyntaxRange range) -> AstErrorExpr*
    {
        auto result = CreateAstNode<AstErrorExpr>(range);

        // Note anything with error type is const.
        result->SetConst(true);
        result->SetDeducedType(Type::GetErrorType());
        return result;
    }

    auto AstBuilder::BuildLiteralExpr(AstSyntaxRange range, ConstValue value) -> AstLiteralExpr*
    {
        auto result = CreateAstNode<AstLiteralExpr>(range, std::move(value));

        if (auto glslType = result->GetValue().GetGlslType()) {
            result->SetConst(true);
            result->SetDeducedType(Type::GetBuiltinType(*glslType));
        }
        else {
            result->SetConst(false);
            result->SetDeducedType(Type::GetErrorType());
        }
        return result;
    }

    auto AstBuilder::BuildNameAccessExpr(AstSyntaxRange range, AstSyntaxToken idToken) -> AstNameAccessExpr*
    {
        auto result = CreateAstNode<AstNameAccessExpr>(range, idToken);

        // Set a good default since the logic is complicated.
        result->SetConst(false);
        result->SetDeducedType(Type::GetErrorType());
        result->SetResolvedDecl(nullptr);

        if (idToken.IsIdentifier()) {
            if (auto symbolDecl = symbolTable.FindSymbol(idToken.text.Str())) {
                if (auto variableDeclaratorDecl = symbolDecl->As<AstVariableDeclaratorDecl>()) {
                    // TODO: should we also check if initializer is const?
                    result->SetConst(variableDeclaratorDecl->IsConstVariable());
                    result->SetDeducedType(variableDeclaratorDecl->GetResolvedType());
                }
                else if (auto paramDecl = symbolDecl->As<AstParamDecl>()) {
                    result->SetConst(false);
                    result->SetDeducedType(paramDecl->GetResolvedType());
                }
                else if (auto interfaceBlockDecl = symbolDecl->As<AstInterfaceBlockDecl>()) {
                    GLSLD_ASSERT(interfaceBlockDecl->GetDeclarator().has_value());
                    result->SetConst(false);
                    result->SetDeducedType(interfaceBlockDecl->GetResolvedInstanceType());
                }
                else if (auto structDecl = symbolDecl->As<AstStructDecl>()) {
                    result->SetConst(false);
                    result->SetDeducedType(Type::GetErrorType());
                }
                else if (auto fieldDecl = symbolDecl->As<AstBlockFieldDeclaratorDecl>()) {
                    // TODO: GLSLD_ASSERT(fieldDecl->GetScope() == DeclScope::Global);
                    result->SetConst(false);
                    result->SetDeducedType(fieldDecl->GetResolvedType());
                }

                result->SetResolvedDecl(symbolDecl);
            }
        }

        return result;
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
            return swizzleDim == 1 ? Type::GetScalarType(scalarDesc->type)
                                   : Type::GetVectorType(scalarDesc->type, swizzleDim);
        }
        else if (auto vectorDesc = baseType->GetVectorDesc()) {
            return swizzleDim == 1 ? Type::GetScalarType(vectorDesc->scalarType)
                                   : Type::GetVectorType(vectorDesc->scalarType, swizzleDim);
        }
        else {
            GLSLD_UNREACHABLE();
        }
    }

    auto AstBuilder::BuildDotAccessExpr(AstSyntaxRange range, AstExpr* lhsExpr, AstSyntaxToken idToken) -> AstExpr*
    {
        auto baseType = lhsExpr->GetDeducedType();
        if (baseType->IsScalar() || baseType->IsVector()) {
            // This is swizzle access
            auto result = CreateAstNode<AstSwizzleAccessExpr>(range, lhsExpr, idToken);

            auto swizzleDesc = SwizzleDesc{};
            if (idToken.IsIdentifier()) {
                swizzleDesc = SwizzleDesc::Parse(idToken.text.StrView());
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
                if (auto structDesc = baseType->GetStructDesc()) {
                    if (auto memberDesc = structDesc->FindMember(idToken.text.Str()); memberDesc) {
                        GLSLD_ASSERT(memberDesc->decl);
                        result->SetConst(lhsExpr->IsConst()); // FIXME: but types also matter
                        result->SetDeducedType(memberDesc->type);
                        result->SetResolvedDecl(memberDesc->decl);
                    }
                }
            }
            return result;
        }
    }

    static auto DeduceIndexAccessType(const Type* baseType) -> const Type*
    {
        if (baseType->IsError()) {
            return baseType;
        }

        if (auto arrayDesc = baseType->GetArrayDesc()) {
            return arrayDesc->elementType;
        }
        else if (auto vectorDesc = baseType->GetVectorDesc()) {
            return Type::GetScalarType(vectorDesc->scalarType);
        }
        else if (auto matrixDesc = baseType->GetMatrixDesc()) {
            // Note we are indexing to a column vector here.
            return Type::GetVectorType(matrixDesc->scalarType, matrixDesc->dimCol);
        }

        return Type::GetErrorType();
    }

    auto AstBuilder::BuildIndexAccessExpr(AstSyntaxRange range, AstExpr* baseExpr, AstExpr* indexExpr)
        -> AstIndexAccessExpr*
    {
        indexExpr   = TryMakeImplicitCast(indexExpr, Type::GetBuiltinType(GlslBuiltinType::Ty_int));
        auto result = CreateAstNode<AstIndexAccessExpr>(range, baseExpr, indexExpr);

        result->SetDeducedType(DeduceIndexAccessType(baseExpr->GetDeducedType()));
        result->SetConst(baseExpr->IsConst() && indexExpr->IsConst());
        return result;
    }

    static auto DeduceUnaryExprType(UnaryOp op, const Type* operandType) -> const Type*
    {
        if (operandType->IsError()) {
            return Type::GetErrorType();
        }

        switch (op) {
        case UnaryOp::Identity:
        case UnaryOp::Negate:
        case UnaryOp::PrefixInc:
        case UnaryOp::PrefixDec:
        case UnaryOp::PostfixInc:
        case UnaryOp::PostfixDec:
            if (operandType->IsArithmetic()) {
                return operandType;
            }
            break;

        case UnaryOp::BitwiseNot:
            if (operandType->IsIntegral()) {
                return operandType;
            }
            break;

        case UnaryOp::LogicalNot:
            if (operandType->IsScalarBool()) {
                return operandType;
            }
            break;

        case UnaryOp::Length:
            if (operandType->IsArray() || operandType->IsVector() || operandType->IsMatrix()) {
                return Type::GetBuiltinType(GlslBuiltinType::Ty_int);
            }
            break;
        }

        return Type::GetErrorType();
    }

    auto AstBuilder::BuildUnaryExpr(AstSyntaxRange range, AstExpr* operand, UnaryOp opcode) -> AstUnaryExpr*
    {
        const Type* operandType = operand->GetDeducedType();

        bool isConst            = false;
        const Type* deducedType = DeduceUnaryExprType(opcode, operandType);

        if (opcode == UnaryOp::Length) {
            if (operandType->IsVector() || operandType->IsMatrix()) {
                isConst = true;
            }
            else if (auto desc = operandType->GetArrayDesc()) {
                isConst = desc->dimSize != 0;
            }
        }
        else if (!deducedType->IsError() && operand->IsConst()) {
            isConst = true;
        }

        auto result = CreateAstNode<AstUnaryExpr>(range, operand, opcode);
        result->SetConst(isConst);
        result->SetDeducedType(deducedType);
        return result;
    }

    struct BinaryExprTypeInfo
    {
        const Type* deducedType;
        const Type* lhsContextType;
        const Type* rhsContextType;
    };

    static auto DeduceBinaryExprType(BinaryOp op, const Type* lhsType, const Type* rhsType) -> BinaryExprTypeInfo;

    // Either:
    // 1. lhs is scalar, rhs is scalar
    // 2. lhs is vector, rhs is vector
    static auto DeduceComponentWiseBinaryExprType(BinaryOp op, const Type* lhsType, const Type* rhsType)
        -> BinaryExprTypeInfo
    {
        BinaryExprTypeInfo result{
            .deducedType    = Type::GetErrorType(),
            .lhsContextType = lhsType,
            .rhsContextType = rhsType,
        };

        if (lhsType->IsVector()) {
            GLSLD_ASSERT(rhsType->IsVector());
            auto lhsDim = lhsType->GetVectorDesc()->vectorSize;
            auto rhsDim = rhsType->GetVectorDesc()->vectorSize;

            // If the vector sizes are different, we can't do component-wise operation.
            // Deduces to error type.
            if (lhsDim != rhsDim) {
                return result;
            }
        }

        auto deduceWithConversion = [&]() {
            if (lhsType->IsSameWith(rhsType)) {
                result.deducedType = lhsType;
            }
            else if (lhsType->IsConvertibleTo(rhsType)) {
                GLSLD_ASSERT(!rhsType->IsConvertibleTo(lhsType));
                result.deducedType    = rhsType;
                result.lhsContextType = rhsType;
            }
            else if (rhsType->IsConvertibleTo(lhsType)) {
                GLSLD_ASSERT(!lhsType->IsConvertibleTo(rhsType));
                result.deducedType    = lhsType;
                result.rhsContextType = lhsType;
            }
        };

        // Upon this point, we could ensure that lhs and rhs are of the same dimension.
        switch (op) {
        case BinaryOp::Comma:
        case BinaryOp::Assign:
        case BinaryOp::AddAssign:
        case BinaryOp::SubAssign:
        case BinaryOp::MulAssign:
        case BinaryOp::DivAssign:
        case BinaryOp::ModAssign:
        case BinaryOp::LShiftAssign:
        case BinaryOp::RShiftAssign:
        case BinaryOp::AndAssign:
        case BinaryOp::XorAssign:
        case BinaryOp::OrAssign:
            GLSLD_ASSERT(false && "Comma/Assignment should have been handled earlier.");
            break;

        case BinaryOp::Plus:
        case BinaryOp::Minus:
        case BinaryOp::Mul:
        case BinaryOp::Div:
            deduceWithConversion();
            break;

        case BinaryOp::Modulo:
            if (lhsType->IsIntegral() && rhsType->IsIntegral()) {
                deduceWithConversion();
            }
            break;

        case BinaryOp::Equal:
        case BinaryOp::NotEqual:
            if (lhsType->IsEqualComparableTo(rhsType)) {
                deduceWithConversion();
                result.deducedType = Type::GetBuiltinType(GlslBuiltinType::Ty_bool);
            }
            break;

        case BinaryOp::Less:
        case BinaryOp::LessEq:
        case BinaryOp::Greater:
        case BinaryOp::GreaterEq:
            if (lhsType->IsOrderingComparableTo(rhsType)) {
                deduceWithConversion();
                result.deducedType = Type::GetBuiltinType(GlslBuiltinType::Ty_bool);
            }
            break;

        case BinaryOp::BitwiseAnd:
        case BinaryOp::BitwiseOr:
        case BinaryOp::BitwiseXor:
            if (lhsType->IsIntegral() && rhsType->IsIntegral()) {
                deduceWithConversion();
            }
            break;

        case BinaryOp::LogicalAnd:
        case BinaryOp::LogicalOr:
        case BinaryOp::LogicalXor:
            if (lhsType->IsScalarBool() && rhsType->IsScalarBool()) {
                result.deducedType = Type::GetBuiltinType(GlslBuiltinType::Ty_bool);
            }
            break;

        case BinaryOp::ShiftLeft:
        case BinaryOp::ShiftRight:
            if (lhsType->IsIntegral() && rhsType->IsIntegral()) {
                result.deducedType = lhsType;
            }
            break;
        }

        return result;
    }

    // Either:
    // 1. lhs is scalar, rhs is vector/matrix
    // 2. lhs is vector/matrix, rhs is scalar
    static auto DeduceScalarBroadcastBinaryExprType(BinaryOp op, const Type* lhsType, const Type* rhsType)
        -> BinaryExprTypeInfo
    {
        GLSLD_ASSERT((lhsType->IsScalar() && (rhsType->IsVector() || rhsType->IsMatrix())) ||
                     (rhsType->IsScalar() && (lhsType->IsVector() || lhsType->IsMatrix())));

        const Type* scalarType        = lhsType->IsScalar() ? lhsType : rhsType;
        const Type* compositeType     = lhsType->IsScalar() ? rhsType : lhsType;
        const Type* compositeElemType = Type::GetScalarType(*compositeType->GetElementScalarKind());
        GLSLD_ASSERT(compositeType->IsVector() || compositeType->IsMatrix());

        const Type* deducedType          = Type::GetErrorType();
        const Type* scalarContextType    = scalarType;
        const Type* compositeContextType = compositeType;

        auto deduceWithElemConversion = [&]() {
            if (scalarType->IsConvertibleTo(compositeElemType)) {
                deducedType       = compositeType;
                scalarContextType = compositeElemType;
            }
            else if (compositeElemType->IsConvertibleTo(scalarType)) {
                // Note conversion on the vector/matrix type is also allowed.
                auto scalarKind = scalarType->GetScalarDesc()->type;
                if (auto desc = compositeType->GetVectorDesc()) {
                    compositeContextType = Type::GetVectorType(scalarKind, desc->vectorSize);
                }
                else if (auto desc = compositeType->GetMatrixDesc()) {
                    compositeContextType = Type::GetMatrixType(scalarKind, desc->dimRow, desc->dimCol);
                }
                deducedType = compositeContextType;
            }
        };

        switch (op) {
        case BinaryOp::Comma:
        case BinaryOp::Assign:
        case BinaryOp::AddAssign:
        case BinaryOp::SubAssign:
        case BinaryOp::MulAssign:
        case BinaryOp::DivAssign:
        case BinaryOp::ModAssign:
        case BinaryOp::LShiftAssign:
        case BinaryOp::RShiftAssign:
        case BinaryOp::AndAssign:
        case BinaryOp::XorAssign:
        case BinaryOp::OrAssign:
            GLSLD_ASSERT(false && "Comma/Assignment should have been handled earlier.");
            break;

        case BinaryOp::Plus:
        case BinaryOp::Minus:
        case BinaryOp::Mul:
        case BinaryOp::Div:
            deduceWithElemConversion();
            break;

        case BinaryOp::Modulo:
            if (scalarType->IsIntegral() && compositeType->IsIntegral() && compositeType->IsVector()) {
                deduceWithElemConversion();
            }
            break;

        case BinaryOp::Equal:
        case BinaryOp::NotEqual:
        case BinaryOp::Less:
        case BinaryOp::LessEq:
        case BinaryOp::Greater:
        case BinaryOp::GreaterEq:
            break;

        case BinaryOp::BitwiseAnd:
        case BinaryOp::BitwiseOr:
        case BinaryOp::BitwiseXor:
            if (scalarType->IsIntegral() && compositeType->IsIntegral() && compositeType->IsVector()) {
                deduceWithElemConversion();
            }
            break;

        case BinaryOp::LogicalAnd:
        case BinaryOp::LogicalOr:
        case BinaryOp::LogicalXor:
            break;

        case BinaryOp::ShiftLeft:
        case BinaryOp::ShiftRight:
            if (lhsType->IsVector() && lhsType->IsIntegral() && rhsType->IsScalar() && rhsType->IsIntegral()) {
                deducedType = compositeType;
            }
            break;
        }

        return BinaryExprTypeInfo{
            .deducedType    = deducedType,
            .lhsContextType = lhsType->IsScalar() ? scalarContextType : compositeContextType,
            .rhsContextType = lhsType->IsScalar() ? compositeContextType : scalarContextType,
        };
    }

    // Either:
    // 1. lhs is matrix, rhs is vector
    // 2. lhs is vector, rhs is matrix
    // 3. lhs is matrix, rhs is matrix
    static auto DeduceMatrixOpBinaryExprType(BinaryOp op, const Type* lhsType, const Type* rhsType)
        -> BinaryExprTypeInfo
    {
        GLSLD_ASSERT((op != BinaryOp::Comma && !IsAssignmentOp(op)) &&
                     "Comma/Assignment should have been handled earlier.");

        const Type* deducedType    = Type::GetErrorType();
        const Type* lhsContextType = lhsType;
        const Type* rhsContextType = rhsType;

        auto lhsElemType = Type::GetScalarType(*lhsType->GetElementScalarKind());
        auto rhsElemType = Type::GetScalarType(*rhsType->GetElementScalarKind());
        if (op == BinaryOp::Mul) {
            // This is matrix multiplication
            const Type* resultElemType;
            if (lhsElemType->IsSameWith(rhsElemType)) {
                resultElemType = lhsElemType;
            }
            else if (lhsElemType->IsConvertibleTo(rhsElemType)) {
                resultElemType = rhsElemType;
            }
            else if (rhsElemType->IsConvertibleTo(lhsElemType)) {
                resultElemType = lhsElemType;
            }
            else {
                // Two fundamental types are at least convertible in one direction
                GLSLD_UNREACHABLE();
            }

            ScalarKind resultScalarKind = resultElemType->GetScalarDesc()->type;
            if (lhsType->IsVector() && rhsType->IsMatrix()) {
                auto lhsVectorDesc = lhsType->GetVectorDesc();
                auto rhsMatrixDesc = rhsType->GetMatrixDesc();
                if (lhsVectorDesc->vectorSize == rhsMatrixDesc->dimCol) {
                    deducedType = Type::GetVectorType(resultScalarKind, rhsMatrixDesc->dimRow);
                    if (lhsElemType != resultElemType) {
                        lhsContextType = Type::GetVectorType(resultScalarKind, lhsVectorDesc->vectorSize);
                    }
                    if (rhsElemType != resultElemType) {
                        rhsContextType =
                            Type::GetMatrixType(resultScalarKind, rhsMatrixDesc->dimRow, rhsMatrixDesc->dimCol);
                    }
                }
            }
            else if (lhsType->IsMatrix() && rhsType->IsVector()) {
                auto lhsMatrixDesc = lhsType->GetMatrixDesc();
                auto rhsVectorDesc = rhsType->GetVectorDesc();
                if (lhsMatrixDesc->dimRow == rhsVectorDesc->vectorSize) {
                    deducedType = Type::GetVectorType(resultScalarKind, lhsMatrixDesc->dimCol);
                    if (lhsElemType != resultElemType) {
                        lhsContextType =
                            Type::GetMatrixType(resultScalarKind, lhsMatrixDesc->dimRow, lhsMatrixDesc->dimCol);
                    }
                    if (rhsElemType != resultElemType) {
                        rhsContextType = Type::GetVectorType(resultScalarKind, rhsVectorDesc->vectorSize);
                    }
                }
            }
            else if (lhsType->IsMatrix() && rhsType->IsMatrix()) {
                auto lhsMatrixDesc = lhsType->GetMatrixDesc();
                auto rhsMatrixDesc = rhsType->GetMatrixDesc();
                if (lhsMatrixDesc->dimRow == rhsMatrixDesc->dimCol) {
                    deducedType = Type::GetMatrixType(resultScalarKind, rhsMatrixDesc->dimRow, lhsMatrixDesc->dimCol);
                    if (lhsElemType != resultElemType) {
                        lhsContextType =
                            Type::GetMatrixType(resultScalarKind, lhsMatrixDesc->dimRow, lhsMatrixDesc->dimCol);
                    }
                    if (rhsElemType != resultElemType) {
                        rhsContextType =
                            Type::GetMatrixType(resultScalarKind, rhsMatrixDesc->dimRow, rhsMatrixDesc->dimCol);
                    }
                }
            }
            else {
                GLSLD_ASSERT(false);
            }
        }
        else if (op == BinaryOp::Plus || op == BinaryOp::Minus || op == BinaryOp::Div) {
            // This is component-wise operation
            if (lhsElemType->IsSameWith(rhsElemType)) {
                deducedType = lhsType;
            }
            else if (lhsElemType->IsConvertibleTo(rhsElemType)) {
                deducedType    = rhsType;
                lhsContextType = rhsType;
            }
            else if (rhsElemType->IsConvertibleTo(lhsElemType)) {
                deducedType    = lhsType;
                rhsContextType = lhsType;
            }
            else {
                // Two fundamental types are at least convertible in one direction
                GLSLD_UNREACHABLE();
            }
        }

        return BinaryExprTypeInfo{
            .deducedType    = deducedType,
            .lhsContextType = lhsContextType,
            .rhsContextType = rhsContextType,
        };
    }

    static auto DeduceAssignmentBinaryExprType(std::optional<BinaryOp> baseOp, const Type* lhsType, const Type* rhsType)
        -> BinaryExprTypeInfo
    {
        // FIXME: need to review the behavior of this function
        auto deducedType    = lhsType;
        auto rhsContextType = rhsType;

        if (baseOp) {
            auto baseOpTypeInfo = DeduceBinaryExprType(*baseOp, lhsType, rhsType);
            rhsContextType      = baseOpTypeInfo.rhsContextType;
        }
        else if (rhsType->IsConvertibleTo(lhsType)) {
            rhsContextType = lhsType;
        }

        return BinaryExprTypeInfo{
            .deducedType    = deducedType,
            .lhsContextType = lhsType,
            .rhsContextType = rhsContextType,
        };
    }

    static auto DeduceBinaryExprType(BinaryOp op, const Type* lhsType, const Type* rhsType) -> BinaryExprTypeInfo
    {
        const auto defaultResult = BinaryExprTypeInfo{
            .deducedType    = Type::GetErrorType(),
            .lhsContextType = lhsType,
            .rhsContextType = rhsType,
        };

        switch (op) {
        case BinaryOp::Comma:
            return BinaryExprTypeInfo{
                .deducedType    = rhsType,
                .lhsContextType = lhsType,
                .rhsContextType = rhsType,
            };
            break;
        case BinaryOp::Assign:
            return DeduceAssignmentBinaryExprType(std::nullopt, lhsType, rhsType);
        case BinaryOp::MulAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::Mul, lhsType, rhsType);
        case BinaryOp::DivAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::Div, lhsType, rhsType);
        case BinaryOp::ModAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::Modulo, lhsType, rhsType);
        case BinaryOp::AddAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::Plus, lhsType, rhsType);
        case BinaryOp::SubAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::Minus, lhsType, rhsType);
        case BinaryOp::LShiftAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::ShiftLeft, lhsType, rhsType);
        case BinaryOp::RShiftAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::ShiftRight, lhsType, rhsType);
        case BinaryOp::AndAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::BitwiseAnd, lhsType, rhsType);
        case BinaryOp::XorAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::BitwiseXor, lhsType, rhsType);
        case BinaryOp::OrAssign:
            return DeduceAssignmentBinaryExprType(BinaryOp::BitwiseOr, lhsType, rhsType);

        default:
            if (lhsType->IsError() || rhsType->IsError()) {
                return defaultResult;
            }

            if (lhsType->IsScalar()) {
                if (rhsType->IsScalar()) {
                    return DeduceComponentWiseBinaryExprType(op, lhsType, rhsType);
                }
                else if (rhsType->IsVector() || rhsType->IsMatrix()) {
                    return DeduceScalarBroadcastBinaryExprType(op, lhsType, rhsType);
                }
            }
            else if (rhsType->IsScalar()) {
                GLSLD_ASSERT(!lhsType->IsScalar());
                if (lhsType->IsVector() || lhsType->IsMatrix()) {
                    return DeduceScalarBroadcastBinaryExprType(op, lhsType, rhsType);
                }
            }
            else if (lhsType->IsVector()) {
                if (rhsType->IsVector()) {
                    return DeduceComponentWiseBinaryExprType(op, lhsType, rhsType);
                }
                else if (rhsType->IsMatrix()) {
                    return DeduceMatrixOpBinaryExprType(op, lhsType, rhsType);
                }
            }
            else if (rhsType->IsVector()) {
                GLSLD_ASSERT(!lhsType->IsVector());
                if (lhsType->IsMatrix()) {
                    return DeduceMatrixOpBinaryExprType(op, lhsType, rhsType);
                }
            }
            else if (lhsType->IsMatrix() && rhsType->IsMatrix()) {
                return DeduceMatrixOpBinaryExprType(op, lhsType, rhsType);
            }

            break;
        }

        return defaultResult;
    }

    auto AstBuilder::BuildBinaryExpr(AstSyntaxRange range, AstExpr* lhs, AstExpr* rhs, BinaryOp opcode)
        -> AstBinaryExpr*
    {
        bool isConst      = false;
        auto exprTypeInfo = DeduceBinaryExprType(opcode, lhs->GetDeducedType(), rhs->GetDeducedType());

        if (!exprTypeInfo.deducedType->IsError() && lhs->IsConst() && rhs->IsConst()) {
            // Comma and assignment operators are not const
            isConst = opcode != BinaryOp::Comma && !IsAssignmentOp(opcode);
        }

        lhs = TryMakeImplicitCast(lhs, exprTypeInfo.lhsContextType);
        rhs = TryMakeImplicitCast(rhs, exprTypeInfo.rhsContextType);

        auto result = CreateAstNode<AstBinaryExpr>(range, lhs, rhs, opcode);
        result->SetConst(isConst);
        result->SetDeducedType(exprTypeInfo.deducedType);
        return result;
    }

    auto AstBuilder::BuildSelectExpr(AstSyntaxRange range, AstExpr* condExpr, AstExpr* trueExpr, AstExpr* falseExpr)
        -> AstSelectExpr*
    {
        bool isConst            = false;
        const Type* deducedType = Type::GetErrorType();

        if (trueExpr->GetDeducedType()->IsError() || falseExpr->GetDeducedType()->IsError()) {
            deducedType = Type::GetErrorType();
        }
        else if (trueExpr->GetDeducedType()->IsSameWith(falseExpr->GetDeducedType())) {
            deducedType = trueExpr->GetDeducedType();
        }
        else if (trueExpr->GetDeducedType()->IsConvertibleTo(falseExpr->GetDeducedType())) {
            deducedType = falseExpr->GetDeducedType();
            trueExpr    = TryMakeImplicitCast(trueExpr, deducedType);
        }
        else if (falseExpr->GetDeducedType()->IsConvertibleTo(trueExpr->GetDeducedType())) {
            deducedType = trueExpr->GetDeducedType();
            falseExpr   = TryMakeImplicitCast(falseExpr, deducedType);
        }

        if (!deducedType->IsError() && condExpr->IsConst() && trueExpr->IsConst() && falseExpr->IsConst()) {
            isConst = true;
        }

        auto result = CreateAstNode<AstSelectExpr>(range, condExpr, trueExpr, falseExpr);
        result->SetConst(isConst);
        result->SetDeducedType(deducedType);
        return result;
    }

    auto AstBuilder::BuildImplicitCastExpr(AstSyntaxRange range, AstExpr* expr, const Type* castType)
        -> AstImplicitCastExpr*
    {
        auto result = CreateAstNode<AstImplicitCastExpr>(range, expr);
        result->SetConst(expr->IsConst());
        result->SetDeducedType(castType);
        return result;
    }

    auto AstBuilder::BuildFuntionCallExpr(AstSyntaxRange range, AstSyntaxToken functionName, std::vector<AstExpr*> args)
        -> AstFunctionCallExpr*
    {
        bool isConst                            = false;
        const Type* deducedType                 = Type::GetErrorType();
        const AstFunctionDecl* resolvedFunction = nullptr;

        if (functionName.IsIdentifier()) {
            std::vector<const Type*> argTypes;
            for (auto arg : args) {
                argTypes.push_back(arg->GetDeducedType());
            }

            auto functionNameText = functionName.text.StrView();
            if (auto function = symbolTable.FindFunction(functionNameText, argTypes, false)) {
                isConst = false;
                if (IsConstEvalFunction(functionNameText) &&
                    std::ranges::all_of(args, [](const AstExpr* arg) { return arg->IsConst(); })) {
                    // FIXME: this is a loose check. Is it good enough?
                    isConst = true;
                }
                deducedType      = function->GetReturnType()->GetResolvedType();
                resolvedFunction = function;

                for (size_t i = 0; i < args.size(); ++i) {
                    // Construct implicit cast if needed
                    args[i] = TryMakeImplicitCast(args[i], function->GetParams()[i]->GetResolvedType());
                }
            }
        }

        auto result = CreateAstNode<AstFunctionCallExpr>(range, functionName, CopyArray(args));
        result->SetConst(isConst);
        result->SetDeducedType(deducedType);
        result->SetResolvedFunction(resolvedFunction);
        return result;
    }

    auto AstBuilder::BuildConstructorCallExpr(AstSyntaxRange range, AstQualType* qualType, std::vector<AstExpr*> args)
        -> AstConstructorCallExpr*
    {
        // Infer implicit array size if needed
        // Notably, for implicitly sized array, only outermost dimension could be unsized.
        auto constructedType = qualType->GetResolvedType();
        if (auto arrayDesc = constructedType->GetArrayDesc(); arrayDesc && arrayDesc->dimSize == 0) {
            constructedType = astContext.GetArrayType(arrayDesc->elementType, args.size());
        }

        // Make implicit cast if needed
        // Note we don't add implicit cast for casting constructor
        if (constructedType->IsArithmetic()) {
            const bool isConversionCtor =
                args.size() == 1 && args.front()->GetDeducedType()->GetDimension() == constructedType->GetDimension();
            if (!isConversionCtor) {
                for (size_t i = 0; i < args.size(); ++i) {
                    if (args[i]->GetDeducedType()->IsArithmetic()) {
                        args[i] = TryMakeImplicitCast(
                            args[i], Type::GetArithmeticType(*constructedType->GetElementScalarKind(),
                                                             args[i]->GetDeducedType()->GetDimension()));
                    }
                }
            }
        }
        else if (constructedType->IsStruct() || constructedType->IsArray()) {
            for (size_t i = 0; i < args.size(); ++i) {
                args[i] = TryMakeImplicitCast(args[i], constructedType->GetComponentType(i));
            }
        }

        auto result = CreateAstNode<AstConstructorCallExpr>(range, qualType, CopyArray(args));
        result->SetConst(std::ranges::all_of(args, [](const AstExpr* arg) { return arg->IsConst(); }));
        result->SetDeducedType(constructedType);
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
        return CreateAstNode<AstCompoundStmt>(range, CopyArray(stmts));
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
        if (expr && returnType && expr->GetDeducedType()->IsConvertibleTo(returnType)) {
            expr = TryMakeImplicitCast(expr, returnType);
        }
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

    auto AstBuilder::BuildPrecisionDecl(AstSyntaxRange range, AstQualType* type) -> AstPrecisionDecl*
    {
        return CreateAstNode<AstPrecisionDecl>(range, type);
    }

    auto AstBuilder::BuildVariableDecl(AstSyntaxRange range, AstQualType* qualType, std::vector<Declarator> declarators)
        -> AstVariableDecl*
    {

        ArraySpan<AstVariableDeclaratorDecl*> declaratorNodes = {
            arena.Construct<AstVariableDeclaratorDecl*[]>(declarators.size()), declarators.size()};
        for (const auto& [i, declarator] : std::views::enumerate(declarators)) {
            auto resolvedType = astContext.GetArrayType(qualType->GetResolvedType(), declarator.arraySpec);

            // Infer implicit array size if needed
            // Notably, for implicitly sized array, only outermost dimension could be unsized.
            if (auto arrayDesc = resolvedType->GetArrayDesc();
                arrayDesc && arrayDesc->dimSize == 0 && declarator.initializer) {
                if (auto initArrayDesc = declarator.initializer->GetDeducedType()->GetArrayDesc();
                    initArrayDesc && arrayDesc->elementType == initArrayDesc->elementType) {
                    resolvedType = astContext.GetArrayType(arrayDesc->elementType, initArrayDesc->dimSize);
                }
            }

            // FIXME: avoids const_cast
            declaratorNodes[i] = CreateAstNode<AstVariableDeclaratorDecl>(
                range, declarator.nameToken, declarator.arraySpec,
                TryMakeImplicitCast(const_cast<AstInitializer*>(declarator.initializer), resolvedType), qualType);
            declaratorNodes[i]->SetScope(GetCurrentScope());
            declaratorNodes[i]->SetResolvedType(resolvedType);
        }

        auto result = CreateAstNode<AstVariableDecl>(range, qualType, declaratorNodes);
        result->SetScope(GetCurrentScope());

        symbolTable.GetCurrentLevel()->AddVariableDecl(*result);
        return result;
    }

    auto AstBuilder::BuildStructFieldDecl(AstSyntaxRange range, AstQualType* qualType,
                                          std::vector<Declarator> declarators) -> AstStructFieldDecl*
    {
        ArraySpan<AstStructFieldDeclaratorDecl*> declaratorNodes = {
            arena.Construct<AstStructFieldDeclaratorDecl*[]>(declarators.size()), declarators.size()};
        for (const auto& [i, declarator] : std::views::enumerate(declarators)) {
            auto resolvedType = astContext.GetArrayType(qualType->GetResolvedType(), declarator.arraySpec);

            declaratorNodes[i] = CreateAstNode<AstStructFieldDeclaratorDecl>(
                range, declarator.nameToken, declarator.arraySpec, declarator.initializer, qualType);
            declaratorNodes[i]->SetScope(DeclScope::Struct);
            declaratorNodes[i]->SetResolvedType(resolvedType);
        }

        auto result = CreateAstNode<AstStructFieldDecl>(range, qualType, declaratorNodes);
        result->SetScope(DeclScope::Struct);
        return result;
    }

    auto AstBuilder::BuildStructDecl(AstSyntaxRange range, std::optional<AstSyntaxToken> declTok,
                                     std::vector<AstStructFieldDecl*> members) -> AstStructDecl*
    {
        auto result = CreateAstNode<AstStructDecl>(range, declTok, CopyArray(members));
        result->SetScope(GetCurrentScope());
        result->SetDeclaredType(astContext.CreateStructType(*result));

        return result;
    }

    auto AstBuilder::BuildBlockFieldDecl(AstSyntaxRange range, AstQualType* qualType,
                                         std::vector<Declarator> declarators) -> AstBlockFieldDecl*
    {
        ArraySpan<AstBlockFieldDeclaratorDecl*> declaratorNodes = {
            arena.Construct<AstBlockFieldDeclaratorDecl*[]>(declarators.size()), declarators.size()};
        for (const auto& [i, declarator] : std::views::enumerate(declarators)) {
            auto resolvedType = astContext.GetArrayType(qualType->GetResolvedType(), declarator.arraySpec);

            declaratorNodes[i] = CreateAstNode<AstBlockFieldDeclaratorDecl>(
                range, declarator.nameToken, declarator.arraySpec, declarator.initializer, qualType);
            // TODO: set global scope for unnamed block field decl
            declaratorNodes[i]->SetScope(DeclScope::Block);
            declaratorNodes[i]->SetResolvedType(resolvedType);
        }

        auto result = CreateAstNode<AstBlockFieldDecl>(range, qualType, declaratorNodes);
        result->SetScope(DeclScope::Block);
        return result;
    }

    auto AstBuilder::BuildInterfaceBlockDecl(AstSyntaxRange range, AstTypeQualifierSeq* quals, AstSyntaxToken declTok,
                                             std::vector<AstBlockFieldDecl*> members,
                                             std::optional<Declarator> declarator) -> AstInterfaceBlockDecl*
    {
        GLSLD_ASSERT(GetCurrentScope() == DeclScope::Global);
        auto result = CreateAstNode<AstInterfaceBlockDecl>(range, quals, declTok, CopyArray(members), declarator);

        auto blockType = astContext.CreateInterfaceBlockType(*result);
        result->SetScope(GetCurrentScope());
        result->SetResolvedBlockType(blockType);
        if (declarator) {
            result->SetResolvedInstanceType(astContext.GetArrayType(blockType, declarator->arraySpec));
        }
        else {
            result->SetResolvedInstanceType(Type::GetErrorType());
        }

        symbolTable.GetCurrentLevel()->AddInterfaceBlockDecl(*result);
        return result;
    }

    auto AstBuilder::BuildParamDecl(AstSyntaxRange range, AstQualType* qualType, std::optional<Declarator> declarator)
        -> AstParamDecl*
    {
        GLSLD_ASSERT(GetCurrentScope() == DeclScope::Function);
        auto result = CreateAstNode<AstParamDecl>(range, qualType, declarator);

        result->SetScope(GetCurrentScope());
        result->SetResolvedType(
            astContext.GetArrayType(qualType->GetResolvedType(), declarator ? declarator->arraySpec : nullptr));

        symbolTable.GetCurrentLevel()->AddParamDecl(*result);
        return result;
    }

    auto AstBuilder::BuildFunctionDecl(AstSyntaxRange range, AstQualType* returnType, AstSyntaxToken declTok,
                                       std::vector<AstParamDecl*> params, AstStmt* body) -> AstFunctionDecl*
    {
        // FIXME: don't build AST for local function decl?
        // GLSLD_ASSERT(GetCurrentScope() == DeclScope::Global);
        auto result = CreateAstNode<AstFunctionDecl>(range, returnType, declTok, CopyArray(params), body);

        // FIXME: set the correct first declaration
        result->SetScope(GetCurrentScope());
        result->SetFirstDeclaration(result);

        symbolTable.GetCurrentLevel()->AddFunctionDecl(*result);
        return result;
    }

#pragma endregion

    auto AstBuilder::TryMakeImplicitCast(AstExpr* expr, const Type* contextType) -> AstExpr*
    {
        GLSLD_ASSERT(expr);
        if (expr->GetDeducedType()->IsSameWith(contextType) || contextType->IsError()) {
            return expr;
        }

        return BuildImplicitCastExpr(expr->GetSyntaxRange(), expr, contextType);
    }

    auto AstBuilder::TryMakeImplicitCast(AstInitializer* initializer, const Type* contextType) -> AstInitializer*
    {
        if (initializer && initializer->Is<AstExpr>()) {
            return TryMakeImplicitCast(initializer->As<AstExpr>(), contextType);
        }
        else {
            return initializer;
        }
    }
} // namespace glsld
#include "TypeChecker.h"

namespace glsld
{
    namespace
    {
        // FIXME: avoid doing another traversal. We could do this during parsing
        auto ParseIntegerLiteral(StringView literalText) -> std::pair<const Type*, ConstValue>
        {
            GLSLD_ASSERT(!literalText.Empty());
            if (literalText.EndWith("u") || literalText.EndWith("U")) {
                auto literalTextNoSuffix = literalText.DropBack(1);

                uint32_t value;
                auto parseResult = std::from_chars(literalTextNoSuffix.Data(),
                                                   literalTextNoSuffix.Data() + literalTextNoSuffix.Size(), value);
                if (parseResult.ptr == literalTextNoSuffix.Data() + literalTextNoSuffix.Size()) {
                    return {GetBuiltinTypeDesc(GlslBuiltinType::Ty_uint), ConstValue::FromValue<uint32_t>(value)};
                }
            }
            else {
                auto literalTextNoSuffix = literalText;

                int32_t value;
                auto parseResult = std::from_chars(literalTextNoSuffix.Data(),
                                                   literalTextNoSuffix.Data() + literalTextNoSuffix.Size(), value);
                if (parseResult.ptr == literalTextNoSuffix.Data() + literalTextNoSuffix.Size()) {
                    return {GetBuiltinTypeDesc(GlslBuiltinType::Ty_int), ConstValue::FromValue<int32_t>(value)};
                }
            }

            return {GetErrorTypeDesc(), ConstValue{}};
        }

        auto ParseFloatLiteral(StringView literalText) -> std::pair<const Type*, ConstValue>
        {
            GLSLD_ASSERT(!literalText.Empty());
            if (literalText.EndWith("lf") || literalText.EndWith("LF")) {
                auto literalTextNoSuffix = literalText.DropBack(2);

                double value;
                auto parseResult = std::from_chars(literalTextNoSuffix.Data(),
                                                   literalTextNoSuffix.Data() + literalTextNoSuffix.Size(), value);
                if (parseResult.ptr == literalTextNoSuffix.Data() + literalTextNoSuffix.Size()) {
                    return {GetBuiltinTypeDesc(GlslBuiltinType::Ty_double), ConstValue::FromValue<double>(value)};
                }
            }
            else {
                auto literalTextNoSuffix = literalText;
                if (literalText.EndWith("f") || literalText.EndWith("F")) {
                    literalTextNoSuffix = literalText.DropBack(1);
                }

                float value;
                auto parseResult = std::from_chars(literalTextNoSuffix.Data(),
                                                   literalTextNoSuffix.Data() + literalTextNoSuffix.Size(), value);
                if (parseResult.ptr == literalTextNoSuffix.Data() + literalTextNoSuffix.Size()) {
                    return {GetBuiltinTypeDesc(GlslBuiltinType::Ty_float), ConstValue::FromValue<float>(value)};
                }
            }

            return {GetErrorTypeDesc(), ConstValue{}};
        }

        auto DeduceUnaryExprType(UnaryOp op, const Type* operand) -> const Type*
        {
            switch (op) {
            case UnaryOp::Identity:
                return operand;
            case UnaryOp::Negate:
                if (operand->IsSameWith(GlslBuiltinType::Ty_int) || operand->IsSameWith(GlslBuiltinType::Ty_uint) ||
                    operand->IsSameWith(GlslBuiltinType::Ty_float) || operand->IsSameWith(GlslBuiltinType::Ty_double)) {
                    return operand;
                }
                else {
                    return GetErrorTypeDesc();
                }
            case UnaryOp::BitwiseNot:
                if (operand->IsSameWith(GlslBuiltinType::Ty_int) || operand->IsSameWith(GlslBuiltinType::Ty_uint)) {
                    return operand;
                }
                else {
                    return GetErrorTypeDesc();
                }
            case UnaryOp::LogicalNot:
                if (operand->IsSameWith(GlslBuiltinType::Ty_bool)) {
                    return operand;
                }
                else {
                    return GetErrorTypeDesc();
                }
            case UnaryOp::PrefixInc:
            case UnaryOp::PrefixDec:
            case UnaryOp::PostfixInc:
            case UnaryOp::PostfixDec:
                if (operand->IsSameWith(GlslBuiltinType::Ty_int) || operand->IsSameWith(GlslBuiltinType::Ty_uint)) {
                    return operand;
                }
                else {
                    return GetErrorTypeDesc();
                }
            case UnaryOp::Length:
                // FIXME: test if the type has length operation
                return GetBuiltinTypeDesc(GlslBuiltinType::Ty_int);
            }

            GLSLD_UNREACHABLE();
        }

        auto DeduceBinaryExprType(BinaryOp op, const Type* lhs, const Type* rhs) -> const Type*
        {
            // FIXME: implement this
            if (lhs->IsSameWith(rhs)) {
                return lhs;
            }
            else {
                return GetErrorTypeDesc();
            }
        }

    } // namespace

    class TypeCheckVisitor : public AstVisitor<TypeCheckVisitor>
    {
    public:
        TypeCheckVisitor(TypeChecker& typeChecker) : typeChecker(typeChecker)
        {
        }

        auto ExitAstQualType(AstQualType& type) -> void
        {
            typeChecker.ResolveType(type);
        }

        auto EnterAstFunctionDecl(AstFunctionDecl& decl) -> AstVisitPolicy
        {
            if (typeChecker.InFunctionScope()) {
                // nested function isn't allowed
                return AstVisitPolicy::Leave;
            }

            return AstVisitPolicy::Traverse;
        }

        auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            typeChecker.EnterFunctionScope(decl);
        }
        auto ExitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            typeChecker.ExitFunctionScope(decl);
        }

        auto ExitAstStructDecl(AstStructDecl& decl) -> void
        {
            typeChecker.DeclareStruct(decl);
        }
        auto ExitAstVariableDecl(AstVariableDecl& decl) -> void
        {
            typeChecker.DeclareVariable(decl);
        }
        auto ExitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
        {
            typeChecker.DeclareInterfaceBlock(decl);
        }
        auto ExitAstParamDecl(AstParamDecl& decl) -> void
        {
            typeChecker.DeclareParameter(decl);
        }

        auto VisitAstCompoundStmt(AstCompoundStmt& stmt) -> void
        {
            // ignore the outermost compound stmt because we share scope with the function decl
            if (compoundStmtDepth > 0) {
                typeChecker.EnterBlockScope();
            }
            compoundStmtDepth += 1;
        }
        auto ExitAstCompoundStmt(AstCompoundStmt& stmt) -> void
        {
            // ignore the outermost compound stmt because we share scope with the function decl
            compoundStmtDepth -= 1;
            if (compoundStmtDepth > 0) {
                typeChecker.ExitBlockScope();
            }
        }
        auto ExitAstStmt(AstStmt& stmt) -> void
        {
            switch (stmt.GetTag()) {
            case AstNodeTag::AstErrorStmt:
            // case AstNodeTag::AstCompoundStmt:
            case AstNodeTag::AstExprStmt:
            case AstNodeTag::AstDeclStmt:
            case AstNodeTag::AstForStmt:
            case AstNodeTag::AstDoWhileStmt:
            case AstNodeTag::AstWhileStmt:
            case AstNodeTag::AstIfStmt:
                // FIXME: check if the condition is bool
            case AstNodeTag::AstLabelStmt:
                // FIXME: only allowed in switch stmt?
            case AstNodeTag::AstSwitchStmt:
            case AstNodeTag::AstJumpStmt:
                // FIXME: check if is a valid jump stmt
                typeChecker.CheckJumpStmt(*stmt.As<AstJumpStmt>());
            case AstNodeTag::AstReturnStmt:
                // FIXME: check if the return type is correct
                typeChecker.CheckReturnStmt(*stmt.As<AstReturnStmt>());
                break;
            default:
                GLSLD_UNREACHABLE();
            }
        };

        auto VisitAstInvokeExpr(AstInvokeExpr& expr) -> void
        {
            typeChecker.PreprocessInvokeExpr(expr);
        }

        auto ExitAstExpr(AstExpr& expr) -> void
        {
            switch (expr.GetTag()) {
            case AstNodeTag::AstErrorExpr:
                // Nothing to do for error expr.
                break;
            case AstNodeTag::AstConstantExpr:
                typeChecker.CheckConstantExpr(*expr.As<AstConstantExpr>());
                break;
            case AstNodeTag::AstNameAccessExpr:
                typeChecker.CheckNameAccessExpr(*expr.As<AstNameAccessExpr>());
                break;
            case AstNodeTag::AstUnaryExpr:
                typeChecker.CheckUnaryExpr(*expr.As<AstUnaryExpr>());
                break;
            case AstNodeTag::AstBinaryExpr:
                typeChecker.CheckBinaryExpr(*expr.As<AstBinaryExpr>());
                break;
            case AstNodeTag::AstSelectExpr:
                typeChecker.CheckSelectExpr(*expr.As<AstSelectExpr>());
                break;
            case AstNodeTag::AstInvokeExpr:
                typeChecker.CheckInvokeExpr(*expr.As<AstInvokeExpr>());
                break;
            case AstNodeTag::AstIndexAccessExpr:
                typeChecker.CheckIndexAccessExpr(*expr.As<AstIndexAccessExpr>());
                break;
            default:
                GLSLD_UNREACHABLE();
            }
        }

    private:
        TypeChecker& typeChecker;

        int compoundStmtDepth = 0;
    };

    auto TypeChecker::DoTypeCheck(const SymbolTable* externalSymbolTable) -> std::unique_ptr<SymbolTable>
    {
        this->externalSymbolTable = externalSymbolTable;
        symbolTableStack.push_back(std::make_unique<SymbolTable>());
        TypeCheckVisitor{*this}.TraverseAst(compilerObject.GetAstContext());

        this->currentFunction     = nullptr;
        this->externalSymbolTable = nullptr;

        GLSLD_ASSERT(symbolTableStack.size() == 1);
        auto result = std::move(symbolTableStack.back());
        symbolTableStack.clear();
        return result;
    }

    auto TypeChecker::ResolveType(AstQualType& type) -> void
    {
        const Type* resolvedType = GetErrorTypeDesc();

        // Resolve element type
        if (auto builtinType = GetGlslBuiltinType(type.GetTypeNameTok())) {
            resolvedType = GetBuiltinTypeDesc(*builtinType);
        }
        else if (type.GetStructDecl()) {
            // A struct type
            // NOTE it is already resolved
            resolvedType = type.GetStructDecl()->GetTypeDesc();
        }
        else {
            auto symbol = FindSymbol(type.GetTypeNameTok().text.Str());
            if (symbol && symbol.GetDecl()->Is<AstStructDecl>()) {
                resolvedType = symbol.GetDecl()->As<AstStructDecl>()->GetTypeDesc();
                type.SetResolvedStructDecl(symbol.GetDecl()->As<AstStructDecl>());
            }
        }

        // Resolve array type if any
        resolvedType = compilerObject.GetTypeContext().GetArrayType(resolvedType, type.GetArraySpec());

        type.SetResolvedType(resolvedType);
    }

    auto TypeChecker::DeclareStruct(AstStructDecl& decl) -> void
    {
        decl.SetTypeDesc(compilerObject.GetTypeContext().CreateStructType(decl));
        for (auto memberDecl : decl.GetMembers()) {
            for (size_t i = 0; i < memberDecl->GetDeclarators().size(); ++i) {
                const auto& declarator = memberDecl->GetDeclarators()[i];
                decl.AddMemberDecl(declarator.declTok.text.Str(), DeclView{memberDecl, i});
            }
        }

        // FIXME: Test redefinition
        GetSymbolTable().AddStructDecl(decl);
    }

    auto TypeChecker::DeclareInterfaceBlock(AstInterfaceBlockDecl& decl) -> void
    {
        // FIXME: Resolve a type, but the type shouldn't be registered into the symbol table
        auto blockType    = compilerObject.GetTypeContext().CreateStructType(decl);
        auto instanceType = blockType;
        if (decl.GetDeclarator() && decl.GetDeclarator()->arraySize) {
            instanceType = compilerObject.GetTypeContext().GetArrayType(blockType, decl.GetDeclarator()->arraySize);
        }

        decl.SetBlockType(blockType);
        decl.SetInstanceType(instanceType);

        for (auto memberDecl : decl.GetMembers()) {
            for (size_t i = 0; i < memberDecl->GetDeclarators().size(); ++i) {
                const auto& declarator = memberDecl->GetDeclarators()[i];
                decl.AddMemberDecl(declarator.declTok.text.Str(), DeclView{memberDecl, i});
            }
        }

        // FIXME: Test redefinition
        GetSymbolTable().AddInterfaceBlockDecl(decl);
    }

    auto TypeChecker::DeclareVariable(AstVariableDecl& decl) -> void
    {
        // NOTE this could have 0 declarator. Then nothing is added.
        GetSymbolTable().AddVariableDecl(decl);

        // FIXME: check for initializer
        // FIXME: Test redefinition
    }

    auto TypeChecker::DeclareParameter(AstParamDecl& decl) -> void
    {
        GetSymbolTable().AddParamDecl(decl);

        // FIXME: Test redefinition
    }

    auto TypeChecker::EnterFunctionScope(AstFunctionDecl& decl) -> void
    {
        GLSLD_ASSERT(!InFunctionScope());

        currentFunction = &decl;
        PushScope();
    }

    auto TypeChecker::ExitFunctionScope(AstFunctionDecl& decl) -> void
    {
        GLSLD_ASSERT(InFunctionScope());

        currentFunction = nullptr;
        PopScope();

        std::vector<const Type*> paramTypes;
        for (auto param : decl.GetParams()) {
            auto type = param->GetType()->GetResolvedType();
            if (param->GetType()->GetArraySpec()) {
                type = compilerObject.GetTypeContext().GetArrayType(type, param->GetType()->GetArraySpec());
            }
            paramTypes.push_back(type);
        }
        decl.SetResolvedParamTypes(std::move(paramTypes));
        decl.SetResolvedReturnType(decl.GetReturnType()->GetResolvedType());

        // Since recursion is not allowed, we can add the function to the symbol table here
        GetSymbolTable().AddFunction(decl);
    }

    auto TypeChecker::EnterBlockScope() -> void
    {
        GLSLD_ASSERT(InFunctionScope());
        PushScope();
    }

    auto TypeChecker::ExitBlockScope() -> void
    {
        GLSLD_ASSERT(InFunctionScope());
        PopScope();
    }

    auto TypeChecker::PreprocessInvokeExpr(AstInvokeExpr& expr) -> void
    {
        if (auto func = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
            // Case 1: `IDENTIFIER(...)` or `EXPR.IDENTIFIER(...)`

            if (func->GetAccessName().klass != TokenKlass::Identifier) {
                // This is a constructor
                // FIXME: Is this always a constructor? Should we check the type?
                func->SetAccessType(NameAccessType::Constructor);
            }
            else {
                auto symbol = FindSymbol(func->GetAccessName().text.Str());
                if (symbol &&
                    (symbol.GetDecl()->Is<AstStructDecl>() || symbol.GetDecl()->Is<AstInterfaceBlockDecl>())) {
                    // FIXME: could interface block being constructor?
                    func->SetAccessType(NameAccessType::Constructor);
                }
                else {
                    // We assume unknown identifier being called is function
                    func->SetAccessType(NameAccessType::Function);
                }
            }
        }
        else if (auto invokedExpr = expr.GetInvokedExpr()->As<AstIndexAccessExpr>();
                 invokedExpr && invokedExpr->GetInvokedExpr()->Is<AstNameAccessExpr>()) {
            // Case 2: `TYPE[...]()`
            invokedExpr->GetInvokedExpr()->As<AstNameAccessExpr>()->SetAccessType(NameAccessType::Constructor);
        }
        else {
            // FIXME: Are there other cases? Perhaps need to report error for invalid invocation.
        }
    }

    auto TypeChecker::CheckConstantExpr(AstConstantExpr& expr) -> void
    {
        // FIXME: handle extension suffix like S and LF
        switch (expr.GetToken().klass) {
        case TokenKlass::K_true:
            expr.SetConstValue(ConstValue::FromValue<bool>(true));
            expr.SetDeducedType(GetBuiltinTypeDesc(GlslBuiltinType::Ty_bool));
            break;
        case TokenKlass::K_false:
            expr.SetConstValue(ConstValue::FromValue<bool>(false));
            expr.SetDeducedType(GetBuiltinTypeDesc(GlslBuiltinType::Ty_bool));
            break;
        case TokenKlass::IntegerConstant:
        {
            auto [type, value] = ParseIntegerLiteral(expr.GetToken().text.StrView());
            expr.SetDeducedType(type);
            expr.SetConstValue(std::move(value));
            break;
        }
        case TokenKlass::FloatConstant:
        {
            auto [type, value] = ParseFloatLiteral(expr.GetToken().text.StrView());
            expr.SetDeducedType(type);
            expr.SetConstValue(std::move(value));
            break;
        }
        default:
            GLSLD_UNREACHABLE();
        }
    }

    auto TypeChecker::CheckNameAccessExpr(AstNameAccessExpr& expr) -> void
    {
        if (expr.GetAccessType() == NameAccessType::Unknown) {
            expr.SetAccessType(NameAccessType::Variable);
        }
        else {
            // NOTE we might have set the access type in `PreprocessInvokeExpr`.
            // Since we resolve function call in `CheckInvokeExpr`, type checking for invoked
            // expression is there as well. Return early here.
            return;
        }

        auto accessName = expr.GetAccessName().text.StrView();
        if (expr.GetAccessChain()) {
            // Accessing with the syntax `expr.xxx`

            auto accessChainType = expr.GetAccessChain()->GetDeducedType();
            if (accessChainType->IsScalar() || accessChainType->IsVector()) {
                // This could be a swizzle access
                ResolveSwizzleAccess(expr);
            }
            else if (auto structDesc = accessChainType->GetStructDesc()) {
                // This is a field access
                if (auto structDecl = structDesc->decl->As<AstStructDecl>()) {
                    auto accessedDecl = structDecl->FindMemberDecl(std::string{accessName});
                    if (accessedDecl.IsValid()) {
                        GLSLD_ASSERT(accessedDecl.GetDecl()->Is<AstStructMemberDecl>());
                        expr.SetDeducedType(GetDeclType(accessedDecl));
                        expr.SetAccessedDecl(accessedDecl);
                    }
                }
                else if (auto blockDecl = structDesc->decl->As<AstInterfaceBlockDecl>()) {
                    auto accessedDecl = blockDecl->FindMemberDecl(std::string{accessName});
                    if (accessedDecl.IsValid()) {
                        GLSLD_ASSERT(accessedDecl.GetDecl()->Is<AstStructMemberDecl>());
                        expr.SetDeducedType(GetDeclType(accessedDecl));
                        expr.SetAccessedDecl(accessedDecl);
                    }
                }
            }
            else {
                // Bad access chain
                // FIXME: report error
            }
        }
        else {
            // Accessing with the syntax `xxx`

            GLSLD_ASSERT(expr.GetAccessType() == NameAccessType::Variable);
            if (auto symbol = FindSymbol(std::string{accessName})) {
                if (auto varDecl = symbol.GetDecl()->As<AstVariableDecl>()) {
                    // Variable

                    expr.SetAccessedDecl(symbol);

                    const auto& declarator = varDecl->GetDeclarators()[symbol.GetIndex()];
                    expr.SetDeducedType(compilerObject.GetTypeContext().GetArrayType(
                        varDecl->GetType()->GetResolvedType(), declarator.arraySize));
                }
                else if (auto paramDecl = symbol.GetDecl()->As<AstParamDecl>()) {
                    // Parameter

                    expr.SetAccessedDecl(symbol);

                    GLSLD_ASSERT(symbol.GetIndex() == 0);
                    if (paramDecl->GetDeclarator() && paramDecl->GetDeclarator()->arraySize) {
                        expr.SetDeducedType(compilerObject.GetTypeContext().GetArrayType(
                            paramDecl->GetType()->GetResolvedType(), paramDecl->GetDeclarator()->arraySize));
                    }
                    else {
                        expr.SetDeducedType(paramDecl->GetType()->GetResolvedType());
                    }
                }
                else if (auto blockDecl = symbol.GetDecl()->As<AstInterfaceBlockDecl>()) {
                    // Interface block

                    expr.SetAccessedDecl(symbol);
                    expr.SetDeducedType(blockDecl->GetInstanceType());
                }
                else if (auto memberDecl = symbol.GetDecl()->As<AstStructMemberDecl>()) {
                    // Unnamed interface block member

                    expr.SetAccessedDecl(symbol);
                    // FIXME: should we really use AstStructMemberDecl? Maybe AstInterfaceBlockMemberDecl?
                    const auto& declarator = memberDecl->GetDeclarators()[symbol.GetIndex()];
                    expr.SetDeducedType(compilerObject.GetTypeContext().GetArrayType(
                        memberDecl->GetType()->GetResolvedType(), declarator.arraySize));
                }
                else {
                    // FIXME: Anything else? If not, report error.
                }
            }
            else {
                // FIXME: report error, unknown symbol accessed
            }
        }
    }

    auto TypeChecker::CheckIndexAccessExpr(AstIndexAccessExpr& expr) -> void
    {
        for (auto dimExpr : expr.GetArraySpec()->GetSizeList()) {
            if (!dimExpr || !dimExpr->GetDeducedType()->IsIntegralScalarType()) {
                // FIXME: report error, bad dim index
                // FIXME: Why do we early return here?
                return;
            }
        }

        auto invokedExprType = expr.GetInvokedExpr()->GetDeducedType();
        if (auto arrayTypeDesc = invokedExprType->GetArrayDesc()) {
            size_t numIndexedDims = expr.GetArraySpec()->GetSizeList().size();
            if (numIndexedDims <= arrayTypeDesc->dimSizes.size()) {
                // FIXME: deduce array type properly
                auto dim2 = arrayTypeDesc->dimSizes;
                for (size_t i = 0; i < numIndexedDims; ++i) {
                    dim2.pop_back();
                }

                expr.SetDeducedType(compilerObject.GetTypeContext().GetArrayType(arrayTypeDesc->elementType, dim2));
            }
        }
        else {
            // FIXME: support other types like vector and matrix
            // FIXME: report error, not an array type
        }
    }

    auto TypeChecker::CheckUnaryExpr(AstUnaryExpr& expr) -> void
    {
        expr.SetConstValue(EvalUnaryConstExpr(expr.GetOperator(), expr.GetOperandExpr()->GetConstValue()));
        expr.SetDeducedType(DeduceUnaryExprType(expr.GetOperator(), expr.GetOperandExpr()->GetDeducedType()));
    }

    auto TypeChecker::CheckBinaryExpr(AstBinaryExpr& expr) -> void
    {
        expr.SetConstValue(EvalBinaryConstExpr(expr.GetOperator(), expr.GetLeftOperandExpr()->GetConstValue(),
                                               expr.GetRightOperandExpr()->GetConstValue()));
        expr.SetDeducedType(DeduceBinaryExprType(expr.GetOperator(), expr.GetLeftOperandExpr()->GetDeducedType(),
                                                 expr.GetRightOperandExpr()->GetDeducedType()));
    }

    auto TypeChecker::CheckSelectExpr(AstSelectExpr& expr) -> void
    {
        expr.SetConstValue(EvalSelectConstExpr(expr.GetPredicateExpr()->GetConstValue(),
                                               expr.GetIfBranchExpr()->GetConstValue(),
                                               expr.GetElseBranchExpr()->GetConstValue()));
        // FIXME: deduce type properly
        expr.SetDeducedType(GetErrorTypeDesc());
    }

    auto TypeChecker::CheckInvokeExpr(AstInvokeExpr& expr) -> void
    {
        // Resolve the called entity first
        ResolveInvokeExpr(expr);

        // FIXME: handle function call
        // FIXME: handle things like `S[2](...)`
        if (auto invokedExpr = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
            if (invokedExpr->GetAccessType() == NameAccessType::Function) {
                auto funcDecl = invokedExpr->GetAccessedDecl().GetDecl()->As<AstFunctionDecl>();
            }
            else if (invokedExpr->GetAccessType() == NameAccessType::Constructor) {
                if (invokedExpr->GetAccessName().klass != TokenKlass::Identifier) {
                    // This is a constructor
                    auto builtinType = GetGlslBuiltinType(invokedExpr->GetAccessName());
                    GLSLD_ASSERT(builtinType.has_value());
                    expr.SetDeducedType(GetBuiltinTypeDesc(*builtinType));
                    return;
                }
            }
            else {
                GLSLD_UNREACHABLE();
            }
        }
    }

    auto TypeChecker::GetDeclType(DeclView declView) -> const Type*
    {
        if (!declView.IsValid()) {
            return GetErrorTypeDesc();
        }

        auto decl = declView.GetDecl();
        if (decl->Is<AstEmptyDecl>()) {
            return GetErrorTypeDesc();
        }
        else if (auto structDecl = decl->As<AstStructDecl>()) {
            return structDecl->GetTypeDesc();
        }
        else if (auto blockDecl = decl->As<AstInterfaceBlockDecl>()) {
            return blockDecl->GetBlockType();
        }
        else if (auto memberDecl = decl->As<AstStructMemberDecl>()) {
            if (declView.GetIndex() < memberDecl->GetDeclarators().size()) {
                return compilerObject.GetTypeContext().GetArrayType(
                    memberDecl->GetType()->GetResolvedType(),
                    memberDecl->GetDeclarators()[declView.GetIndex()].arraySize);
            }
        }
        else if (auto varDecl = decl->As<AstVariableDecl>()) {
            if (declView.GetIndex() < varDecl->GetDeclarators().size()) {
                return compilerObject.GetTypeContext().GetArrayType(
                    varDecl->GetType()->GetResolvedType(), varDecl->GetDeclarators()[declView.GetIndex()].arraySize);
            }
        }
        else if (auto paramDecl = decl->As<AstParamDecl>()) {
            // FIXME: arrayness
            return paramDecl->GetType()->GetResolvedType();
        }
        else if (auto funcDecl = decl->As<AstFunctionDecl>()) {
            // FIXME: function type?
            return GetErrorTypeDesc();
        }

        GLSLD_UNREACHABLE();
    }
} // namespace glsld
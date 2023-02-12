#include "TypeChecker.h"

namespace glsld
{
    namespace
    {
        // FIXME: avoid doing another traversal. We could do this during parsing
        auto ParseIntegerLiteral(StringView literalText) -> ConstValue
        {
            GLSLD_ASSERT(!literalText.Empty());
            if (literalText.EndWith("u") || literalText.EndWith("U")) {
                auto literalTextNoSuffix = literalText.DropBack(1);

                uint32_t value;
                auto parseResult = std::from_chars(literalTextNoSuffix.Data(),
                                                   literalTextNoSuffix.Data() + literalTextNoSuffix.Size(), value);
                if (parseResult.ptr == literalTextNoSuffix.Data() + literalTextNoSuffix.Size()) {
                    return ConstValue::FromValue<BuiltinType::Ty_uint>(value);
                }
                else {
                    return ConstValue{};
                }
            }
            else {
                auto literalTextNoSuffix = literalText;

                int32_t value;
                auto parseResult = std::from_chars(literalTextNoSuffix.Data(),
                                                   literalTextNoSuffix.Data() + literalTextNoSuffix.Size(), value);
                if (parseResult.ptr == literalTextNoSuffix.Data() + literalTextNoSuffix.Size()) {
                    return ConstValue::FromValue<BuiltinType::Ty_int>(value);
                }
                else {
                    return ConstValue{};
                }
            }
        }

        auto ParseFloatLiteral(StringView literalText) -> ConstValue
        {
            GLSLD_ASSERT(!literalText.Empty());
            if (literalText.EndWith("lf") || literalText.EndWith("LF")) {
                auto literalTextNoSuffix = literalText.DropBack(2);

                double value;
                auto parseResult = std::from_chars(literalTextNoSuffix.Data(),
                                                   literalTextNoSuffix.Data() + literalTextNoSuffix.Size(), value);
                if (parseResult.ptr == literalTextNoSuffix.Data() + literalTextNoSuffix.Size()) {
                    return ConstValue::FromValue<BuiltinType::Ty_double>(value);
                }
                else {
                    return ConstValue{};
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
                    return ConstValue::FromValue<BuiltinType::Ty_float>(value);
                }
                else {
                    return ConstValue{};
                }
            }
        }

        auto EvalUnaryType(UnaryOp op, const TypeDesc* operand) -> const TypeDesc*
        {
            switch (op) {
            case UnaryOp::Identity:
                return operand;
            case UnaryOp::Nagate:
                if (operand->IsSameWith(BuiltinType::Ty_int) || operand->IsSameWith(BuiltinType::Ty_uint) ||
                    operand->IsSameWith(BuiltinType::Ty_float) || operand->IsSameWith(BuiltinType::Ty_double)) {
                    return operand;
                }
                else {
                    return GetErrorTypeDesc();
                }
            case UnaryOp::BitwiseNot:
                if (operand->IsSameWith(BuiltinType::Ty_int) || operand->IsSameWith(BuiltinType::Ty_uint)) {
                    return operand;
                }
                else {
                    return GetErrorTypeDesc();
                }
            case UnaryOp::LogicalNot:
                if (operand->IsSameWith(BuiltinType::Ty_bool)) {
                    return operand;
                }
                else {
                    return GetErrorTypeDesc();
                }
            case UnaryOp::PrefixInc:
            case UnaryOp::PrefixDec:
            case UnaryOp::PostfixInc:
            case UnaryOp::PostfixDec:
                if (operand->IsSameWith(BuiltinType::Ty_int) || operand->IsSameWith(BuiltinType::Ty_uint)) {
                    return operand;
                }
                else {
                    return GetErrorTypeDesc();
                }
            case UnaryOp::Length:
                // FIXME: test if the type has length operation
                return GetBuiltinTypeDesc(BuiltinType::Ty_int);
            }

            GLSLD_UNREACHABLE();
        }

        auto EvalBinaryType(BinaryOp op, const TypeDesc* lhs, const TypeDesc* rhs) -> const TypeDesc*
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

        auto VisitAstInvokeExpr(AstInvokeExpr& expr) -> void
        {
            typeChecker.PreprocessInvokeExpr(expr);
        }

        auto ExitAstExpr(AstExpr& expr) -> void
        {
            switch (expr.GetTag()) {
            case AstNodeTag::AstErrorExpr:
                typeChecker.CheckErrorExpr(*expr.As<AstErrorExpr>());
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

    auto TypeChecker::TypeCheck(AstContext& ast, const SymbolTable* externalSymbolTable) -> SymbolTable
    {
        this->astContext          = &ast;
        this->externalSymbolTable = externalSymbolTable;
        TypeCheckVisitor{*this}.TraverseAst(ast);

        this->currentFunction     = nullptr;
        this->externalSymbolTable = nullptr;
        this->astContext          = nullptr;

        return std::move(symbolTable);
    }

    auto TypeChecker::InFunctionScope() -> bool
    {
        return currentFunction != nullptr;
    }

    auto TypeChecker::DeclareStruct(AstStructDecl& decl) -> void
    {
        decl.SetTypeDesc(astContext->CreateStructType(decl));
        for (auto memberDecl : decl.GetMembers()) {
            for (size_t i = 0; i < memberDecl->GetDeclarators().size(); ++i) {
                const auto& declarator = memberDecl->GetDeclarators()[i];
                decl.AddMemberDecl(declarator.declTok.text.Str(), DeclView{memberDecl, i});
            }
        }

        symbolTable.AddStructDecl(decl);
    }

    auto TypeChecker::DeclareInterfaceBlock(AstInterfaceBlockDecl& decl) -> void
    {
        // FIXME: Resolve a type, but the type shouldn't be registered into the symbol table
        decl.SetTypeDesc(astContext->CreateStructType(decl));
        for (auto memberDecl : decl.GetMembers()) {
            for (size_t i = 0; i < memberDecl->GetDeclarators().size(); ++i) {
                const auto& declarator = memberDecl->GetDeclarators()[i];
                decl.AddMemberDecl(declarator.declTok.text.Str(), DeclView{memberDecl, i});
            }
        }

        symbolTable.AddInterfaceBlockDecl(decl);
    }

    auto TypeChecker::DeclareVariable(AstVariableDecl& decl) -> void
    {
        // NOTE this could have 0 declarator. Then nothing is added.
        symbolTable.AddVariableDecl(decl);
    }

    auto TypeChecker::DeclareParameter(AstParamDecl& decl) -> void
    {
        symbolTable.AddParamDecl(decl);
    }

    // NOTE this is before children nodes are visited
    auto TypeChecker::EnterFunctionScope(AstFunctionDecl& decl) -> void
    {
        GLSLD_ASSERT(!InFunctionScope());

        currentFunction = &decl;
        symbolTable.PushScope();
    }

    auto TypeChecker::ExitFunctionScope(AstFunctionDecl& decl) -> void
    {
        GLSLD_ASSERT(InFunctionScope());

        currentFunction = nullptr;
        symbolTable.PopScope();

        symbolTable.AddFunction(decl);
    }

    auto TypeChecker::EnterBlockScope() -> void
    {
        GLSLD_ASSERT(InFunctionScope());
        symbolTable.PushScope();
    }

    auto TypeChecker::ExitBlockScope() -> void
    {
        GLSLD_ASSERT(InFunctionScope());
        symbolTable.PopScope();
    }

    auto TypeChecker::PreprocessInvokeExpr(AstInvokeExpr& expr) -> void
    {
        // FIXME: handle construct like `float[4](...)`
        if (auto func = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
            if (func->GetAccessName().klass != TokenKlass::Identifier) {
                // This is a constructor
                func->SetAccessType(NameAccessType::Constructor);
            }
            else {
                auto symbol = symbolTable.FindSymbol(func->GetAccessName().text.Str());
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
    }

    auto TypeChecker::CheckErrorExpr(AstErrorExpr& expr) -> void
    {
    }

    auto TypeChecker::CheckConstantExpr(AstConstantExpr& expr) -> void
    {
        // FIXME: handle literals with typing suffix
        switch (expr.GetToken().klass) {
        case TokenKlass::K_true:
            expr.SetConstValue(ConstValue::FromValue<BuiltinType::Ty_bool>(true));
            break;
        case TokenKlass::K_false:
            expr.SetConstValue(ConstValue::FromValue<BuiltinType::Ty_bool>(false));
            break;
        case TokenKlass::IntegerConstant:
            expr.SetConstValue(ParseIntegerLiteral(expr.GetToken().text.StrView()));
            break;
        case TokenKlass::FloatConstant:
            expr.SetConstValue(ParseFloatLiteral(expr.GetToken().text.StrView()));
            break;
        default:
            GLSLD_UNREACHABLE();
        }

        expr.SetDeducedType(expr.GetConstValue().GetTypeDesc());
    }

    auto TypeChecker::CheckNameAccessExpr(AstNameAccessExpr& expr) -> void
    {
        if (expr.GetAccessType() == NameAccessType::Unknown) {
            expr.SetAccessType(NameAccessType::Variable);
        }
        else {
            // NOTE we resolve function call in CheckInvokeExpr, so type checking for invoked
            // expression is there as well.
            return;
        }

        auto accessName = expr.GetAccessName().text.StrView();
        if (expr.GetAccessChain()) {
            // Accessing with the syntax `expr.xxx`

            auto accessChainType = expr.GetAccessChain()->GetDeducedType();
            if (accessChainType->IsScalar() || accessChainType->IsVector()) {
                // This is a swizzle access
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
                // bad access chain
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

                    // FIXME: handle array type
                    expr.SetDeducedType(
                        astContext->GetArrayType(varDecl->GetType()->GetResolvedType(), declarator.arraySize));
                }
                else if (auto paramDecl = symbol.GetDecl()->As<AstParamDecl>()) {
                    // Parameter

                    // FIXME: handle array type
                    expr.SetAccessedDecl(symbol);
                    expr.SetDeducedType(paramDecl->GetType()->GetResolvedType());
                }
                else if (auto blockDecl = symbol.GetDecl()->As<AstInterfaceBlockDecl>()) {
                    // Interface block

                    expr.SetAccessedDecl(symbol);
                    // FIXME: handle array type
                    expr.SetDeducedType(blockDecl->GetTypeDesc());
                }
                else if (auto memberDecl = symbol.GetDecl()->As<AstStructMemberDecl>()) {
                    // Unnamed interface block member

                    expr.SetAccessedDecl(symbol);
                    // FIXME: should we really use AstStructMemberDecl?
                    const auto& declarator = memberDecl->GetDeclarators()[symbol.GetIndex()];
                    expr.SetDeducedType(
                        astContext->GetArrayType(memberDecl->GetType()->GetResolvedType(), declarator.arraySize));
                }
            }
        }
    }

    auto TypeChecker::CheckIndexAccessExpr(AstIndexAccessExpr& expr) -> void
    {
        expr.SetDeducedType(GetErrorTypeDesc());

        if (!expr.GetArraySpec()) {
            return;
        }
        for (auto dimExpr : expr.GetArraySpec()->GetSizeList()) {
            if (!dimExpr || !dimExpr->GetDeducedType()->IsIntegralScalarType()) {
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

                expr.SetDeducedType(astContext->GetArrayType(arrayTypeDesc->elementType, dim2));
            }
        }
    }

    auto TypeChecker::CheckUnaryExpr(AstUnaryExpr& expr) -> void
    {
        expr.SetConstValue(EvaluateUnaryOp(expr.GetOperator(), expr.GetOperandExpr()->GetConstValue()));
        expr.SetDeducedType(EvalUnaryType(expr.GetOperator(), expr.GetOperandExpr()->GetDeducedType()));
    }

    auto TypeChecker::CheckBinaryExpr(AstBinaryExpr& expr) -> void
    {
        expr.SetConstValue(EvaluateBinaryOp(expr.GetOperator(), expr.GetLeftOperandExpr()->GetConstValue(),
                                            expr.GetRightOperandExpr()->GetConstValue()));
        expr.SetDeducedType(EvalBinaryType(expr.GetOperator(), expr.GetLeftOperandExpr()->GetDeducedType(),
                                           expr.GetRightOperandExpr()->GetDeducedType()));
    }

    auto TypeChecker::CheckSelectExpr(AstSelectExpr& expr) -> void
    {
        expr.SetConstValue(EvaluateSelectOp(expr.GetPredicateExpr()->GetConstValue(),
                                            expr.GetIfBranchExpr()->GetConstValue(),
                                            expr.GetElseBranchExpr()->GetConstValue()));
        expr.SetDeducedType(GetErrorTypeDesc());
    }

    auto TypeChecker::CheckInvokeExpr(AstInvokeExpr& expr) -> void
    {
        // resolve first
        ResolveInvokeExpr(expr);

        expr.SetDeducedType(GetErrorTypeDesc());

        // FIXME: handle function call
        // FIXME: handle things like `S[2](...)`
        if (auto invokedExpr = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
            if (invokedExpr->GetAccessName().klass != TokenKlass::Identifier) {
                // This is a constructor
                auto builtinType = GetBuiltinType(invokedExpr->GetAccessName());
                GLSLD_ASSERT(builtinType.has_value());
                expr.SetDeducedType(GetBuiltinTypeDesc(*builtinType));
                return;
            }
        }
    }
} // namespace glsld
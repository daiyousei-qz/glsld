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
                    return ConstValue{value};
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
                    return ConstValue{value};
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
                    return ConstValue{value};
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
                    return ConstValue{value};
                }
                else {
                    return ConstValue{};
                }
            }
        }
    } // namespace

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
            expr.SetConstValue(true);
            break;
        case TokenKlass::K_false:
            expr.SetConstValue(false);
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
        expr.SetDeducedType(EvalUnary(expr.GetOperator(), expr.GetOperandExpr()->GetDeducedType()));
    }

    auto TypeChecker::CheckBinaryExpr(AstBinaryExpr& expr) -> void
    {
        expr.SetConstValue(EvaluateBinaryOp(expr.GetOperator(), expr.GetLeftOperandExpr()->GetConstValue(),
                                            expr.GetRightOperandExpr()->GetConstValue()));
        expr.SetDeducedType(EvalBinary(expr.GetOperator(), expr.GetLeftOperandExpr()->GetDeducedType(),
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
#pragma once
#include "AstVisitor.h"
#include "SymbolTable.h"
#include "Typing.h"

#include <charconv>
#include <optional>

namespace glsld
{
    class TypeChecker
    {
    public:
        auto TypeCheck(AstContext& ast, const SymbolTable* externalSymbolTable) -> SymbolTable
        {
            this->astContext          = &ast;
            this->externalSymbolTable = externalSymbolTable;
            TypeCheckVisitor{*this}.TraverseAst(ast);

            this->currentFunction     = nullptr;
            this->externalSymbolTable = nullptr;
            this->astContext          = nullptr;

            return std::move(symbolTable);
        }

    private:
        //
        // Type
        //

        auto ResolveType(AstQualType& type) -> void
        {
            const TypeDesc* resolvedType = GetErrorTypeDesc();

            // Resolve element type
            if (auto builtinType = GetBuiltinType(type.GetTypeNameTok())) {
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
                    // FIXME: could also be interface decl
                    resolvedType = symbol.GetDecl()->As<AstStructDecl>()->GetTypeDesc();
                }
            }

            // Resolve array type if any
            if (type.GetArraySpec() != nullptr && !type.GetArraySpec()->GetSizeList().empty()) {
                resolvedType = astContext->GetArrayType(resolvedType, type.GetArraySpec());
            }

            type.SetResolvedType(resolvedType);
        }

        //
        // Global Decl
        //

        auto InFunctionScope() -> bool
        {
            return currentFunction != nullptr;
        }

        auto DeclareStructType(AstStructDecl& decl) -> void
        {
            decl.SetTypeDesc(astContext->CreateStructType(&decl));
            symbolTable.AddStructType(decl);
        }
        auto DeclareVariable(AstVariableDecl& decl) -> void
        {
            // NOTE this could have 0 declarator. Then nothing is added.
            symbolTable.AddVariableDecl(decl);
        }
        auto DeclareInterfaceBlock(AstInterfaceBlockDecl& decl) -> void
        {
            // FIXME: is this needed?
            // decl.SetTypeDesc(astContext->CreateStructType(&decl));
            symbolTable.AddInterfaceBlockType(decl);
        }
        auto DeclareParameter(AstParamDecl& decl) -> void
        {
            symbolTable.AddParamDecl(decl);
        }

        // NOTE this is before children nodes are visited
        auto EnterFunctionScope(AstFunctionDecl& decl) -> void
        {
            GLSLD_ASSERT(!InFunctionScope());

            currentFunction = &decl;
            symbolTable.PushScope();
        }
        auto ExitFunctionScope(AstFunctionDecl& decl) -> void
        {
            GLSLD_ASSERT(InFunctionScope());

            currentFunction = nullptr;
            symbolTable.PopScope();

            symbolTable.AddFunction(decl);
        }

        auto EnterBlockScope() -> void
        {
            GLSLD_ASSERT(InFunctionScope());
            symbolTable.PushScope();
        }
        auto ExitBlockScope() -> void
        {
            GLSLD_ASSERT(InFunctionScope());
            symbolTable.PopScope();
        }

        //
        // Expr
        //
        // PreprocessXXXExpr: action before children are type checked
        // CheckXXXExpr: action after children are type checked

        auto PreprocessInvokeExpr(AstInvokeExpr& expr) -> void
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

        auto CheckErrorExpr(AstErrorExpr& expr) -> void
        {
            expr.SetConstValue(ConstValue{});
            expr.SetDeducedType(GetErrorTypeDesc());
        }
        auto CheckConstantExpr(AstConstantExpr& expr) -> void
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
        auto CheckNameAccessExpr(AstNameAccessExpr& expr) -> void
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
                // Accessing with `expr.xxx`
                auto accessChainType = expr.GetAccessChain()->GetDeducedType();
                if (accessChainType->IsScalar() || accessChainType->IsVector()) {
                    // This is a swizzle access
                    ResolveSwizzleAccess(expr);
                }
                else if (auto structDesc = accessChainType->GetStructDesc()) {
                    // This is a field access
                    if (auto structDecl = structDesc->decl->As<AstStructDecl>()) {
                        for (auto memberDecl : structDecl->GetMembers()) {
                            size_t declaratorIndex = 0;
                            for (const auto& declarator : memberDecl->GetDeclarators()) {
                                if (declarator.declTok.text.StrView() == accessName) {
                                    expr.SetAccessedDecl(DeclView{memberDecl, declaratorIndex});
                                    break;
                                }

                                declaratorIndex += 1;
                            }

                            if (declaratorIndex < memberDecl->GetDeclarators().size()) {
                                break;
                            }
                        }
                    }
                    for (const auto& [memberName, memberType] : structDesc->members) {
                        if (memberName == accessName) {
                            expr.SetDeducedType(memberType);
                            // expr.SetAccessType(NameAccessType::MemberVariable);
                            break;
                        }
                    }
                }
                else {
                    // bad access chain
                }
            }
            else {
                // Accessing a non-member name
                GLSLD_ASSERT(expr.GetAccessType() == NameAccessType::Variable);
                if (auto symbol = symbolTable.FindSymbol(std::string{accessName})) {
                    if (auto varDecl = symbol.GetDecl()->As<AstVariableDecl>()) {
                        const auto& declarator = varDecl->GetDeclarators()[symbol.GetIndex()];

                        expr.SetAccessedDecl(symbol);
                        // FIXME: handle array type
                        if (declarator.arraySize != nullptr) {
                            expr.SetDeducedType(
                                astContext->GetArrayType(varDecl->GetType()->GetResolvedType(), declarator.arraySize));
                        }
                        else {
                            expr.SetDeducedType(varDecl->GetType()->GetResolvedType());
                        }
                    }
                    else if (auto paramDecl = symbol.GetDecl()->As<AstParamDecl>()) {
                        expr.SetAccessedDecl(symbol);
                        // FIXME: handle array type
                        expr.SetDeducedType(paramDecl->GetType()->GetResolvedType());
                    }
                    else if (auto memberDecl = symbol.GetDecl()->As<AstStructMemberDecl>()) {
                        // Unnamed interface block member
                        // FIXME: should we really use AstStructMemberDecl?
                        const auto& declarator = memberDecl->GetDeclarators()[symbol.GetIndex()];
                        // FIXME: handle array type
                        if (declarator.arraySize != nullptr) {
                            expr.SetDeducedType(astContext->GetArrayType(memberDecl->GetType()->GetResolvedType(),
                                                                         declarator.arraySize));
                        }
                        else {
                            expr.SetDeducedType(memberDecl->GetType()->GetResolvedType());
                        }
                    }
                }
                else {
                    expr.SetDeducedType(GetErrorTypeDesc());
                }
            }
        }
        auto CheckIndexAccessExpr(AstIndexAccessExpr& expr) -> void
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
                if (numIndexedDims < arrayTypeDesc->dimSizes.size()) {
                    // FIXME: deduce array type
                }
            }
        }
        auto CheckUnaryExpr(AstUnaryExpr& expr) -> void
        {
            expr.SetConstValue(EvaluateUnaryOp(expr.GetOperator(), expr.GetOperandExpr()->GetConstValue()));
            expr.SetDeducedType(EvalUnary(expr.GetOperator(), expr.GetOperandExpr()->GetDeducedType()));
        }
        auto CheckBinaryExpr(AstBinaryExpr& expr) -> void
        {
            expr.SetConstValue(EvaluateBinaryOp(expr.GetOperator(), expr.GetLeftOperandExpr()->GetConstValue(),
                                                expr.GetRightOperandExpr()->GetConstValue()));
            expr.SetDeducedType(EvalBinary(expr.GetOperator(), expr.GetLeftOperandExpr()->GetDeducedType(),
                                           expr.GetRightOperandExpr()->GetDeducedType()));
        }
        auto CheckSelectExpr(AstSelectExpr& expr) -> void
        {
            expr.SetConstValue(EvaluateSelectOp(expr.GetPredicateExpr()->GetConstValue(),
                                                expr.GetIfBranchExpr()->GetConstValue(),
                                                expr.GetElseBranchExpr()->GetConstValue()));
            expr.SetDeducedType(GetErrorTypeDesc());
        }
        auto CheckInvokeExpr(AstInvokeExpr& expr) -> void
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

        //
        // Type Eval
        //
        auto EvalUnary(UnaryOp op, const TypeDesc* operand) -> const TypeDesc*
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
            }

            GLSLD_UNREACHABLE();
        }

        auto EvalBinary(BinaryOp op, const TypeDesc* lhs, const TypeDesc* rhs) -> const TypeDesc*
        {
            // FIXME: implement this
            if (lhs->IsSameWith(rhs)) {
                return lhs;
            }
            else {
                return GetErrorTypeDesc();
            }
        }

    private:
        auto ResolveSwizzleAccess(AstNameAccessExpr& expr) -> void
        {
            GLSLD_ASSERT(expr.GetAccessChain()->GetDeducedType()->IsScalar() ||
                         expr.GetAccessChain()->GetDeducedType()->IsVector());
            auto swizzleName = expr.GetAccessName().text.StrView();

            if (swizzleName.size() <= 4) {
                // FIXME: set correct scalar type
                expr.SetAccessType(NameAccessType::Swizzle);
                expr.SetDeducedType(GetVectorTypeDesc(ScalarType::Float, swizzleName.size()));
            }
        }

        // The following type of expression could be invoked:
        // 1. `func()` where `func` is a function name
        // 2. `type()` where `type` is a type specifier
        // 3. `expr.length` where `expr` has array/vector type
        auto ResolveInvokeExpr(AstInvokeExpr& expr) -> void
        {
            if (auto invokedExpr = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                // Case 1: `ID()`
                auto accessName = invokedExpr->GetAccessName().text.Str();
                std::vector<const TypeDesc*> argTypes;
                for (auto argExpr : expr.GetArguments()) {
                    argTypes.push_back(argExpr->GetDeducedType());
                }

                switch (invokedExpr->GetAccessType()) {
                case NameAccessType::Constructor:
                {
                    if (auto builtinType = GetBuiltinType(invokedExpr->GetAccessName())) {
                        // Builtin types
                    }
                    else {
                        // User-defined types
                        auto symbol = symbolTable.FindSymbol(accessName);
                        if (symbol &&
                            (symbol.GetDecl()->Is<AstStructDecl>() || symbol.GetDecl()->Is<AstInterfaceBlockDecl>())) {
                            invokedExpr->SetAccessedDecl(symbol);
                        }
                    }

                    break;
                }
                case NameAccessType::Function:
                {
                    auto funcSymbol = FindFunction(accessName, argTypes);
                    if (funcSymbol) {
                        // FIXME: invoked expr should have deduced type of function?
                        invokedExpr->SetAccessedDecl(DeclView{funcSymbol});
                        expr.SetDeducedType(funcSymbol->GetReturnType()->GetResolvedType());
                    }
                    break;
                }
                default:
                    GLSLD_UNREACHABLE();
                }
            }
            else if (auto invokedExpr = expr.GetInvokedExpr()->As<AstIndexAccessExpr>();
                     invokedExpr && invokedExpr->GetInvokedExpr()->Is<AstNameAccessExpr>()) {
                // Case 2: `Type[...]()`
                invokedExpr->GetInvokedExpr()->As<AstNameAccessExpr>()->SetAccessType(NameAccessType::Constructor);
            }
        }

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
                typeChecker.DeclareStructType(decl);
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

        auto FindFunction(const std::string& name, const std::vector<const TypeDesc*>& argTypes) -> AstFunctionDecl*
        {
            if (auto funcDecl = symbolTable.FindFunction(name, argTypes)) {
                return funcDecl;
            }
            else {
                return externalSymbolTable ? externalSymbolTable->FindFunction(name, argTypes) : nullptr;
            }
        }

        auto FindSymbol(const std::string& name) -> DeclView
        {
            return symbolTable.FindSymbol(name);
        }

        AstContext* astContext                 = nullptr;
        const SymbolTable* externalSymbolTable = nullptr;

        AstFunctionDecl* currentFunction = nullptr;
        SymbolTable symbolTable;
    };
} // namespace glsld
#if !defined(DECL_EXPROP)
#error "missing DECL_EXPROP macro"
#else

/* error */
DECL_EXPROP(Error, AstErrorExpr)

/* constant */
DECL_EXPROP(Const, AstConstantExpr)
DECL_EXPROP(VarAccess, AstNameAccessExpr)

/* unary ops */
DECL_EXPROP(Identity, AstUnaryExpr)
DECL_EXPROP(Nagate, AstUnaryExpr)
DECL_EXPROP(BitwiseNot, AstUnaryExpr)
DECL_EXPROP(LogicalNot, AstUnaryExpr)
DECL_EXPROP(PrefixInc, AstUnaryExpr)
DECL_EXPROP(PrefixDec, AstUnaryExpr)
DECL_EXPROP(PostfixInc, AstUnaryExpr)
DECL_EXPROP(PostfixDec, AstUnaryExpr)

/* assignment ops */
DECL_EXPROP(Assign, AstBinaryExpr)
DECL_EXPROP(MulAssign, AstBinaryExpr)
DECL_EXPROP(DivAssign, AstBinaryExpr)
DECL_EXPROP(ModAssign, AstBinaryExpr)
DECL_EXPROP(AddAssign, AstBinaryExpr)
DECL_EXPROP(SubAssign, AstBinaryExpr)
DECL_EXPROP(LShiftAssign, AstBinaryExpr)
DECL_EXPROP(RShiftAssign, AstBinaryExpr)
DECL_EXPROP(AndAssign, AstBinaryExpr)
DECL_EXPROP(XorAssign, AstBinaryExpr)
DECL_EXPROP(OrAssign, AstBinaryExpr)

/* binary ops */
DECL_EXPROP(Comma, AstBinaryExpr)
DECL_EXPROP(Plus, AstBinaryExpr)
DECL_EXPROP(Minus, AstBinaryExpr)
DECL_EXPROP(Mul, AstBinaryExpr)
DECL_EXPROP(Div, AstBinaryExpr)
DECL_EXPROP(Modulo, AstBinaryExpr)
DECL_EXPROP(Equal, AstBinaryExpr)
DECL_EXPROP(NotEqual, AstBinaryExpr)
DECL_EXPROP(Less, AstBinaryExpr)
DECL_EXPROP(LessEq, AstBinaryExpr)
DECL_EXPROP(Greater, AstBinaryExpr)
DECL_EXPROP(GreaterEq, AstBinaryExpr)
DECL_EXPROP(BitwiseAnd, AstBinaryExpr)
DECL_EXPROP(BitwiseOr, AstBinaryExpr)
DECL_EXPROP(BitwiseXor, AstBinaryExpr)
DECL_EXPROP(LogicalAnd, AstBinaryExpr)
DECL_EXPROP(LogicalOr, AstBinaryExpr)
DECL_EXPROP(LogicalXor, AstBinaryExpr)
DECL_EXPROP(ShiftLeft, AstBinaryExpr)
DECL_EXPROP(ShiftRight, AstBinaryExpr)

/* binary ops */
DECL_EXPROP(Select, AstSelectExpr)

/* misc ops */
DECL_EXPROP(IndexAccess, AstInvokeExpr)
DECL_EXPROP(FunctionCall, AstInvokeExpr)

/* stmt ops */
DECL_EXPROP(CompoundStmt, AstCompoundStmt)
DECL_EXPROP(ExprStmt, AstExprStmt)
DECL_EXPROP(DeclStmt, AstDeclStmt)
DECL_EXPROP(IfStmt, AstIfStmt)
DECL_EXPROP(ForStmt, AstForStmt)
DECL_EXPROP(WhileStmt, AstWhileStmt)
DECL_EXPROP(SwitchStmt, AstSwitchStmt)
DECL_EXPROP(ContinueStmt, AstJumpStmt)
DECL_EXPROP(BreakStmt, AstJumpStmt)
DECL_EXPROP(DiscardStmt, AstJumpStmt)
DECL_EXPROP(ReturnStmt, AstReturnStmt)

#endif
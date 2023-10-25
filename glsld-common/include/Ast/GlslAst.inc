
DECL_AST_BEGIN_BASE(AstNodeBase)

// misc
DECL_AST_TYPE(AstTranslationUnit)
DECL_AST_TYPE(AstTypeQualifierSeq)
DECL_AST_TYPE(AstQualType)
DECL_AST_TYPE(AstArraySpec)
//DECL_AST_TYPE(AstDeclarator)

// expr
DECL_AST_BEGIN_BASE(AstInitializer)

DECL_AST_TYPE(AstInitializerList)

DECL_AST_BEGIN_BASE(AstExpr)
DECL_AST_TYPE(AstErrorExpr)
DECL_AST_TYPE(AstLiteralExpr)
DECL_AST_TYPE(AstNameAccessExpr)
DECL_AST_TYPE(AstMemberNameAccessExpr)
DECL_AST_TYPE(AstIndexAccessExpr)
DECL_AST_TYPE(AstUnaryExpr)
DECL_AST_TYPE(AstBinaryExpr)
DECL_AST_TYPE(AstSelectExpr)
DECL_AST_TYPE(AstInvokeExpr)
DECL_AST_TYPE(AstConstructorExpr)
DECL_AST_END_BASE(AstExpr)

DECL_AST_END_BASE(AstInitializer)

// stmt
DECL_AST_BEGIN_BASE(AstStmt)
DECL_AST_TYPE(AstErrorStmt)
DECL_AST_TYPE(AstEmptyStmt)
DECL_AST_TYPE(AstCompoundStmt)
DECL_AST_TYPE(AstExprStmt)
DECL_AST_TYPE(AstDeclStmt)
DECL_AST_TYPE(AstForStmt)
DECL_AST_TYPE(AstDoWhileStmt)
DECL_AST_TYPE(AstWhileStmt)
DECL_AST_TYPE(AstIfStmt)
DECL_AST_TYPE(AstLabelStmt)
DECL_AST_TYPE(AstSwitchStmt)
DECL_AST_TYPE(AstJumpStmt)
DECL_AST_TYPE(AstReturnStmt)
DECL_AST_END_BASE(AstStmt)

// decl
DECL_AST_BEGIN_BASE(AstDecl)
DECL_AST_TYPE(AstErrorDecl)
DECL_AST_TYPE(AstEmptyDecl)
//DECL_AST_TYPE(AstStructMemberDecl)
DECL_AST_TYPE(AstStructDecl)
DECL_AST_TYPE(AstVariableDecl)
DECL_AST_TYPE(AstParamDecl)
DECL_AST_TYPE(AstFunctionDecl)
DECL_AST_TYPE(AstInterfaceBlockDecl)
DECL_AST_END_BASE(AstDecl)

DECL_AST_END_BASE(AstNodeBase)

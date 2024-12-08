#include "AstTestFixture.h"

#include <catch2/catch_all.hpp>

using namespace glsld;

TEST_CASE_METHOD(AstTestFixture, "Simple Stmt")
{
    SetTemplate("void main() {{ {} }}", [](AstMatcher matcher) {
        return TranslationUnit(FunctionDecl(Any(), Any(), CompoundStmt(std::move(matcher))));
    });

    SECTION("EmptyStmt")
    {
        GLSLD_CHECK_AST(";", EmptyStmt());
    }

    SECTION("CompoundStmt")
    {
        GLSLD_CHECK_AST("{}", CompoundStmt());
        GLSLD_CHECK_AST("{;}", CompoundStmt(EmptyStmt()));
        GLSLD_CHECK_AST("{1;}", CompoundStmt(ExprStmt(LiteralExpr(1))));
        GLSLD_CHECK_AST("{;;}", CompoundStmt(EmptyStmt(), EmptyStmt()));

        GLSLD_CHECK_AST("{{}}", CompoundStmt(CompoundStmt()));
        GLSLD_CHECK_AST("{{} {}}", CompoundStmt(CompoundStmt(), CompoundStmt()));
        GLSLD_CHECK_AST("{{}; {}}", CompoundStmt(CompoundStmt(), EmptyStmt(), CompoundStmt()));
    }

    SECTION("ExprStmt")
    {
        GLSLD_CHECK_AST("1;", ExprStmt(LiteralExpr(1)));
        GLSLD_CHECK_AST("1 + 2;", ExprStmt(BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2))));
    }

    // TODO: DeclStmt

    SECTION("IfStmt")
    {
        GLSLD_CHECK_AST("if (true) {}", IfStmt(LiteralExpr(true), CompoundStmt()));
        GLSLD_CHECK_AST("if (true) ;", IfStmt(LiteralExpr(true), EmptyStmt()));

        GLSLD_CHECK_AST("if (true) {} else {}", IfStmt(LiteralExpr(true), CompoundStmt(), CompoundStmt()));
        GLSLD_CHECK_AST("if (true) ; else ;", IfStmt(LiteralExpr(true), EmptyStmt(), EmptyStmt()));

        GLSLD_CHECK_AST("if (true) {} else if (false) {}",
                        IfStmt(LiteralExpr(true), CompoundStmt(), IfStmt(LiteralExpr(false), CompoundStmt())));
        GLSLD_CHECK_AST(
            "if (true) {} else if (false) {} else {}",
            IfStmt(LiteralExpr(true), CompoundStmt(), IfStmt(LiteralExpr(false), CompoundStmt(), CompoundStmt())));
    }

    SECTION("WhileStmt")
    {
        GLSLD_CHECK_AST("while (true) {}", WhileStmt(LiteralExpr(true), CompoundStmt()));
        GLSLD_CHECK_AST("while (true) ;", WhileStmt(LiteralExpr(true), EmptyStmt()));

        GLSLD_CHECK_AST("while (true) while (false) {}",
                        WhileStmt(LiteralExpr(true), WhileStmt(LiteralExpr(false), CompoundStmt())));
    }

    SECTION("DoWhileStmt")
    {
        GLSLD_CHECK_AST("do {} while (true);", DoWhileStmt(CompoundStmt(), LiteralExpr(true)));
        GLSLD_CHECK_AST("do ; while (true);", DoWhileStmt(EmptyStmt(), LiteralExpr(true)));
    }

    SECTION("ForStmt")
    {
        GLSLD_CHECK_AST("for (;;) {}", ForStmt(EmptyStmt(), EmptyStmt(), EmptyStmt(), CompoundStmt()));
        GLSLD_CHECK_AST("for (1; 2; 3) {}", ForStmt(ExprStmt(LiteralExpr(1)), ExprStmt(LiteralExpr(2)),
                                                    ExprStmt(LiteralExpr(3)), CompoundStmt()));
    }

    SECTION("BreakStmt")
    {
        GLSLD_CHECK_AST("break;", BreakStmt());
    }

    SECTION("ContinueStmt")
    {
        GLSLD_CHECK_AST("continue;", ContinueStmt());
    }

    SECTION("ReturnStmt")
    {
        GLSLD_CHECK_AST("return;", ReturnStmt());
        GLSLD_CHECK_AST("return 1;", ReturnStmt(LiteralExpr(1)));
    }
}

TEST_CASE_METHOD(AstTestFixture, "Permissive Stmt")
{
}
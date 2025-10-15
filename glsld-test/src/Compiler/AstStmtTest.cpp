#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "AstStmtTest")
{
    SetTestTemplate("unknown foo() {{ {} }}", [this](AstMatcher* matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("foo"), {}, AnyStmt()),
                         FunctionDecl(AnyQualType(), IdTok("foo"), {}, CompoundStmt({matcher})));
    });

    SECTION("EmptyStmt")
    {
        GLSLD_CHECK_AST(";", EmptyStmt());
    }

    SECTION("CompoundStmt")
    {
        GLSLD_CHECK_AST("{}", CompoundStmt({}));
        GLSLD_CHECK_AST("{;}", CompoundStmt({
                                   EmptyStmt(),
                               }));
        GLSLD_CHECK_AST("{1;}", CompoundStmt({
                                    ExprStmt(LiteralExpr(1)),
                                }));
        GLSLD_CHECK_AST("{;;}", CompoundStmt({
                                    EmptyStmt(),
                                    EmptyStmt(),
                                }));

        GLSLD_CHECK_AST("{{}}", CompoundStmt({
                                    CompoundStmt({}),
                                }));
        GLSLD_CHECK_AST("{{} {}}", CompoundStmt({
                                       CompoundStmt({}),
                                       CompoundStmt({}),
                                   }));
        GLSLD_CHECK_AST("{{}; {}}", CompoundStmt({
                                        CompoundStmt({}),
                                        EmptyStmt(),
                                        CompoundStmt({}),
                                    }));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("{", CompoundStmt({}));
            GLSLD_CHECK_AST("{;", CompoundStmt({
                                      EmptyStmt(),
                                  }));
            GLSLD_CHECK_AST("{{", CompoundStmt({
                                      CompoundStmt({}),
                                  }));
        }
    }

    SECTION("ExprStmt")
    {
        GLSLD_CHECK_AST("1;", ExprStmt(LiteralExpr(1)));
        GLSLD_CHECK_AST("1 + 2;", ExprStmt(BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2))));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("1", ExprStmt(LiteralExpr(1)));
            GLSLD_CHECK_AST("{ 1 2 }", CompoundStmt({
                                           ExprStmt(LiteralExpr(1)),
                                           ExprStmt(LiteralExpr(2)),
                                       }));
        }
    }

    // TODO: DeclStmt

    SECTION("IfStmt")
    {
        GLSLD_CHECK_AST("if (true) {}", IfStmt(LiteralExpr(true), CompoundStmt({})));
        GLSLD_CHECK_AST("if (true) ;", IfStmt(LiteralExpr(true), EmptyStmt()));

        GLSLD_CHECK_AST("if (true) {} else {}", IfStmt(LiteralExpr(true), CompoundStmt({}), CompoundStmt({})));
        GLSLD_CHECK_AST("if (true) ; else ;", IfStmt(LiteralExpr(true), EmptyStmt(), EmptyStmt()));

        GLSLD_CHECK_AST("if (true) {} else if (false) {}",
                        IfStmt(LiteralExpr(true), CompoundStmt({}), IfStmt(LiteralExpr(false), CompoundStmt({}))));
        GLSLD_CHECK_AST("if (true) {} else if (false) {} else {}",
                        IfStmt(LiteralExpr(true), CompoundStmt({}),
                               IfStmt(LiteralExpr(false), CompoundStmt({}), CompoundStmt({}))));
    }

    SECTION("WhileStmt")
    {
        GLSLD_CHECK_AST("while (true) {}", WhileStmt(LiteralExpr(true), CompoundStmt({})));
        GLSLD_CHECK_AST("while (true) ;", WhileStmt(LiteralExpr(true), EmptyStmt()));

        GLSLD_CHECK_AST("while (true) while (false) {}",
                        WhileStmt(LiteralExpr(true), WhileStmt(LiteralExpr(false), CompoundStmt({}))));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("while", WhileStmt(ErrorExpr(), ErrorStmt()));
            GLSLD_CHECK_AST("while;", WhileStmt(ErrorExpr(), ErrorStmt()));
            GLSLD_CHECK_AST("while (", WhileStmt(ErrorExpr(), ErrorStmt()));
            GLSLD_CHECK_AST("while (1", WhileStmt(LiteralExpr(1), ErrorStmt()));
            GLSLD_CHECK_AST("while ()", WhileStmt(ErrorExpr(), ExprStmt(ErrorExpr())));
            GLSLD_CHECK_AST("while () ;", WhileStmt(ErrorExpr(), EmptyStmt()));
            GLSLD_CHECK_AST("while () {", WhileStmt(ErrorExpr(), CompoundStmt({})));
            GLSLD_CHECK_AST("while () {}", WhileStmt(ErrorExpr(), CompoundStmt({})));
        }
    }

    SECTION("DoWhileStmt")
    {
        GLSLD_CHECK_AST("do {} while (true);", DoWhileStmt(CompoundStmt({}), LiteralExpr(true)));
        GLSLD_CHECK_AST("do ; while (false);", DoWhileStmt(EmptyStmt(), LiteralExpr(false)));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("do {} while (true)", DoWhileStmt(CompoundStmt({}), LiteralExpr(true)));
            GLSLD_CHECK_AST("do", DoWhileStmt(ExprStmt(ErrorExpr()), ErrorExpr()));
            GLSLD_CHECK_AST("do {", DoWhileStmt(CompoundStmt({}), ErrorExpr()));
            GLSLD_CHECK_AST("do {} while (", DoWhileStmt(CompoundStmt({}), ErrorExpr()));
            GLSLD_CHECK_AST("do {} while (1", DoWhileStmt(CompoundStmt({}), LiteralExpr(1)));
            GLSLD_CHECK_AST("do {} while (1;", DoWhileStmt(CompoundStmt({}), LiteralExpr(1)));
            GLSLD_CHECK_AST("do {} while ()", DoWhileStmt(CompoundStmt({}), ErrorExpr()));
        }
    }

    SECTION("ForStmt")
    {
        GLSLD_CHECK_AST("for (;;) {}", ForStmt(EmptyStmt(), NullAst(), NullAst(), CompoundStmt({})));
        GLSLD_CHECK_AST("for (;;) ;", ForStmt(EmptyStmt(), NullAst(), NullAst(), EmptyStmt()));
        GLSLD_CHECK_AST("for (1; 2; 3) {}",
                        ForStmt(ExprStmt(LiteralExpr(1)), LiteralExpr(2), LiteralExpr(3), CompoundStmt({})));

        GLSLD_CHECK_AST("for (1; 2; 3) for (4; 5; 6) ;",
                        ForStmt(ExprStmt(LiteralExpr(1)), LiteralExpr(2), LiteralExpr(3),
                                ForStmt(ExprStmt(LiteralExpr(4)), LiteralExpr(5), LiteralExpr(6), EmptyStmt())));

        GLSLD_CHECK_AST(
            "for (int i = 0; i < 4; ++i) {}",
            ForStmt(DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), IdTok("i"), NullAst(), LiteralExpr(0))),
                    BinaryExpr(BinaryOp::Less, NameAccessExpr("i"), LiteralExpr(4)),
                    UnaryExpr(UnaryOp::PrefixInc, NameAccessExpr("i")), CompoundStmt({})));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("for", ForStmt(ErrorStmt(), ErrorExpr(), ErrorExpr(), ErrorStmt()));
            GLSLD_CHECK_AST("for;", ForStmt(ErrorStmt(), ErrorExpr(), ErrorExpr(), ErrorStmt()));
            GLSLD_CHECK_AST("for () ;", ForStmt(ExprStmt(ErrorExpr()), ErrorExpr(), ErrorExpr(), EmptyStmt()));
            // FIXME: this is weird parsing behavior. Could we do better?
            GLSLD_CHECK_AST("for (;) ;", ForStmt(EmptyStmt(), ErrorExpr(), ErrorExpr(), EmptyStmt()));
        }
    }

    SECTION("SwitchStmt")
    {
        GLSLD_CHECK_AST("switch (1) {}", SwitchStmt(LiteralExpr(1), CompoundStmt({})));
        GLSLD_CHECK_AST("switch (1) { case 2: break; default: break; }",
                        SwitchStmt(LiteralExpr(1), CompoundStmt({CaseLabelStmt(LiteralExpr(2)), BreakStmt(),
                                                                 DefaultLabelStmt(), BreakStmt()})));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("switch", SwitchStmt(ErrorExpr(), ErrorStmt()));
            GLSLD_CHECK_AST("switch (", SwitchStmt(ErrorExpr(), ErrorStmt()));
            GLSLD_CHECK_AST("switch (1", SwitchStmt(LiteralExpr(1), ErrorStmt()));
            GLSLD_CHECK_AST("switch ()", SwitchStmt(ErrorExpr(), ErrorStmt()));
            GLSLD_CHECK_AST("switch () {}", SwitchStmt(ErrorExpr(), CompoundStmt({})));
            GLSLD_CHECK_AST("switch {case 1: break;}", SwitchStmt(ErrorExpr(), ErrorStmt()));
            GLSLD_CHECK_AST("switch () {break;}", SwitchStmt(ErrorExpr(), CompoundStmt({BreakStmt()})));
            GLSLD_CHECK_AST("switch () {case 1: break}",
                            SwitchStmt(ErrorExpr(), CompoundStmt({CaseLabelStmt(LiteralExpr(1)), BreakStmt()})));
            GLSLD_CHECK_AST("switch () {case: break}",
                            SwitchStmt(ErrorExpr(), CompoundStmt({CaseLabelStmt(ErrorExpr()), BreakStmt()})));
            GLSLD_CHECK_AST("switch () {case 1 break default break}",
                            SwitchStmt(ErrorExpr(), CompoundStmt({CaseLabelStmt(LiteralExpr(1)), BreakStmt(),
                                                                  DefaultLabelStmt(), BreakStmt()})));
        }
    }

    SECTION("JumpStmt")
    {
        GLSLD_CHECK_AST("break;", BreakStmt());
        GLSLD_CHECK_AST("continue;", ContinueStmt());
        GLSLD_CHECK_AST("discard;", DiscardStmt());

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("break", BreakStmt());
            GLSLD_CHECK_AST("{ continue continue }", CompoundStmt({
                                                         ContinueStmt(),
                                                         ContinueStmt(),
                                                     }));
        }
    }

    SECTION("ReturnStmt")
    {
        GLSLD_CHECK_AST("return;", ReturnStmt());
        GLSLD_CHECK_AST("return 1;", ReturnStmt(LiteralExpr(1)));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("return", ReturnStmt(ErrorExpr()));
            // FIXME: support this test
            // GLSLD_CHECK_AST("{ return return }", CompoundStmt({ReturnStmt(), ReturnStmt()}));
            GLSLD_CHECK_AST("{ return x return }", CompoundStmt({
                                                       ReturnStmt(NameAccessExpr("x")),
                                                       ReturnStmt(ErrorExpr()),
                                                   }));
        }
    }
}
#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Compiler::AstStmtTest")
{
    SetTestTemplate("unknown foo() {{ {} }}", [this](AstMatcher* matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("foo"), {}, AnyStmt()),
                         FunctionDecl(AnyQualType(), IdTok("foo"), {}, CompoundStmt({matcher})));
    });

    SECTION("EmptyStmt")
    {
        CheckAst(";", EmptyStmt());
    }

    SECTION("CompoundStmt")
    {
        CheckAst("{}", CompoundStmt({}));
        CheckAst("{;}", CompoundStmt({
                            EmptyStmt(),
                        }));
        CheckAst("{1;}", CompoundStmt({
                             ExprStmt(LiteralExpr(1)),
                         }));
        CheckAst("{;;}", CompoundStmt({
                             EmptyStmt(),
                             EmptyStmt(),
                         }));

        CheckAst("{{}}", CompoundStmt({
                             CompoundStmt({}),
                         }));
        CheckAst("{{} {}}", CompoundStmt({
                                CompoundStmt({}),
                                CompoundStmt({}),
                            }));
        CheckAst("{{}; {}}", CompoundStmt({
                                 CompoundStmt({}),
                                 EmptyStmt(),
                                 CompoundStmt({}),
                             }));
        CheckAst("{ int i = 0; return; }",
                 CompoundStmt({
                     DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), IdTok("i"), NullAst(), LiteralExpr(0))),
                     ReturnStmt(),
                 }));

        SECTION("Permissive")
        {
            CheckAst("{", CompoundStmt({}));
            CheckAst("{;", CompoundStmt({
                               EmptyStmt(),
                           }));
            CheckAst("{{", CompoundStmt({
                               CompoundStmt({}),
                           }));
        }
    }

    SECTION("ExprStmt")
    {
        CheckAst("1;", ExprStmt(LiteralExpr(1)));
        CheckAst("1 + 2;", ExprStmt(BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2))));

        SECTION("Permissive")
        {
            CheckAst("1", ExprStmt(LiteralExpr(1)));
            CheckAst("{ 1 2 }", CompoundStmt({
                                    ExprStmt(LiteralExpr(1)),
                                    ExprStmt(LiteralExpr(2)),
                                }));
        }
    }

    SECTION("DeclStmt")
    {
        CheckAst("int v;", DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), IdTok("v"), NullAst(), NullAst())));
        CheckAst("int v = 1;",
                 DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), IdTok("v"), NullAst(), LiteralExpr(1))));
        CheckAst("int v[2];", DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), IdTok("v"),
                                                    ArraySpec({LiteralExpr(2)}), NullAst())));

        SECTION("Permissive")
        {
            CheckAst("int v", DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), IdTok("v"), NullAst(), NullAst())));
            CheckAst("int v =", DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), IdTok("v"), NullAst(),
                                                      ImplicitCastExpr(ErrorExpr()))));
            CheckAst("int;", DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), {})));
        }
    }

    SECTION("IfStmt")
    {
        CheckAst("if (true) {}", IfStmt(LiteralExpr(true), CompoundStmt({})));
        CheckAst("if (true) ;", IfStmt(LiteralExpr(true), EmptyStmt()));

        CheckAst("if (true) {} else {}", IfStmt(LiteralExpr(true), CompoundStmt({}), CompoundStmt({})));
        CheckAst("if (true) ; else ;", IfStmt(LiteralExpr(true), EmptyStmt(), EmptyStmt()));

        CheckAst("if (true) {} else if (false) {}",
                 IfStmt(LiteralExpr(true), CompoundStmt({}), IfStmt(LiteralExpr(false), CompoundStmt({}))));
        CheckAst("if (true) {} else if (false) {} else {}",
                 IfStmt(LiteralExpr(true), CompoundStmt({}),
                        IfStmt(LiteralExpr(false), CompoundStmt({}), CompoundStmt({}))));
        CheckAst("if (true) return;", IfStmt(LiteralExpr(true), ReturnStmt()));
        CheckAst("if (true) { return 1; } else discard;",
                 IfStmt(LiteralExpr(true), CompoundStmt({ReturnStmt(LiteralExpr(1))}), DiscardStmt()));
        CheckAst("if (true) {} else if (false) return;",
                 IfStmt(LiteralExpr(true), CompoundStmt({}), IfStmt(LiteralExpr(false), ReturnStmt())));
    }

    SECTION("WhileStmt")
    {
        CheckAst("while (true) {}", WhileStmt(LiteralExpr(true), CompoundStmt({})));
        CheckAst("while (true) ;", WhileStmt(LiteralExpr(true), EmptyStmt()));

        CheckAst("while (true) while (false) {}",
                 WhileStmt(LiteralExpr(true), WhileStmt(LiteralExpr(false), CompoundStmt({}))));
        CheckAst("while (true) { continue; }", WhileStmt(LiteralExpr(true), CompoundStmt({ContinueStmt()})));

        SECTION("Permissive")
        {
            CheckAst("while", WhileStmt(ErrorExpr(), ErrorStmt()));
            CheckAst("while;", WhileStmt(ErrorExpr(), ErrorStmt()));
            CheckAst("while (", WhileStmt(ErrorExpr(), ErrorStmt()));
            CheckAst("while (1", WhileStmt(LiteralExpr(1), ErrorStmt()));
            CheckAst("while ()", WhileStmt(ErrorExpr(), ExprStmt(ErrorExpr())));
            CheckAst("while () ;", WhileStmt(ErrorExpr(), EmptyStmt()));
            CheckAst("while () {", WhileStmt(ErrorExpr(), CompoundStmt({})));
            CheckAst("while () {}", WhileStmt(ErrorExpr(), CompoundStmt({})));
        }
    }

    SECTION("DoWhileStmt")
    {
        CheckAst("do {} while (true);", DoWhileStmt(CompoundStmt({}), LiteralExpr(true)));
        CheckAst("do ; while (false);", DoWhileStmt(EmptyStmt(), LiteralExpr(false)));
        CheckAst("do { break; } while (true);", DoWhileStmt(CompoundStmt({BreakStmt()}), LiteralExpr(true)));

        SECTION("Permissive")
        {
            CheckAst("do {} while (true)", DoWhileStmt(CompoundStmt({}), LiteralExpr(true)));
            CheckAst("do", DoWhileStmt(ExprStmt(ErrorExpr()), ErrorExpr()));
            CheckAst("do {", DoWhileStmt(CompoundStmt({}), ErrorExpr()));
            CheckAst("do {} while (", DoWhileStmt(CompoundStmt({}), ErrorExpr()));
            CheckAst("do {} while (1", DoWhileStmt(CompoundStmt({}), LiteralExpr(1)));
            CheckAst("do {} while (1;", DoWhileStmt(CompoundStmt({}), LiteralExpr(1)));
            CheckAst("do {} while ()", DoWhileStmt(CompoundStmt({}), ErrorExpr()));
        }
    }

    SECTION("ForStmt")
    {
        CheckAst("for (;;) {}", ForStmt(EmptyStmt(), NullAst(), NullAst(), CompoundStmt({})));
        CheckAst("for (;;) ;", ForStmt(EmptyStmt(), NullAst(), NullAst(), EmptyStmt()));
        CheckAst("for (1; 2; 3) {}",
                 ForStmt(ExprStmt(LiteralExpr(1)), LiteralExpr(2), LiteralExpr(3), CompoundStmt({})));

        CheckAst("for (1; 2; 3) for (4; 5; 6) ;",
                 ForStmt(ExprStmt(LiteralExpr(1)), LiteralExpr(2), LiteralExpr(3),
                         ForStmt(ExprStmt(LiteralExpr(4)), LiteralExpr(5), LiteralExpr(6), EmptyStmt())));

        CheckAst("for (int i = 0; i < 4; ++i) {}",
                 ForStmt(DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), IdTok("i"), NullAst(), LiteralExpr(0))),
                         BinaryExpr(BinaryOp::Less, NameAccessExpr("i"), LiteralExpr(4)),
                         UnaryExpr(UnaryOp::PrefixInc, NameAccessExpr("i")), CompoundStmt({})));
        CheckAst("for (int i = 0; i < 10; ) break;",
                 ForStmt(DeclStmt(VariableDecl(NamedType(TokenKlass::K_int), IdTok("i"), NullAst(), LiteralExpr(0))),
                         BinaryExpr(BinaryOp::Less, NameAccessExpr("i"), LiteralExpr(10)), NullAst(), BreakStmt()));

        SECTION("Permissive")
        {
            CheckAst("for", ForStmt(ErrorStmt(), ErrorExpr(), ErrorExpr(), ErrorStmt()));
            CheckAst("for;", ForStmt(ErrorStmt(), ErrorExpr(), ErrorExpr(), ErrorStmt()));
            CheckAst("for () ;", ForStmt(ExprStmt(ErrorExpr()), ErrorExpr(), ErrorExpr(), EmptyStmt()));
            // FIXME: this is weird parsing behavior. Could we do better?
            CheckAst("for (;) ;", ForStmt(EmptyStmt(), ErrorExpr(), ErrorExpr(), EmptyStmt()));
        }
    }

    SECTION("SwitchStmt")
    {
        CheckAst("switch (1) {}", SwitchStmt(LiteralExpr(1), CompoundStmt({})));
        CheckAst("switch (1) { case 2: break; default: break; }",
                 SwitchStmt(LiteralExpr(1), CompoundStmt({CaseLabelStmt(LiteralExpr(2)), BreakStmt(),
                                                          DefaultLabelStmt(), BreakStmt()})));
        CheckAst("switch (value) { case 1: return; case 2: continue; default: discard; }",
                 SwitchStmt(NameAccessExpr("value"),
                            CompoundStmt({CaseLabelStmt(LiteralExpr(1)), ReturnStmt(), CaseLabelStmt(LiteralExpr(2)),
                                          ContinueStmt(), DefaultLabelStmt(), DiscardStmt()})));

        SECTION("Permissive")
        {
            CheckAst("switch", SwitchStmt(ErrorExpr(), ErrorStmt()));
            CheckAst("switch (", SwitchStmt(ErrorExpr(), ErrorStmt()));
            CheckAst("switch (1", SwitchStmt(LiteralExpr(1), ErrorStmt()));
            CheckAst("switch ()", SwitchStmt(ErrorExpr(), ErrorStmt()));
            CheckAst("switch () {}", SwitchStmt(ErrorExpr(), CompoundStmt({})));
            CheckAst("switch {case 1: break;}", SwitchStmt(ErrorExpr(), ErrorStmt()));
            CheckAst("switch () {break;}", SwitchStmt(ErrorExpr(), CompoundStmt({BreakStmt()})));
            CheckAst("switch () {case 1: break}",
                     SwitchStmt(ErrorExpr(), CompoundStmt({CaseLabelStmt(LiteralExpr(1)), BreakStmt()})));
            CheckAst("switch () {case: break}",
                     SwitchStmt(ErrorExpr(), CompoundStmt({CaseLabelStmt(ErrorExpr()), BreakStmt()})));
            CheckAst("switch () {case 1 break default break}",
                     SwitchStmt(ErrorExpr(), CompoundStmt({CaseLabelStmt(LiteralExpr(1)), BreakStmt(),
                                                           DefaultLabelStmt(), BreakStmt()})));
        }
    }

    SECTION("JumpStmt")
    {
        CheckAst("break;", BreakStmt());
        CheckAst("continue;", ContinueStmt());
        CheckAst("discard;", DiscardStmt());

        SECTION("Permissive")
        {
            CheckAst("break", BreakStmt());
            CheckAst("{ continue continue }", CompoundStmt({
                                                  ContinueStmt(),
                                                  ContinueStmt(),
                                              }));
        }
    }

    SECTION("ReturnStmt")
    {
        CheckAst("return;", ReturnStmt());
        CheckAst("return 1;", ReturnStmt(LiteralExpr(1)));
        CheckAst("return foo;", ReturnStmt(NameAccessExpr("foo")));

        SECTION("Permissive")
        {
            CheckAst("return", ReturnStmt(ErrorExpr()));
            // FIXME: support this test
            // CheckAst("{ return return }", CompoundStmt({ReturnStmt(), ReturnStmt()}));
            CheckAst("{ return x return }", CompoundStmt({
                                                ReturnStmt(NameAccessExpr("x")),
                                                ReturnStmt(ErrorExpr()),
                                            }));
        }
    }
}
#include "AstTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(AstTestFixture, "Simple Const Eval")
{

    SetTestTemplate("unknown x = {};", [](AstMatcher matcher) {
        return TranslationUnit(VariableDecl1(AnyQualType(), std::move(matcher)));
    });
}
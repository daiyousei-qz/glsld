#include "AstBase.h"
#include "AstPrinter.h"

namespace glsld
{
    auto AstNodeTagToString(AstNodeTag tag) -> std::string_view
    {
        switch (tag) {
        case AstNodeTag::Invalid:
            return "Invalid";

#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
        return #TYPE;
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE
        default:
            GLSLD_UNREACHABLE();
        }
    };

    auto AstNodeBase::Dump() -> void
    {
        AstPrinter printer;
        printer.Traverse(*this);
        fmt::print(stderr, "{}", printer.Export());
    }
} // namespace glsld
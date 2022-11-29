#include "AstBase.h"
#include "AstPrinter.h"

namespace glsld
{
    auto AstNodeBase::Dump() -> void
    {
        AstPrinter printer;
        printer.Traverse(*this);
        fmt::print(stderr, "{}", printer.Export());
    }
} // namespace glsld
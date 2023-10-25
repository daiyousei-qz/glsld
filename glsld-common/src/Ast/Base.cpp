#include "Ast/Base.h"
#include "Ast/AstDumper.h"

namespace glsld
{
    auto AstNode::Print() const -> void
    {
        AstDumper dumper;
        dumper.DumpChildNode("AstNode", *this);
        fmt::print("{}\n", dumper.GetBufferView());
    }
} // namespace glsld
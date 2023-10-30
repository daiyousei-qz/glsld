#include "Basic/Print.h"
#include "Ast/Base.h"
#include "Ast/AstDumper.h"

namespace glsld
{
    auto AstNode::Print() const -> void
    {
        AstDumper dumper;
        dumper.DumpChildNode("AstNode", *this);
        glsld::Print("{}\n", dumper.GetBufferView());
    }

    auto AstNode::DebugPrint() const -> void
    {
        AstDumper dumper;
        dumper.DumpChildNode("AstNode", *this);
        glsld::DebugPrint("{}\n", dumper.GetBufferView());
    }
} // namespace glsld
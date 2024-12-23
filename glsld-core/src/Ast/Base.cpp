#include "Basic/Print.h"
#include "Ast/Base.h"
#include "Ast/AstDumper.h"

namespace glsld
{
    auto AstNode::Print() const -> std::string
    {
        AstDumper dumper;
        dumper.DumpChildNode("AstNode", *this);
        return dumper.GetBufferView().Str();
    }

    auto AstNode::DebugPrint() const -> void
    {
        glsld::DebugPrint("{}\n", Print());
    }
} // namespace glsld
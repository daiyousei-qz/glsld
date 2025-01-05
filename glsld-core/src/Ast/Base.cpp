#include "Ast/Base.h"
#include "Ast/AstPrinter.h"

namespace glsld
{
    auto AstNode::ToString() const -> std::string
    {
        AstPrinter printer;
        printer.PrintChildNode("AstNode", *this);
        return printer.GetBufferView().Str();
    }

    auto AstNode::Dump() const -> void
    {
        glsld::DebugPrint("{}\n", ToString());
    }
} // namespace glsld
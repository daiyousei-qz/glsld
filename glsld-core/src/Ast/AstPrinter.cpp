#include "Ast/AstPrinter.h"
#include "Ast/Dispatch.h"

namespace glsld
{
    auto AstPrinter::PrintChildNode(StringView key, const AstNode& node) -> void
    {
        PrintIndentation();
        fmt::format_to(std::back_inserter(buffer), "+{}: {}[{}, {}) #{:x} -> {{\n", key,
                       AstNodeTagToString(node.GetTag()), node.GetSyntaxRange().GetBeginID().GetTokenIndex(),
                       node.GetSyntaxRange().GetEndID().GetTokenIndex(), GetPointerIdentifier(&node));

        PushIndent();
        InvokeAstDispatched(node,
                            [this]<typename AstType>(const AstType& dispatchedNode) { dispatchedNode.DoPrint(*this); });
        PopIndent();

        PrintIndentation();
        fmt::format_to(std::back_inserter(buffer), "}}\n", key, AstNodeTagToString(node.GetTag()));
    }

    auto AstPrinter::PrintChildItem(StringView key, std::function<void(AstPrinter&)> callback) -> void
    {
        PrintIndentation();
        fmt::format_to(std::back_inserter(buffer), "-{} -> {{\n", key);

        PushIndent();
        callback(*this);
        PopIndent();

        PrintIndentation();
        fmt::format_to(std::back_inserter(buffer), "}}\n");
    }
} // namespace glsld
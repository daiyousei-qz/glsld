#pragma once
#include "Ast.h"
#include "AstVisitor.h"

#include <fmt/format.h>

namespace glsld
{
    class AstPrinter : public AstVisitor<AstPrinter>
    {
    public:
        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            PrintIdent(true);
            depth += 1;
            Print("@{}[{}]", AstNodeTagToString(node.GetTag()), static_cast<const void*>(&node));
            return AstVisitPolicy::Traverse;
        }
        auto VisitAstNodeBase(AstNodeBase& node) -> void
        {
            node.DispatchInvoke([this]<typename AstType>(AstType& dispatchedNode) {
                auto printDumpedData = [&](const std::string& data) {
                    if (!data.empty()) {
                        PrintIdent();
                        Print("{}", data);
                    }
                };

                printDumpedData(dispatchedNode.DumpNodeData());
                printDumpedData(dispatchedNode.DumpPayloadData());
            });
        }

        auto ExitAstNodeBase(AstNodeBase& node) -> void
        {
            depth -= 1;
        }

        auto Export() -> std::string
        {
            buffer.push_back('\n');
            return std::move(buffer);
        }

    private:
        template <typename... Args>
        auto Print(fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
        }

        auto PrintIdent(bool isNode = false) -> void
        {
            if (!buffer.empty()) {
                Print("\n");
            }

            for (int i = 0; i < depth; ++i) {
                if (isNode && i + 1 == depth) {
                    Print("'--");
                }
                else {
                    Print("|  ");
                }
            }
        }

        std::string buffer;
        int depth = 0;
    };
} // namespace glsld
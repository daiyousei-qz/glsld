#pragma once
#include "Basic/Common.h"
#include "Ast/Dispatch.h"
#include "Ast/AstVisitor.h"

namespace glsld
{
    class AstDumper
    {
    private:
        size_t indentDepth = 0;

    public:
        template <typename T>
        auto DumpAttribute(StringView key, T value) -> void
        {
            PrintIndentation();
            fmt::print("@ {} = {}\n", key, value);
        }
        auto DumpChildNode(StringView key, const AstNode& node) -> void
        {
            PrintIndentation();
            fmt::print("+ {}: {} -> {{\n", key, AstNodeTagToString(node.GetTag()));

            PushIndent();
            InvokeAstDispatched(
                node, [this]<typename AstType>(const AstType& dispatchedNode) { dispatchedNode.Dump(*this); });
            PopIndent();

            PrintIndentation();
            fmt::print("}}\n", key, AstNodeTagToString(node.GetTag()));
        }
        auto DumpChildItem(StringView key, std::function<void(AstDumper&)> callback) -> void
        {
            PushIndent();
            PopIndent();
        }

    private:
        auto PushIndent() -> void
        {
            indentDepth += 1;
        }

        auto PopIndent() -> void
        {
            indentDepth -= 1;
        }

        auto PrintIndentation() -> void
        {
            for (size_t i = 0; i < indentDepth; ++i) {
                fmt::print("  ");
            }
        }
    };

    inline auto DumpAst(AstNode& node) -> void
    {
        AstDumper dumper;
        dumper.DumpChildNode("TopLevel", node);
    }

    // class AstPrinter : public AstVisitor<AstPrinter>
    // {
    // private:
    //     std::string buffer;
    //     int depth         = 0;
    //     bool printPayload = false;

    // public:
    //     AstPrinter(bool printPayload = false) : printPayload(printPayload)
    //     {
    //     }

    //     auto EnterAstNode(AstNode& node) -> AstVisitPolicy
    //     {
    //         PrintIdent(true);
    //         depth += 1;
    //         Print("@{}[{}]", AstNodeTagToString(node.GetTag()), static_cast<const void*>(&node));
    //         return AstVisitPolicy::Traverse;
    //     }
    //     auto VisitAstNode(AstNode& node) -> void
    //     {
    //         InvokeAstDispatched(node, [this]<typename AstType>(AstType& dispatchedNode) {
    //             auto printDumpedData = [&](const std::string& data) {
    //                 if (!data.empty()) {
    //                     PrintIdent();
    //                     Print("{}", data);
    //                 }
    //             };

    //             printDumpedData(dispatchedNode.DumpNodeData());
    //             if (printPayload) {
    //                 printDumpedData(dispatchedNode.DumpPayloadData());
    //             }
    //         });
    //     }

    //     auto ExitAstNode(AstNode& node) -> void
    //     {
    //         depth -= 1;
    //     }

    //     auto Export() -> std::string
    //     {
    //         buffer.push_back('\n');
    //         return std::move(buffer);
    //     }

    // private:
    //     template <typename... Args>
    //     auto Print(fmt::format_string<Args...> fmt, Args&&... args) -> void
    //     {
    //         fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
    //     }

    //     auto PrintIdent(bool isNode = false) -> void
    //     {
    //         if (!buffer.empty()) {
    //             Print("\n");
    //         }

    //         for (int i = 0; i < depth; ++i) {
    //             if (isNode && i + 1 == depth) {
    //                 Print("'--");
    //             }
    //             else {
    //                 Print("|  ");
    //             }
    //         }
    //     }
    // };

    // inline auto AstNodeToString(AstNode* node, bool printPayload = true) -> std::string
    // {
    //     AstPrinter printer{printPayload};
    //     printer.Traverse(*node);
    //     return printer.Export();
    // }
} // namespace glsld
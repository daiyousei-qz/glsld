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
        size_t indentSize  = 4;

        fmt::memory_buffer buffer;

    public:
        AstDumper() = default;

        auto GetAstNodeIdentifier(const AstNode& node) -> uint64_t
        {
            return reinterpret_cast<uint64_t>(&node);
        }

        template <typename T>
        auto DumpAttribute(StringView key, T value) -> void
        {
            PrintIndentation();
            fmt::format_to(std::back_inserter(buffer), "@{} = {}\n", key, value);
        }
        auto DumpChildNode(StringView key, const AstNode& node) -> void
        {
            PrintIndentation();
            fmt::format_to(std::back_inserter(buffer), "+{}: {} -> {{\n", key, AstNodeTagToString(node.GetTag()));

            PushIndent();
            InvokeAstDispatched(
                node, [this]<typename AstType>(const AstType& dispatchedNode) { dispatchedNode.Dump(*this); });
            PopIndent();

            PrintIndentation();
            fmt::format_to(std::back_inserter(buffer), "}}\n", key, AstNodeTagToString(node.GetTag()));
        }
        auto DumpChildItem(StringView key, std::function<void(AstDumper&)> callback) -> void
        {
            PushIndent();
            PopIndent();
        }

        auto GetBufferView() const noexcept -> StringView
        {
            return {buffer.data(), buffer.size()};
        }

        auto Export() -> std::string
        {
            auto result = fmt::to_string(buffer);
            buffer.clear();
            return result;
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
            for (size_t i = 0; i < indentDepth * indentSize; ++i) {
                buffer.push_back(' ');
            }
        }
    };

    inline auto DumpAst(AstNode& node) -> void
    {
        AstDumper dumper;
        dumper.DumpChildNode("TopLevel", node);
    }
} // namespace glsld
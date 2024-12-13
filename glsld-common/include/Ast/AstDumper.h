#pragma once
#include "Basic/Common.h"
#include "Ast/Dispatch.h"
#include "Ast/AstVisitor.h"

#include <functional>

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

        auto GetPointerIdentifier(const void* ptr) -> uintptr_t
        {
            return reinterpret_cast<uintptr_t>(ptr);
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
            fmt::format_to(std::back_inserter(buffer), "+{}: {}[{}, {}) #{:x} -> {{\n", key,
                           AstNodeTagToString(node.GetTag()), node.GetSyntaxRange().GetBeginID().GetTokenIndex(),
                           node.GetSyntaxRange().GetEndID().GetTokenIndex(), GetPointerIdentifier(&node));

            PushIndent();
            InvokeAstDispatched(
                node, [this]<typename AstType>(const AstType& dispatchedNode) { dispatchedNode.DoDump(*this); });
            PopIndent();

            PrintIndentation();
            fmt::format_to(std::back_inserter(buffer), "}}\n", key, AstNodeTagToString(node.GetTag()));
        }
        auto DumpChildItem(StringView key, std::function<void(AstDumper&)> callback) -> void
        {
            PrintIndentation();
            fmt::format_to(std::back_inserter(buffer), "-{} -> {{\n", key);

            PushIndent();
            callback(*this);
            PopIndent();

            PrintIndentation();
            fmt::format_to(std::back_inserter(buffer), "}}\n");
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
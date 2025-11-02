#pragma once
#include "Basic/Print.h"
#include "Support/StringView.h"

#include <functional>

namespace glsld
{
    class AstNode;

    class AstPrinter
    {
    private:
        size_t indentDepth = 0;
        size_t indentSize  = 4;

        fmt::memory_buffer buffer;

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

    public:
        AstPrinter() = default;

        auto GetPointerIdentifier(const void* ptr) -> uintptr_t
        {
            return reinterpret_cast<uintptr_t>(ptr);
        }

        template <typename T>
        auto PrintAttribute(StringView key, T value) -> void
        {
            PrintIndentation();
            fmt::format_to(std::back_inserter(buffer), "@{} = {}\n", key, value);
        }
        auto PrintChildNode(StringView key, const AstNode& node) -> void;
        auto PrintChildItem(StringView key, std::function<void(AstPrinter&)> callback) -> void;

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
    };
} // namespace glsld
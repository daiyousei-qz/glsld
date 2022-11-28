#pragma once
#include "Ast.h"
#include "AstVisitor.h"

#include <fmt/format.h>

namespace glsld
{
    class AstPrinter : public glsld::AstVisitor<AstPrinter>
    {
    public:
        auto VisitAstNodeBase(glsld::AstNodeBase& node) -> void
        {
            PrintIdent();
            depth += 1;
            fmt::print(stderr, "{}\n", glsld::AstNodeTagToString(node.GetTag()));
        }
        auto ExitAstNodeBase(glsld::AstNodeBase& node) -> void
        {
            depth -= 1;
        }

    private:
        auto PrintIdent() -> void
        {
            for (int i = 0; i < depth; ++i) {
                fmt::print(stderr, "  ");
            }
        }

        int depth = 0;
    };
} // namespace glsld
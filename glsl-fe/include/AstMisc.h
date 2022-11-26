#pragma once
#include "SyntaxToken.h"

#include <vector>

namespace glsld
{
    class AstExpr;

    // Identifier of a declared/used symbol
    class AstDeclId : public SyntaxNode
    {
    public:
        AstDeclId(LexString id) : id(id)
        {
        }

        auto GetIdentifier() -> LexString
        {
            return id;
        }

    private:
        // The actual memory is hosted in the LexContext
        LexString id;
    };

    // array sizes
    class AstArraySpec : public SyntaxNode
    {
    public:
        AstArraySpec()
        {
        }
        AstArraySpec(std::vector<AstExpr*> sizes) : sizes(std::move(sizes))
        {
        }

    private:
        std::vector<AstExpr*> sizes;
    };
} // namespace glsld
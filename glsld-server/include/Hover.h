#pragma once
#include "LanguageQueryProvider.h"
#include "Protocol.h"

namespace glsld
{
    struct HoverContent
    {
        SymbolAccessType type;
        std::string name;
        std::string description;
        std::string documentation;
        std::string code;
        TextRange range;

        // We can't resolve the AST node that declares the symbol.
        bool unknown = false;
    };

    auto ComputeHoverContent(const LanguageQueryProvider& provider, TextPosition position)
        -> std::optional<HoverContent>;

    auto ComputeHover(const LanguageQueryProvider& provider, lsp::Position position) -> std::optional<lsp::Hover>;
} // namespace glsld
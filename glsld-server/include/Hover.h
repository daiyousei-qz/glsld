#pragma once
#include "LanguageQueryProvider.h"
#include "Protocol.h"
#include "SymbolQuery.h"

namespace glsld
{
    struct HoverContent
    {
        // The type of the hovered symbol.
        SymbolDeclType type;

        // The name of the hovered symbol.
        std::string name;

        // The type/value information of the hovered symbol.
        std::string hoverInfo;

        // The description of the hovered symbol, typically parsed from nearby comments or documentation.
        std::string description;

        // The built-in documentation of the hovered symbol.
        std::string documentation;

        // The reconstructed source code of the hovered symbol.
        std::string code;

        // The spelled range of the hovered symbol in the source code.
        TextRange range;

        // We can't resolve the AST node that declares the symbol.
        bool unknown = false;
    };

    auto ComputeHoverContent(const LanguageQueryProvider& provider, TextPosition position)
        -> std::optional<HoverContent>;

    auto ComputeHover(const LanguageQueryProvider& provider, lsp::Position position) -> std::optional<lsp::Hover>;
} // namespace glsld
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

        // The function return type if the hovered symbol is a function.
        std::string returnType;

        // The function parameters if the hovered symbol is a function.
        std::vector<std::string> parameters;

        // The type of the expression if the hovered symbol is part of an expression.
        std::string exprType;

        // The (primitive) value of the expression if the hovered symbol is part of an expression.
        std::string exprValue;

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

    auto QueryHoverContent(const LanguageQueryProvider& provider, TextPosition position) -> std::optional<HoverContent>;

    namespace lsp
    {
        auto ComputeHover(const LanguageQueryProvider& provider, lsp::Position position) -> std::optional<Hover>;
    }
} // namespace glsld
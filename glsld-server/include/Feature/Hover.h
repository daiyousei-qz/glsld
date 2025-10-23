#pragma once
#include "Server/Config.h"
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

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

    auto ComputeHoverText(const HoverContent& hover) -> std::string;

    auto GetHoverOptions(const HoverConfig& config) -> std::optional<lsp::HoverOptions>;

    auto HandleHover(const HoverConfig& config, const LanguageQueryInfo& info, const lsp::HoverParams& params)
        -> std::optional<lsp::Hover>;
} // namespace glsld
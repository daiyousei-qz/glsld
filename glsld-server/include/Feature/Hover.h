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

        // The type of the expression if the hovered symbol is part of an expression.
        // Otherwise, the type of the declared symbol if the hovered symbol is a declarator.
        // Notably, for functions, this is the return type.
        std::string exprType;

        // The function parameters if the hovered symbol is a function.
        std::vector<std::string> parameters;

        // The (primitive) value of the expression if the hovered symbol is part of an expression.
        std::string exprValue;

        // The description of the hovered symbol, typically parsed from nearby comments or documentation.
        std::string description;

        // The reconstructed source code of the hovered symbol.
        std::string code;

        // The spelled range of the hovered symbol in the source code.
        TextRange range;

        // We can't resolve the AST node that declares the symbol.
        bool unknown = false;

        // Whether the symbol is defined in the system preamble
        bool builtin = false;
    };

    auto ComputeHoverText(const HoverContent& hover) -> std::string;

    auto GetHoverOptions(const HoverConfig& config) -> std::optional<lsp::HoverOptions>;

    auto HandleHover(const HoverConfig& config, const LanguageQueryInfo& info, const lsp::HoverParams& params)
        -> std::optional<lsp::Hover>;
} // namespace glsld
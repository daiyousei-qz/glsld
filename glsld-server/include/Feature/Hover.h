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

        // The type of the symbol if applicable:
        // - If the symbol is an expression, this is the type of the expression.
        // - If the symbol is a declarator, this is the type of the declared symbol.
        // - If the symbol is a function name, this is the return type.
        std::string symbolType;

        // The function parameters if the hovered symbol is a function.
        std::vector<std::string> parameters;

        // The (primitive) constant value of the symbol if applicable.
        std::string symbolValue;

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
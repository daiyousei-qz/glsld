#pragma once

#include "Ast/Base.h"
#include "Compiler/SyntaxToken.h"
#include "LanguageQueryProvider.h"

namespace glsld
{
    enum class SymbolDeclType
    {
        // The symbol is a header name. e.g. `"header.h"` in `#include "header.h"`
        HeaderName,

        // The symbol is a macro name. e.g. `MACRO` in `#define MACRO 1`
        Macro,

        // The symbol is a key in the layout qualifier. e.g. `location` in `layout(location = 0) in vec4 a;`
        LayoutQualifier,

        // The symbol is a global variable name. either in a variable declaration or accessing of a variable.
        GlobalVariable,

        // The symbol is a local variable name. either in a variable declaration or accessing of a variable.
        LocalVariable,

        // The symbol is a swizzle name. e.g. `xyz` in `vec3 v; v.xyz;`
        Swizzle,

        // The symbol is accessing of a struct member. e.g. `x` in `s.x;`
        StructMember,

        // The symbol is a function parameter name.
        Parameter,

        // The symbol is a function name, either in a function call or a function declaration.
        Function,

        // The symbol is a type name, either in a type declaration or a type specifier.
        Type,

        // The symbol is a block name. e.g. `Block` in `uniform Block { ... }`
        Block,

        // The symbol is a block instance name. e.g. `block` in `uniform Block { ... } block;`
        BlockInstance,

        // The symbol is a block member name. e.g. `member` in `uniform Block { int member; }`
        BlockMember,
    };

    struct SymbolQueryResult
    {
        // The type of the symbol that the token is accessing.
        SymbolDeclType symbolType;

        // Usually, this is the range of the identifier token.
        // But in case we hit a macro expansion, this is the range of the expaned tokens.
        AstSyntaxRange symbolRange;

        // The text of the symbol spelled in the source file.
        std::string spelledText;

        // The range of the spelled text.
        TextRange spelledRange;

        // The absolute path of the header file if the symbol is a header name.
        const PPSymbolOccurrence* ppSymbolOccurrence = nullptr;

        // The smallest AST node that possesses the symbol.
        const AstNode* astSymbolOccurrence = nullptr;

        // The declaration AST node of the symbol, if any.
        DeclView symbolDecl = {};

        // True if the token is from the declaration. e.g. `a` in `int a;`
        bool isDeclaration = false;
    };

    // TODO: Should we support query for non-identifier?
    // Returns the related information if the cursor position hits an identifier that's accessing a symbol.
    auto QuerySymbolByPosition(const LanguageQueryProvider& provider, TextPosition position)
        -> std::optional<SymbolQueryResult>;
} // namespace glsld
#pragma once

#include "Ast/Base.h"
#include "Compiler/SyntaxToken.h"
#include "LanguageQueryProvider.h"

namespace glsld
{
    enum class SymbolDeclType
    {
        // The symbol is an identifier with none of the following categoty.
        Unknown,

        // The symbol is a key in the layout qualifier. e.g. `location` in `layout(location = 0) in vec4 a;`
        LayoutQualifier,

        // The symbol is a global variable name. either in a variable declaration or accessing of a variable.
        GlobalVariable,

        // The symbol is a local variable name. either in a variable declaration or accessing of a variable.
        LocalVariable,

        // The symbol is a swizzle name. e.g. `xyz` in `vec3 v; v.xyz;`
        Swizzle,

        // The symbol is accessing of a struct member. e.g. `x` in `s.x;`
        MemberVariable,

        // The symbol is a function parameter name.
        Parameter,

        // The symbol is a function name, either in a function call or a function declaration.
        Function,

        // The symbol is a type name, either in a type declaration or a type specifier.
        Type,

        // The symbol is a block name. e.g. `Block` in `uniform Block { ... }`
        InterfaceBlock,

        // The symbol is a block instance name. e.g. `block` in `uniform Block { ... } block;`
        InterfaceBlockInstance,
    };

    struct SymbolQueryResult
    {
        // The token that the cursor is on.
        AstSyntaxToken token;

        // The AST node that possesses the token.
        const AstNode* symbolOwner;

        // The declaration of the symbol that the token is referring to.
        DeclView symbolDecl;

        // The type of the symbol that the token is accessing.
        SymbolDeclType symbolType;

        // True if the token is from the declaration. e.g. `a` in `int a;`
        bool isDeclaration = false;
    };

    // TODO: Should we support query for non-identifier?
    // Returns the related information if the cursor position hits an identifier that's accessing a symbol.
    auto QuerySymbolByPosition(const LanguageQueryProvider& provider, TextPosition position)
        -> std::optional<SymbolQueryResult>;
} // namespace glsld
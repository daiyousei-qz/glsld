#pragma once
#include "Basic/Common.h"
#include "Compiler/SyntaxToken.h"
#include "Language/Extension.h"
#include "Language/ShaderTarget.h"

namespace glsld
{
    class PPCallback
    {
    public:
        virtual auto OnVersionDirective(FileID file, TextRange range, GlslVersion version, GlslProfile profile) -> void
        {
        }
        virtual auto OnExtensionDirective(FileID file, TextRange range, ExtensionId extension,
                                          ExtensionBehavior behavior) -> void
        {
        }

        // Called when a `#include "header.h"` directive is encountered
        // - headerName is the name token of the header file
        // - resolvedPath is the resolved absolute path of the header file, or "" if not found
        virtual auto OnIncludeDirective(const PPToken& headerName, StringView resolvedPath) -> void
        {
        }

        // Called when a `#define MACRO XXX` directive is encountered
        // - macroName is the name token of the macro
        // - params is the list of parameters if the macro is a function-like macro
        // - tokens is the list of tokens that make up the macro replacement list
        // - isFunctionLike is true if the macro is a function-like macro
        virtual auto OnDefineDirective(const PPToken& macroName, ArrayView<PPToken> params, ArrayView<PPToken> tokens,
                                       bool isFunctionLike) -> void
        {
        }

        // Called when a `#undef MACRO` directive is encountered
        // - macroName is the name token of the macro
        virtual auto OnUndefDirective(const PPToken& macroName) -> void
        {
        }
        virtual auto OnIfDirective(bool evalToTrue) -> void
        {
        }
        virtual auto OnElifDirective(bool evalToTrue) -> void
        {
        }
        virtual auto OnIfDefDirective(const PPToken& macroName, bool isNDef) -> void
        {
        }
        virtual auto OnElseDirective() -> void
        {
        }
        virtual auto OnEndifDirective() -> void
        {
        }

        virtual auto OnEnterIncludedFile() -> void
        {
        }
        virtual auto OnExitIncludedFile() -> void
        {
        }
        virtual auto OnMacroExpansion(const PPToken& macroNameTok, AstSyntaxRange expandedTokens) -> void
        {
        }
    };
} // namespace glsld
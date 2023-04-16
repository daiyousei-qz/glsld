#pragma once
#include "Common.h"
#include "SyntaxToken.h"

namespace glsld
{
    class PPCallback
    {
    public:
        virtual auto OnIncludeDirective(const PPToken& headerName) -> void
        {
        }
        virtual auto OnDefineDirective(const PPToken& macroName, ArrayView<PPToken> params) -> void
        {
        }
        virtual auto OnUndefDirective(const PPToken& macroName) -> void
        {
        }
        virtual auto OnIfDefDirective(const PPToken& macroName, bool isNDef) -> void
        {
        }

        virtual auto OnEnterIncludedFile() -> void
        {
        }
        virtual auto OnExitIncludedFile() -> void
        {
        }
        virtual auto OnMacroExpansion(const PPToken& macroUse) -> void
        {
        }
    };
} // namespace glsld
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
        PPCallback()          = default;
        virtual ~PPCallback() = default;

        // Called when a `#version XXX` directive is encountered
        virtual auto OnVersionDirective(FileID file, TextRange range, GlslVersion version, GlslProfile profile) -> void
        {
        }

        // Called when a `#extension EXTENSION : behavior` directive is encountered
        virtual auto OnExtensionDirective(FileID file, TextRange range, ExtensionId extension,
                                          ExtensionBehavior behavior) -> void
        {
        }

        // Called when an unknown `#pragma XXX` directive is encountered
        virtual auto OnUnknownPragmaDirective(ArrayView<PPToken> argTokens) -> void
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

        // Called when a `#if XXX` directive is encountered
        virtual auto OnIfDirective(bool evalToTrue) -> void
        {
        }

        // Called when a `#elif XXX` directive is encountered
        virtual auto OnElifDirective(bool evalToTrue) -> void
        {
        }

        // Called when a `#ifdef MACRO` or `#ifndef MACRO` directive is encountered
        virtual auto OnIfDefDirective(const PPToken& macroName, bool isNDef) -> void
        {
        }

        // Called when a `#else` directive is encountered
        virtual auto OnElseDirective() -> void
        {
        }

        // Called when a `#endif` directive is encountered
        virtual auto OnEndifDirective() -> void
        {
        }

        virtual auto OnEnterIncludedFile() -> void
        {
        }
        virtual auto OnExitIncludedFile() -> void
        {
        }

        // Called when a macro is expanded
        virtual auto OnMacroExpansion(const PPToken& macroNameTok, AstSyntaxRange expandedTokens) -> void
        {
        }

        // Called when a defined operator is encountered, e.g. `defined(MACRO)` or `defined MACRO`
        virtual auto OnDefinedOperator(const PPToken& macroNameTok, bool isDefined) -> void
        {
        }
    };

    class CombinedPPCallback final : public PPCallback
    {
    private:
        PPCallback* first;
        PPCallback* second;

    public:
        CombinedPPCallback(PPCallback* first, PPCallback* second) : first(first), second(second)
        {
        }

        virtual auto OnVersionDirective(FileID file, TextRange range, GlslVersion version, GlslProfile profile)
            -> void override
        {
            first->OnVersionDirective(file, range, version, profile);
            second->OnVersionDirective(file, range, version, profile);
        }

        virtual auto OnExtensionDirective(FileID file, TextRange range, ExtensionId extension,
                                          ExtensionBehavior behavior) -> void override
        {
            first->OnExtensionDirective(file, range, extension, behavior);
            second->OnExtensionDirective(file, range, extension, behavior);
        }

        virtual auto OnUnknownPragmaDirective(ArrayView<PPToken> argTokens) -> void override
        {
            first->OnUnknownPragmaDirective(argTokens);
            second->OnUnknownPragmaDirective(argTokens);
        }

        virtual auto OnIncludeDirective(const PPToken& headerName, StringView resolvedPath) -> void override
        {
            first->OnIncludeDirective(headerName, resolvedPath);
            second->OnIncludeDirective(headerName, resolvedPath);
        }

        virtual auto OnDefineDirective(const PPToken& macroName, ArrayView<PPToken> params, ArrayView<PPToken> tokens,
                                       bool isFunctionLike) -> void override
        {
            first->OnDefineDirective(macroName, params, tokens, isFunctionLike);
            second->OnDefineDirective(macroName, params, tokens, isFunctionLike);
        }

        virtual auto OnUndefDirective(const PPToken& macroName) -> void override
        {
            first->OnUndefDirective(macroName);
            second->OnUndefDirective(macroName);
        }

        virtual auto OnIfDirective(bool evalToTrue) -> void override
        {
            first->OnIfDirective(evalToTrue);
            second->OnIfDirective(evalToTrue);
        }

        virtual auto OnElifDirective(bool evalToTrue) -> void override
        {
            first->OnElifDirective(evalToTrue);
            second->OnElifDirective(evalToTrue);
        }

        virtual auto OnIfDefDirective(const PPToken& macroName, bool isNDef) -> void override
        {
            first->OnIfDefDirective(macroName, isNDef);
            second->OnIfDefDirective(macroName, isNDef);
        }

        virtual auto OnElseDirective() -> void override
        {
            first->OnElseDirective();
            second->OnElseDirective();
        }

        virtual auto OnEndifDirective() -> void override
        {
            first->OnEndifDirective();
            second->OnEndifDirective();
        }

        virtual auto OnEnterIncludedFile() -> void override
        {
            first->OnEnterIncludedFile();
            second->OnEnterIncludedFile();
        }
        virtual auto OnExitIncludedFile() -> void override
        {
            first->OnExitIncludedFile();
            second->OnExitIncludedFile();
        }

        virtual auto OnMacroExpansion(const PPToken& macroNameTok, AstSyntaxRange expandedTokens) -> void override
        {
            first->OnMacroExpansion(macroNameTok, expandedTokens);
            second->OnMacroExpansion(macroNameTok, expandedTokens);
        }

        virtual auto OnDefinedOperator(const PPToken& macroNameTok, bool isDefined) -> void override
        {
            first->OnDefinedOperator(macroNameTok, isDefined);
            second->OnDefinedOperator(macroNameTok, isDefined);
        }
    };
} // namespace glsld
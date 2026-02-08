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

        // Called when a bad preprocessor directive is encountered
        virtual auto OnBadDirective(ArrayView<PPToken> tokens) -> void
        {
        }

        // Called when a `#version XXX` directive is encountered
        virtual auto OnVersionDirective(ArrayView<PPToken> tokens, GlslVersion version, GlslProfile profile) -> void
        {
        }

        // Called when a `#extension EXTENSION : behavior` directive is encountered
        virtual auto OnExtensionDirective(ArrayView<PPToken> tokens, ExtensionId extension, ExtensionBehavior behavior)
            -> void
        {
        }

        // Called when an unknown `#pragma XXX` directive is encountered
        virtual auto OnUnknownPragmaDirective(ArrayView<PPToken> tokens) -> void
        {
        }

        // Called when a `#include "header.h"` directive is encountered
        // - headerName is the name token of the header file, including quotes or angle brackets
        // - resolvedPath is the resolved absolute path of the header file, or "" if not found
        virtual auto OnIncludeDirective(ArrayView<PPToken> tokens, const PPToken& headerName, StringView resolvedPath)
            -> void
        {
        }

        // Called when a `#define MACRO XXX` directive is encountered
        // - macroName is the name token of the macro
        // - paramTokens is the list of parameters if the macro is a function-like macro
        // - replacementTokens is the list of tokens that make up the macro replacement list
        // - isFunctionLike is true if the macro is a function-like macro
        virtual auto OnDefineDirective(ArrayView<PPToken> tokens, const PPToken& macroName,
                                       ArrayView<PPToken> paramTokens, ArrayView<PPToken> replacementTokens,
                                       bool isFunctionLike) -> void
        {
        }

        // Called when a `#undef MACRO` directive is encountered
        // - macroName is the name token of the macro
        virtual auto OnUndefDirective(ArrayView<PPToken> tokens, const PPToken& macroName) -> void
        {
        }

        // Called when a `#if XXX` directive is encountered
        virtual auto OnIfDirective(ArrayView<PPToken> tokens, bool isActive) -> void
        {
        }

        // Called when a `#elif XXX` directive is encountered
        virtual auto OnElifDirective(ArrayView<PPToken> tokens, bool isActive) -> void
        {
        }

        // Called when a `#ifdef MACRO` or `#ifndef MACRO` directive is encountered
        virtual auto OnIfDefDirective(ArrayView<PPToken> tokens, const PPToken& macroName, bool isNDef, bool isActive)
            -> void
        {
        }

        // Called when a `#else` directive is encountered
        virtual auto OnElseDirective(ArrayView<PPToken> tokens, bool isActive) -> void
        {
        }

        // Called when a `#endif` directive is encountered
        virtual auto OnEndifDirective(ArrayView<PPToken> tokens) -> void
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

        virtual auto OnBadDirective(ArrayView<PPToken> tokens) -> void override
        {
            first->OnBadDirective(tokens);
            second->OnBadDirective(tokens);
        }

        virtual auto OnVersionDirective(ArrayView<PPToken> tokens, GlslVersion version, GlslProfile profile)
            -> void override
        {
            first->OnVersionDirective(tokens, version, profile);
            second->OnVersionDirective(tokens, version, profile);
        }

        virtual auto OnExtensionDirective(ArrayView<PPToken> tokens, ExtensionId extension, ExtensionBehavior behavior)
            -> void override
        {
            first->OnExtensionDirective(tokens, extension, behavior);
            second->OnExtensionDirective(tokens, extension, behavior);
        }

        virtual auto OnUnknownPragmaDirective(ArrayView<PPToken> tokens) -> void override
        {
            first->OnUnknownPragmaDirective(tokens);
            second->OnUnknownPragmaDirective(tokens);
        }

        virtual auto OnIncludeDirective(ArrayView<PPToken> tokens, const PPToken& headerName, StringView resolvedPath)
            -> void override
        {
            first->OnIncludeDirective(tokens, headerName, resolvedPath);
            second->OnIncludeDirective(tokens, headerName, resolvedPath);
        }

        virtual auto OnDefineDirective(ArrayView<PPToken> tokens, const PPToken& macroName,
                                       ArrayView<PPToken> paramTokens, ArrayView<PPToken> replacementTokens,
                                       bool isFunctionLike) -> void override
        {
            first->OnDefineDirective(tokens, macroName, paramTokens, replacementTokens, isFunctionLike);
            second->OnDefineDirective(tokens, macroName, paramTokens, replacementTokens, isFunctionLike);
        }

        virtual auto OnUndefDirective(ArrayView<PPToken> tokens, const PPToken& macroName) -> void override
        {
            first->OnUndefDirective(tokens, macroName);
            second->OnUndefDirective(tokens, macroName);
        }

        virtual auto OnIfDirective(ArrayView<PPToken> tokens, bool isActive) -> void override
        {
            first->OnIfDirective(tokens, isActive);
            second->OnIfDirective(tokens, isActive);
        }

        virtual auto OnElifDirective(ArrayView<PPToken> tokens, bool isActive) -> void override
        {
            first->OnElifDirective(tokens, isActive);
            second->OnElifDirective(tokens, isActive);
        }

        virtual auto OnIfDefDirective(ArrayView<PPToken> tokens, const PPToken& macroName, bool isNDef, bool isActive)
            -> void override
        {
            first->OnIfDefDirective(tokens, macroName, isNDef, isActive);
            second->OnIfDefDirective(tokens, macroName, isNDef, isActive);
        }

        virtual auto OnElseDirective(ArrayView<PPToken> tokens, bool isActive) -> void override
        {
            first->OnElseDirective(tokens, isActive);
            second->OnElseDirective(tokens, isActive);
        }

        virtual auto OnEndifDirective(ArrayView<PPToken> tokens) -> void override
        {
            first->OnEndifDirective(tokens);
            second->OnEndifDirective(tokens);
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
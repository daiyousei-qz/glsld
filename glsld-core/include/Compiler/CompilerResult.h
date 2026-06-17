#pragma once
#include "Basic/AtomTable.h"
#include "Basic/SourceInfo.h"
#include "Support/StringView.h"
#include "Compiler/AstContext.h"
#include "Compiler/CompilerConfig.h"
#include "Compiler/MacroTable.h"
#include "Compiler/SymbolTable.h"

#include <memory>

namespace glsld
{
    class PrecompiledPreamble
    {
    private:
        LanguageConfig languageConfig;

        std::unique_ptr<const AtomTable> atomTable;
        std::unique_ptr<const MacroTable> macroTable;
        std::unique_ptr<const SymbolTable> symbolTable;
        std::unique_ptr<const AstContext> astContext;

    public:
        PrecompiledPreamble(LanguageConfig languageConfig, std::unique_ptr<const AtomTable> atomTable,
                            std::unique_ptr<const MacroTable> macroTable, std::unique_ptr<SymbolTable> symbolTable,
                            std::unique_ptr<const AstContext> astContext)
            : languageConfig(languageConfig), atomTable(std::move(atomTable)), macroTable(std::move(macroTable)),
              symbolTable(std::move(symbolTable)), astContext(std::move(astContext))
        {
        }

        auto GetLanguageConfig() const noexcept -> const LanguageConfig&
        {
            return languageConfig;
        }

        auto GetAstContext() const noexcept -> const AstContext&
        {
            return *astContext;
        }
        auto GetAtomTable() const noexcept -> const AtomTable&
        {
            return *atomTable;
        }
        auto GetMacroTable() const noexcept -> const MacroTable&
        {
            return *macroTable;
        }
        auto GetSymbolTable() const noexcept -> const SymbolTable&
        {
            return *symbolTable;
        }

        auto GetPreambleTokens() const -> ArrayView<RawSyntaxToken>
        {
            return astContext->GetTokens();
        }

        auto GetPreambleComments() const -> ArrayView<RawCommentToken>
        {
            return astContext->GetComments();
        }

        auto GetPreambleFiles() const -> ArrayView<PreprocessedFile>
        {
            return astContext->GetFiles();
        }

        auto GetPreambleAst() const -> const AstTranslationUnit*
        {
            return astContext->GetAst();
        }
    };

    class CompilerResult
    {
    protected:
        std::shared_ptr<PrecompiledPreamble> preamble = nullptr;

        std::unique_ptr<const AtomTable> atomTable   = nullptr;
        std::unique_ptr<const AstContext> astContext = nullptr;

    public:
        CompilerResult(std::shared_ptr<PrecompiledPreamble> preamble, std::unique_ptr<const AtomTable> atomTable,
                       std::unique_ptr<const AstContext> astContext)
            : preamble(std::move(preamble)), atomTable(std::move(atomTable)), astContext(std::move(astContext))
        {
        }

        auto GetPreamble() const noexcept -> const std::shared_ptr<PrecompiledPreamble>&
        {
            return preamble;
        }

        auto GetPreambleTokens() const -> ArrayView<RawSyntaxToken>
        {
            return preamble ? preamble->GetPreambleTokens() : ArrayView<RawSyntaxToken>{};
        }

        auto GetPreambleComments() const -> ArrayView<RawCommentToken>
        {
            return preamble ? preamble->GetPreambleComments() : ArrayView<RawCommentToken>{};
        }

        auto GetPreambleFiles() const -> ArrayView<PreprocessedFile>
        {
            return preamble ? preamble->GetPreambleFiles() : ArrayView<PreprocessedFile>{};
        }

        auto GetPreambleAst() const -> const AstTranslationUnit*
        {
            return preamble ? preamble->GetPreambleAst() : nullptr;
        }

        auto GetTokens() const -> ArrayView<RawSyntaxToken>
        {
            return astContext ? astContext->GetTokens() : ArrayView<RawSyntaxToken>{};
        }

        auto GetComments() const -> ArrayView<RawCommentToken>
        {
            return astContext ? astContext->GetComments() : ArrayView<RawCommentToken>{};
        }

        auto GetFiles() const -> ArrayView<PreprocessedFile>
        {
            return astContext ? astContext->GetFiles() : ArrayView<PreprocessedFile>{};
        }

        auto GetAst() const -> const AstTranslationUnit*
        {
            return astContext ? astContext->GetAst() : nullptr;
        }
    };

} // namespace glsld

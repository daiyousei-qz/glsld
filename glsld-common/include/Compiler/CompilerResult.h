#pragma once
#include "Ast/Misc.h"
#include "Basic/AtomTable.h"
#include "Basic/Common.h"
#include "Basic/SourceInfo.h"
#include "Basic/StringView.h"
#include "Compiler/AstContext.h"
#include "Compiler/CompilerArtifacts.h"
#include "Compiler/CompilerConfig.h"
#include "Compiler/MacroTable.h"
#include "Compiler/SymbolTable.h"
#include "Compiler/SyntaxToken.h"

#include <memory>

namespace glsld
{
    struct SourceFileInfo
    {
        FileID id;

        // The absolute path of file. Could be empty if the file is not on disk.
        std::string canonicalPath;
    };

    class PrecompiledPreamble
    {
    private:
        LanguageConfig languageConfig;

        std::string systemPreambleText;
        std::string userPreambleText;

        std::unique_ptr<const AtomTable> atomTable;
        std::unique_ptr<const MacroTable> macroTable;
        std::unique_ptr<const SymbolTable> symbolTable;
        std::unique_ptr<const AstContext> astContext;

        std::unique_ptr<const CompilerArtifacts> artifacts;

    public:
        PrecompiledPreamble(LanguageConfig languageConfig, StringView systemPreamble, StringView userPreamble,
                            std::unique_ptr<const AtomTable> atomTable, std::unique_ptr<const MacroTable> macroTable,
                            std::unique_ptr<SymbolTable> symbolTable, std::unique_ptr<const AstContext> astContext,
                            std::unique_ptr<const CompilerArtifacts> artifacts)
            : languageConfig(languageConfig), systemPreambleText(systemPreamble), userPreambleText(userPreamble),
              atomTable(std::move(atomTable)), macroTable(std::move(macroTable)), symbolTable(std::move(symbolTable)),
              astContext(std::move(astContext)), artifacts(std::move(artifacts))
        {
        }

        auto GetLanguageConfig() const noexcept -> const LanguageConfig&
        {
            return languageConfig;
        }

        auto GetSystemPreamble() const noexcept -> StringView
        {
            return systemPreambleText;
        }

        auto GetUserPreamble() const noexcept -> StringView
        {
            return userPreambleText;
        }

        auto GetSystemPreambleAst() const noexcept -> const AstTranslationUnit*
        {
            return artifacts->GetAst(TranslationUnitID::SystemPreamble);
        }

        auto GetUserPreambleAst() const noexcept -> const AstTranslationUnit*
        {
            return artifacts->GetAst(TranslationUnitID::UserPreamble);
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
    };

    class CompilerResult
    {
    protected:
        std::shared_ptr<PrecompiledPreamble> preamble = nullptr;

        std::unique_ptr<const AstContext> astContext = nullptr;
        std::unique_ptr<const CompilerArtifacts> artifacts;

    public:
        CompilerResult(std::unique_ptr<const AstContext> astContext, std::unique_ptr<const CompilerArtifacts> artifacts)
            : astContext(std::move(astContext)), artifacts(std::move(artifacts))
        {
        }

        auto GetSystemPreambleTokens() const noexcept -> ArrayView<RawSyntaxTokenEntry>
        {
            return artifacts->GetSyntaxTokens(TranslationUnitID::SystemPreamble);
        }
        auto GetUserPreambleTokens() const noexcept -> ArrayView<RawSyntaxTokenEntry>
        {
            return artifacts->GetSyntaxTokens(TranslationUnitID::UserPreamble);
        }
        auto GetUserFileTokens() const noexcept -> ArrayView<RawSyntaxTokenEntry>
        {
            return artifacts->GetSyntaxTokens(TranslationUnitID::UserFile);
        }
        auto GetUserFileComments() const noexcept -> ArrayView<RawCommentTokenEntry>
        {
            return artifacts->GetCommentTokens(TranslationUnitID::UserFile);
        }

        auto GetSystemPreambleAst() const noexcept -> const AstTranslationUnit*
        {
            return artifacts->GetAst(TranslationUnitID::SystemPreamble);
        }
        auto GetUserPreambleAst() const noexcept -> const AstTranslationUnit*
        {
            return artifacts->GetAst(TranslationUnitID::UserPreamble);
        }
        auto GetUserFileAst() const noexcept -> const AstTranslationUnit*
        {
            return artifacts->GetAst(TranslationUnitID::UserFile);
        }

        auto GetMainFileID() const noexcept -> FileID
        {
            return GetUserFileTokens().back().spelledFile;
        }
    };

} // namespace glsld
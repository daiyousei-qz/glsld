#pragma once
#include "Basic/AtomTable.h"
#include "Basic/SourceInfo.h"
#include "Support/StringView.h"
#include "Compiler/AstContext.h"
#include "Compiler/CompilerArtifacts.h"
#include "Compiler/CompilerConfig.h"
#include "Compiler/MacroTable.h"
#include "Compiler/SymbolTable.h"

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

        std::unique_ptr<const CompilerArtifact> systemPreambleArtifacts;
        std::unique_ptr<const CompilerArtifact> userPreambleArtifacts;

    public:
        PrecompiledPreamble(LanguageConfig languageConfig, StringView systemPreamble, StringView userPreamble,
                            std::unique_ptr<const AtomTable> atomTable, std::unique_ptr<const MacroTable> macroTable,
                            std::unique_ptr<SymbolTable> symbolTable, std::unique_ptr<const AstContext> astContext,
                            std::unique_ptr<const CompilerArtifact> systemPreambleArtifacts,
                            std::unique_ptr<const CompilerArtifact> userPreambleArtifacts)
            : languageConfig(languageConfig), systemPreambleText(systemPreamble), userPreambleText(userPreamble),
              atomTable(std::move(atomTable)), macroTable(std::move(macroTable)), symbolTable(std::move(symbolTable)),
              astContext(std::move(astContext)), systemPreambleArtifacts(std::move(systemPreambleArtifacts)),
              userPreambleArtifacts(std::move(userPreambleArtifacts))
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

        auto GetSystemPreambleArtifacts() const noexcept -> const CompilerArtifact&
        {
            return *systemPreambleArtifacts;
        }
        auto GetUserPreambleArtifacts() const noexcept -> const CompilerArtifact&
        {
            return *userPreambleArtifacts;
        }
    };

    class CompilerResult
    {
    protected:
        std::shared_ptr<PrecompiledPreamble> preamble = nullptr;

        std::unique_ptr<const AtomTable> atomTable   = nullptr;
        std::unique_ptr<const AstContext> astContext = nullptr;

        std::unique_ptr<const CompilerArtifact> systemPreambleArtifacts = nullptr;
        std::unique_ptr<const CompilerArtifact> userPreambleArtifacts   = nullptr;
        std::unique_ptr<const CompilerArtifact> userFileArtifacts       = nullptr;

    public:
        CompilerResult(std::shared_ptr<PrecompiledPreamble> preamble, std::unique_ptr<const AtomTable> atomTable,
                       std::unique_ptr<const AstContext> astContext,
                       std::unique_ptr<const CompilerArtifact> systemPreambleArtifacts,
                       std::unique_ptr<const CompilerArtifact> userPreambleArtifacts,
                       std::unique_ptr<const CompilerArtifact> userFileArtifacts)
            : preamble(std::move(preamble)), atomTable(std::move(atomTable)), astContext(std::move(astContext)),
              systemPreambleArtifacts(std::move(systemPreambleArtifacts)),
              userPreambleArtifacts(std::move(userPreambleArtifacts)), userFileArtifacts(std::move(userFileArtifacts))
        {
        }

        auto GetPreamble() const noexcept -> const std::shared_ptr<PrecompiledPreamble>&
        {
            return preamble;
        }

        auto GetSystemPreambleArtifacts() const noexcept -> const CompilerArtifact&
        {
            return *systemPreambleArtifacts;
        }
        auto GetUserPreambleArtifacts() const noexcept -> const CompilerArtifact&
        {
            return *userPreambleArtifacts;
        }
        auto GetUserFileArtifacts() const noexcept -> const CompilerArtifact&
        {
            return *userFileArtifacts;
        }
    };

} // namespace glsld
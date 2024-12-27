#include "Compiler/CompilerInvocationState.h"
#include "Basic/AtomTable.h"
#include "Compiler/AstContext.h"
#include "Compiler/CompilerArtifacts.h"
#include "Compiler/MacroTable.h"
#include "Compiler/SymbolTable.h"
#include <memory>

namespace glsld
{
    auto CompilerInvocationState::Initialize() -> void
    {
        if (preamble) {
            atomTable = std::make_unique<AtomTable>();
            atomTable->Import(preamble->GetAtomTable());
            // FIXME: import macros if preamble is present
            macroTable  = std::make_unique<MacroTable>();
            symbolTable = std::make_unique<SymbolTable>(preamble->GetSymbolTable().GetGlobalLevels());
            astContext  = std::make_unique<AstContext>(&preamble->GetAstContext());

            systemPreambleArtifacts = preamble->GetSystemPreambleArtifacts().CreateReference();
            userPreambleArtifacts   = preamble->GetUserPreambleArtifacts().CreateReference();
        }
        else {
            atomTable   = std::make_unique<AtomTable>();
            macroTable  = std::make_unique<MacroTable>();
            symbolTable = std::make_unique<SymbolTable>();
            astContext  = std::make_unique<AstContext>(nullptr);

            systemPreambleArtifacts = std::make_unique<CompilerArtifact>(TranslationUnitID::SystemPreamble);
            userPreambleArtifacts   = std::make_unique<CompilerArtifact>(TranslationUnitID::UserPreamble);
        }

        diagStream        = std::make_unique<DiagnosticStream>();
        userFileArtifacts = std::make_unique<CompilerArtifact>(TranslationUnitID::UserFile);
    }
} // namespace glsld
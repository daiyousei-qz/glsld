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
        }
        else {
            atomTable   = std::make_unique<AtomTable>();
            macroTable  = std::make_unique<MacroTable>();
            symbolTable = std::make_unique<SymbolTable>();
            astContext  = std::make_unique<AstContext>(nullptr);
        }

        diagStream = std::make_unique<DiagnosticStream>();
        artifacts  = std::make_unique<CompilerArtifacts>();
    }
} // namespace glsld
#pragma once
#include "Basic/Common.h"
#include "Basic/MemoryArena.h"
#include "Ast/Base.h"
#include "Compiler/SymbolTable.h"
#include "Compiler/CompilerContextBase.h"

#include <map>
#include <vector>

namespace glsld
{
    // This class manages everything related to parsing of a translation unit, primarily AST.
    class AstContext final : CompilerContextBase<AstContext>
    {
    private:
        const AstTranslationUnit* translationUnit = nullptr;

        // The cached array types.
        std::map<std::pair<const Type*, std::vector<size_t>>, const Type*> arrayTypes;

        // The memory arena that holds all memory allocated for AST.
        std::unique_ptr<MemoryArena> arena;

        // The current symbol table.
        std::unique_ptr<SymbolTable> symbolTable;

    public:
        AstContext(const AstContext* preambleContext);
        ~AstContext() = default;

        auto GetTranslationUnit() const noexcept
        {
            return translationUnit;
        }

        auto SetTranslationUnit(const AstTranslationUnit* tu)
        {
            GLSLD_ASSERT(!translationUnit && "Translation unit is already set.");
            translationUnit = tu;
        }

        auto GetArena() noexcept -> MemoryArena&
        {
            return *arena;
        }

        auto GetSymbolTable() -> SymbolTable&
        {
            return *symbolTable;
        }

        auto CreateStructType(AstStructDecl& decl) -> const Type*;

        auto CreateInterfaceBlockType(AstInterfaceBlockDecl& decl) -> const Type*;

        auto GetArrayType(const Type* elementType, const AstArraySpec* arraySpec) -> const Type*;

        // TODO: To ensure Type of the same type always uses a single pointer, we need a common context for
        // different module compile?
        auto GetArrayType(const Type* elementType, ArrayView<size_t> dimSizes) -> const Type*;
    };
} // namespace glsld
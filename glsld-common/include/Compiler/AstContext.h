#pragma once
#include "Ast/Misc.h"
#include "Basic/Common.h"
#include "Basic/MemoryArena.h"
#include "Ast/Base.h"

#include <map>
#include <vector>

namespace glsld
{
    // This class manages everything related to parsing of a translation unit, primarily AST.
    class AstContext final
    {
    private:
        const AstTranslationUnit* systemPreambleAst = nullptr;

        const AstTranslationUnit* userPreambleAst = nullptr;

        const AstTranslationUnit* mainFileAst = nullptr;

        // The cached array types.
        std::map<std::pair<const Type*, std::vector<size_t>>, const Type*> arrayTypes;

        // The memory arena that holds all memory allocated for AST.
        MemoryArena arena;

    public:
        AstContext(const AstContext* preambleContext);

        auto GetTranslationUnit() const noexcept
        {
            return mainFileAst;
        }

        auto SetTranslationUnit(const AstTranslationUnit* tu)
        {
            GLSLD_ASSERT(!mainFileAst && "Translation unit is already set.");
            mainFileAst = tu;
        }

        auto GetArena() noexcept -> MemoryArena&
        {
            return arena;
        }

        auto CreateStructType(AstStructDecl& decl) -> const Type*;

        auto CreateInterfaceBlockType(AstInterfaceBlockDecl& decl) -> const Type*;

        auto GetArrayType(const Type* elementType, const AstArraySpec* arraySpec) -> const Type*;

        // TODO: To ensure Type of the same type always uses a single pointer, we need a common context for
        // different module compile?
        auto GetArrayType(const Type* elementType, ArrayView<size_t> dimSizes) -> const Type*;
    };
} // namespace glsld
#pragma once
#include "Basic/Common.h"
#include "Ast/Base.h"
#include "Ast/Dispatch.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/SymbolTable.h"
#include "Compiler/CompilerContextBase.h"

#include <map>
#include <vector>

namespace glsld
{
    // This class manages everything related to parsing of a translation unit.
    class AstContext final : CompilerContextBase<AstContext>
    {
    private:
        // FIXME: optimize memory layout
        std::vector<const AstNode*> nodes;

        const AstTranslationUnit* translationUnit = nullptr;

        // FIXME: optimize memory layout
        std::vector<const Type*> compositeTypes;

        std::map<std::pair<const Type*, std::vector<size_t>>, const Type*> arrayTypes;

        std::unique_ptr<SymbolTable> symbolTable;

    public:
        AstContext(const AstContext* preambleContext);
        ~AstContext();

        auto SetTranslationUnit(const AstTranslationUnit* tu)
        {
            GLSLD_ASSERT(!translationUnit && "Translation unit is already set.");
            translationUnit = tu;
        }
        auto GetTranslationUnit() const noexcept
        {
            return translationUnit;
        }

        auto GetSymbolTable() -> SymbolTable&
        {
            return *symbolTable;
        }

        template <typename T, typename... Args>
            requires AstNodeTrait<T>::isLeafNode
        auto CreateAstNode(AstSyntaxRange range, Args&&... args) -> T*
        {
            auto result = new T(std::forward<Args>(args)...);
            result->Initialize(AstNodeTrait<T>::tag, range);
            nodes.push_back(result);
            return result;
        }

        auto CreateStructType(AstStructDecl& decl) -> const Type*;

        auto CreateInterfaceBlockType(AstInterfaceBlockDecl& decl) -> const Type*;

        auto GetArrayType(const Type* elementType, const AstArraySpec* arraySpec) -> const Type*;

        // TODO: To ensure Type of the same type always uses a single pointer, we need a common context for
        // different module compile?
        auto GetArrayType(const Type* elementType, ArrayView<size_t> dimSizes) -> const Type*;
    };
} // namespace glsld
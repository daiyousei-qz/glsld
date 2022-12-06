#pragma once
#include "Common.h"
#include "Ast.h"
#include "Typing.h"
#include <map>
#include <type_traits>

namespace glsld
{
    // This class holds all AST nodes in a translation unit
    class AstContext
    {
    public:
        friend class Parser;

        ~AstContext()
        {
            for (auto node : nodes) {
                node->DispatchInvoke([](auto& dispatchedNode) { delete &dispatchedNode; });
            }

            for (const auto& [key, typeDesc] : arrayTypeCache) {
                delete typeDesc;
            }
        }

        auto AddGlobalDecl(AstDecl* decl) -> void
        {
            globalDecls.push_back(decl);
            if (auto p = decl->As<AstFunctionDecl>()) {
                functionDecls.push_back(p);
            }
            if (auto p = decl->As<AstVariableDecl>()) {
                variableDecls.push_back(p);
            }
        }

        template <typename T, typename... Args>
            requires AstNodeTrait<T>::isLeafNode
        auto CreateAstNode(SyntaxTokenRange range, Args&&... args) -> T*
        {
            auto result = new T(std::forward<Args>(args)...);
            result->Initialize(AstNodeTrait<T>::tag, range);
            return result;
        }

        auto CreateStructType(AstStructDecl* decl) -> const TypeDesc*
        {
            return GetErrorTypeDesc();
        }

        auto CreateStructType(AstInterfaceBlockDecl* decl) -> const TypeDesc*
        {
            return GetErrorTypeDesc();
        }

        auto GetFunctionType(ArrayView<const TypeDesc*> params) -> const TypeDesc*
        {
            return GetErrorTypeDesc();
        }

        // TODO: To ensure TypeDesc of the same type always uses a single pointer, we need a common context for
        // different module compile?
        auto GetArrayType(const TypeDesc* elementType, const std::vector<size_t>& dimSizes) -> const TypeDesc*
        {
            const TypeDesc* realElementType = elementType;
            std::vector<size_t> realDimSizes;

            // Fold array type if needed
            if (auto arrayTypeDesc = elementType->GetArrayDesc()) {
                realElementType = arrayTypeDesc->elementType;
                realDimSizes    = arrayTypeDesc->dimSizes;
                std::ranges::copy(dimSizes, std::back_inserter(realDimSizes));
            }
            else {
                realDimSizes = dimSizes;
            }

            GLSLD_ASSERT(!realElementType->IsArray());

            // Find and cache the type desc if needed
            auto cachedItem = arrayTypeCache[std::pair{elementType, dimSizes}];
            if (cachedItem == nullptr) {
                cachedItem = new TypeDesc{"unknown[]", ArrayTypeDesc{.elementType = elementType, .dimSizes = dimSizes}};
            }
            return cachedItem;
        }

        std::map<std::pair<const TypeDesc*, std::vector<size_t>>, const TypeDesc*> arrayTypeCache;

        std::vector<AstDecl*> globalDecls;
        std::vector<AstVariableDecl*> variableDecls;
        std::vector<AstFunctionDecl*> functionDecls;

        // FIXME: optimize memory layout
        std::vector<AstNodeBase*> nodes;
    };
} // namespace glsld
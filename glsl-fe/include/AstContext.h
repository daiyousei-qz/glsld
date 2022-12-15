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
            for (const auto& typeDesc : structTypeCache) {
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

        auto CreateStructType(AstStructDecl& decl) -> const TypeDesc*
        {
            std::vector<std::pair<std::string, const TypeDesc*>> memberDesc;
            for (auto memberDecl : decl.GetMembers()) {
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    auto typeDesc = GetArrayType(memberDecl->GetType()->GetResolvedType(), declarator.arraySize);
                    memberDesc.push_back({declarator.declTok.text.Str(), typeDesc});
                }
            }

            auto typeName = decl.GetDeclToken() ? decl.GetDeclToken()->text.StrView() : "";
            structTypeCache.push_back(
                new TypeDesc(fmt::format("struct#{}", typeName), StructTypeDesc{
                                                                     .decl    = &decl,
                                                                     .name    = std::string{typeName},
                                                                     .members = std::move(memberDesc),
                                                                 }));
            return structTypeCache.back();
        }

        auto CreateStructType(AstInterfaceBlockDecl& decl) -> const TypeDesc*
        {
            std::vector<std::pair<std::string, const TypeDesc*>> memberDesc;
            for (auto memberDecl : decl.GetMembers()) {
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    auto typeDesc = GetArrayType(memberDecl->GetType()->GetResolvedType(), declarator.arraySize);
                    memberDesc.push_back({declarator.declTok.text.Str(), typeDesc});
                }
            }

            auto typeName = decl.GetDeclToken().text.StrView();
            structTypeCache.push_back(
                new TypeDesc(fmt::format("block#{}", typeName), StructTypeDesc{
                                                                    .decl    = &decl,
                                                                    .name    = std::string{typeName},
                                                                    .members = std::move(memberDesc),
                                                                }));
            return structTypeCache.back();
        }

        auto GetFunctionType(ArrayView<const TypeDesc*> params) -> const TypeDesc*
        {
            return GetErrorTypeDesc();
        }

        auto GetArrayType(const TypeDesc* elementType, AstArraySpec* arraySpec) -> const TypeDesc*
        {
            std::vector<size_t> dimSizes;
            if (arraySpec != nullptr && !arraySpec->GetSizeList().empty()) {
                std::vector<size_t> dimSizes;
                for (auto arrayDim : arraySpec->GetSizeList()) {
                    if (arrayDim != nullptr) {
                        const auto& dimSizeValue = arrayDim->GetConstValue();
                        if (dimSizeValue.HasIntValue()) {
                            dimSizes.push_back(dimSizeValue.GetIntValue());
                            continue;
                        }
                    }

                    dimSizes.push_back(0);
                }

                if (!dimSizes.empty()) {
                    return GetArrayType(elementType, dimSizes);
                }
            }

            return elementType;
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

            if (realDimSizes.empty()) {
                return elementType;
            }

            // Find and cache the type desc if needed
            auto cachedItem = arrayTypeCache[std::pair{realElementType, realDimSizes}];
            if (cachedItem == nullptr) {
                std::string debugName = realElementType->GetDebugName();
                for (auto dimSize : realDimSizes) {
                    if (dimSize != 0) {
                        debugName += fmt ::format("[{}]", dimSize);
                    }
                    else {
                        debugName += "[]";
                    }
                }
                cachedItem = new TypeDesc{std::move(debugName),
                                          ArrayTypeDesc{.elementType = realElementType, .dimSizes = realDimSizes}};
            }
            return cachedItem;
        }

        std::vector<const TypeDesc*> structTypeCache;
        std::map<std::pair<const TypeDesc*, std::vector<size_t>>, const TypeDesc*> arrayTypeCache;

        std::vector<AstDecl*> globalDecls;
        std::vector<AstVariableDecl*> variableDecls;
        std::vector<AstFunctionDecl*> functionDecls;

        // FIXME: optimize memory layout
        std::vector<AstNodeBase*> nodes;
    };
} // namespace glsld
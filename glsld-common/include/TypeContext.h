#pragma once
#include "Ast.h"
#include "Typing.h"

#include <vector>
#include <map>

namespace glsld
{
    class TypeContext
    {
    public:
        TypeContext() = default;
        ~TypeContext()
        {
            for (const auto& [key, typeDesc] : arrayTypes) {
                delete typeDesc;
            }
            for (const auto& typeDesc : structTypes) {
                delete typeDesc;
            }
        }

        auto CreateStructType(AstStructDecl& decl) -> const Type*
        {
            std::vector<std::pair<std::string, const Type*>> memberDesc;
            for (auto memberDecl : decl.GetMembers()) {
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    auto typeDesc = GetArrayType(memberDecl->GetType()->GetResolvedType(), declarator.arraySize);
                    memberDesc.push_back({declarator.declTok.text.Str(), typeDesc});
                }
            }

            StringView typeName = "<unnamed-struct>";
            if (decl.GetDeclToken()) {
                typeName = decl.GetDeclToken()->text.StrView();
            }
            structTypes.push_back(new Type(typeName.Str(), StructTypeDesc{
                                                               .decl    = &decl,
                                                               .name    = decl.GetDeclToken() ? typeName.Str() : "",
                                                               .members = std::move(memberDesc),
                                                           }));
            return structTypes.back();
        }

        auto CreateStructType(AstInterfaceBlockDecl& decl) -> const Type*
        {
            std::vector<std::pair<std::string, const Type*>> memberDesc;
            for (auto memberDecl : decl.GetMembers()) {
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    auto typeDesc = GetArrayType(memberDecl->GetType()->GetResolvedType(), declarator.arraySize);
                    memberDesc.push_back({declarator.declTok.text.Str(), typeDesc});
                }
            }

            auto typeName = decl.GetDeclToken().text.StrView();
            structTypes.push_back(new Type(typeName.Str(), StructTypeDesc{
                                                               .decl    = &decl,
                                                               .name    = typeName.Str(),
                                                               .members = std::move(memberDesc),
                                                           }));
            return structTypes.back();
        }

        auto GetArrayType(const Type* elementType, AstArraySpec* arraySpec) -> const Type*
        {
            std::vector<size_t> dimSizes;
            if (arraySpec != nullptr && !arraySpec->GetSizeList().empty()) {
                std::vector<size_t> dimSizes;
                for (auto arrayDim : arraySpec->GetSizeList()) {
                    if (arrayDim != nullptr) {
                        const auto& dimSizeValue = arrayDim->GetConstValue();
                        if (dimSizeValue.IsScalarInt32()) {
                            dimSizes.push_back(dimSizeValue.GetInt32Value());
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

        // TODO: To ensure Type of the same type always uses a single pointer, we need a common context for
        // different module compile?
        auto GetArrayType(const Type* elementType, const std::vector<size_t>& dimSizes) -> const Type*
        {
            const Type* realElementType = elementType;
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
            auto cachedItem = arrayTypes[std::pair{realElementType, realDimSizes}];
            if (cachedItem == nullptr) {
                std::string debugName = realElementType->GetDebugName().Str();
                for (auto dimSize : realDimSizes) {
                    if (dimSize != 0) {
                        debugName += fmt::format("[{}]", dimSize);
                    }
                    else {
                        debugName += "[]";
                    }
                }
                cachedItem = new Type{std::move(debugName),
                                      ArrayTypeDesc{.elementType = realElementType, .dimSizes = realDimSizes}};
            }
            return cachedItem;
        }

    private:
        std::vector<const Type*> structTypes;
        std::map<std::pair<const Type*, std::vector<size_t>>, const Type*> arrayTypes;
    };
} // namespace glsld
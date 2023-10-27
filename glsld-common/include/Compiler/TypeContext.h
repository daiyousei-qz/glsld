#pragma once
#include "Basic/Common.h"
#include "Language/Typing.h"
#include "Ast/Decl.h"
#include "Ast/Misc.h"
#include "Compiler/AstEval.h"

#include <vector>
#include <map>

namespace glsld
{
    struct StructDeclRecord
    {
        const AstDecl* structDecl;

        std::unordered_map<std::string, DeclView> memberLookup;
    };

    class TypeContext
    {
    private:
        std::vector<const Type*> structTypes;

        std::unordered_map<const Type*, StructDeclRecord> structDeclLookup;

        std::map<std::pair<const Type*, std::vector<size_t>>, const Type*> arrayTypes;

    public:
        TypeContext() = default;
        ~TypeContext()
        {
            for (const auto& [key, type] : arrayTypes) {
                delete type;
            }
            for (const auto& type : structTypes) {
                delete type;
            }
        }

        auto FindStructTypeDecl(const Type* type) const -> const StructDeclRecord*
        {
            if (auto it = structDeclLookup.find(type); it != structDeclLookup.end()) {
                return &it->second;
            }

            return nullptr;
        }

        auto CreateStructType(AstStructDecl& decl) -> const Type*
        {
            std::vector<std::pair<std::string, const Type*>> memberDesc;
            for (auto memberDecl : decl.GetMembers()) {
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    auto typeDesc = GetArrayType(memberDecl->GetQualType()->GetResolvedType(), declarator.arraySize);
                    memberDesc.push_back({declarator.declTok.text.Str(), typeDesc});
                }
            }

            StringView typeName = "__UnnamedStructType";
            if (decl.GetDeclTok() && decl.GetDeclTok()->IsIdentifier()) {
                typeName = decl.GetDeclTok()->text.StrView();
            }
            structTypes.push_back(new Type(typeName.Str(), StructTypeDesc{
                                                               .name    = decl.GetDeclTok() ? typeName.Str() : "",
                                                               .members = std::move(memberDesc),
                                                           }));

            std::unordered_map<std::string, DeclView> memberLookup;
            for (auto memberDecl : decl.GetMembers()) {
                size_t declIndex = 0;
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    if (declarator.declTok.IsIdentifier()) {
                        memberLookup[declarator.declTok.text.Str()] = DeclView{memberDecl, declIndex};
                    }
                    declIndex += 1;
                }
            }
            structDeclLookup[structTypes.back()] =
                StructDeclRecord{.structDecl = &decl, .memberLookup = std::move(memberLookup)};

            return structTypes.back();
        }

        auto CreateInterfaceBlockType(AstInterfaceBlockDecl& decl) -> const Type*
        {
            std::vector<std::pair<std::string, const Type*>> memberDesc;
            for (auto memberDecl : decl.GetMembers()) {
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    auto typeDesc = GetArrayType(memberDecl->GetQualType()->GetResolvedType(), declarator.arraySize);
                    memberDesc.push_back({declarator.declTok.text.Str(), typeDesc});
                }
            }

            StringView typeName = "__UnnamedBlockType";
            if (decl.GetDeclTok().IsIdentifier()) {
                typeName = decl.GetDeclTok().text.StrView();
            }
            structTypes.push_back(new Type(typeName.Str(), StructTypeDesc{
                                                               .name    = typeName.Str(),
                                                               .members = std::move(memberDesc),
                                                           }));
            std::unordered_map<std::string, DeclView> memberLookup;
            for (auto memberDecl : decl.GetMembers()) {
                size_t declIndex = 0;
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    if (declarator.declTok.IsIdentifier()) {
                        memberLookup[declarator.declTok.text.Str()] = DeclView{memberDecl, declIndex};
                    }
                    declIndex += 1;
                }
            }
            structDeclLookup[structTypes.back()] =
                StructDeclRecord{.structDecl = &decl, .memberLookup = std::move(memberLookup)};

            return structTypes.back();
        }

        auto GetArrayType(const Type* elementType, const AstArraySpec* arraySpec) -> const Type*
        {
            if (elementType->IsError()) {
                // We cannot construct arrays upon error type
                return elementType;
            }

            std::vector<size_t> dimSizes;
            if (arraySpec != nullptr && !arraySpec->GetSizeList().empty()) {
                std::vector<size_t> dimSizes;
                for (auto arrayDim : arraySpec->GetSizeList()) {
                    if (arrayDim != nullptr) {
                        const auto& dimSizeValue = EvaluateAstExpr(arrayDim);
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
            if (elementType->IsError()) {
                // We cannot construct arrays upon error type
                return elementType;
            }

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
    };
} // namespace glsld
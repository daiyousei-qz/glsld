#include "Compiler/AstContext.h"
#include "Compiler/AstEval.h"

namespace glsld
{
    auto AstContext::CreateStructType(AstStructDecl& decl) -> const Type*
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
        compositeTypes.push_back(new Type(typeName.Str(), StructTypeDesc{
                                                              .name    = decl.GetDeclTok() ? typeName.Str() : "",
                                                              .members = std::move(memberDesc),
                                                              .decl    = &decl,
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
        structDeclLookup[compositeTypes.back()] =
            StructDeclRecord{.structDecl = &decl, .memberLookup = std::move(memberLookup)};

        return compositeTypes.back();
    }

    auto AstContext::CreateInterfaceBlockType(AstInterfaceBlockDecl& decl) -> const Type*
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
        compositeTypes.push_back(new Type(typeName.Str(), StructTypeDesc{
                                                              .name    = typeName.Str(),
                                                              .members = std::move(memberDesc),
                                                              .decl    = &decl,
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
        structDeclLookup[compositeTypes.back()] =
            StructDeclRecord{.structDecl = &decl, .memberLookup = std::move(memberLookup)};

        return compositeTypes.back();
    }

    auto AstContext::GetArrayType(const Type* elementType, const AstArraySpec* arraySpec) -> const Type*
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

    auto AstContext::GetArrayType(const Type* elementType, ArrayView<size_t> dimSizes) -> const Type*
    {
        if (elementType->IsError()) {
            // We cannot construct arrays upon error type
            return elementType;
        }
        if (dimSizes.empty()) {
            return elementType;
        }

        const Type* realElementType = elementType;
        std::vector<size_t> realDimSizes;

        // Fold array type if needed
        if (auto arrayTypeDesc = elementType->GetArrayDesc()) {
            realElementType = arrayTypeDesc->elementType;
            realDimSizes    = arrayTypeDesc->dimSizes;
        }
        realDimSizes.insert(realDimSizes.end(), dimSizes.begin(), dimSizes.end());

        GLSLD_ASSERT(!realElementType->IsArray());

        // Find and cache the type desc if needed
        auto& cachedItem = arrayTypes[std::pair{realElementType, realDimSizes}];
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
            compositeTypes.push_back(new Type{std::move(debugName),
                                              ArrayTypeDesc{.elementType = realElementType, .dimSizes = realDimSizes}});
            cachedItem = compositeTypes.back();
        }
        return cachedItem;
    }
}; // namespace glsld
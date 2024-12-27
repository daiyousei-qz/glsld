#include "Compiler/AstContext.h"
#include "Ast/Eval.h"

namespace glsld
{
    AstContext::AstContext(const AstContext* preambleContext)
    {
        if (preambleContext) {
            arrayTypes = preambleContext->arrayTypes;
        }
    }

    auto AstContext::CreateStructType(AstStructDecl& decl) -> const Type*
    {
        std::vector<std::pair<std::string, const Type*>> memberDesc;
        for (auto memberDecl : decl.GetMembers()) {
            for (const auto& declarator : memberDecl->GetDeclarators()) {
                auto typeDesc = GetArrayType(memberDecl->GetQualType()->GetResolvedType(), declarator.arraySize);
                memberDesc.push_back({declarator.nameToken.text.Str(), typeDesc});
            }
        }

        StringView typeName = "__UnnamedStructType";
        if (auto nameToken = decl.GetNameToken(); nameToken && nameToken->IsIdentifier()) {
            typeName = decl.GetNameToken()->text.StrView();
        }

        UnorderedStringMap<DeclView> memberLookup;
        for (auto memberDecl : decl.GetMembers()) {
            size_t declIndex = 0;
            for (const auto& declarator : memberDecl->GetDeclarators()) {
                if (declarator.nameToken.IsIdentifier()) {
                    memberLookup.Insert({declarator.nameToken.text.Str(), DeclView{memberDecl, declIndex}});
                }
                declIndex += 1;
            }
        }

        auto result = arena.Construct<Type>(typeName.Str(), StructTypeDesc{
                                                                .name    = decl.GetNameToken() ? typeName.Str() : "",
                                                                .members = std::move(memberDesc),
                                                                .decl    = &decl,
                                                                .memberDeclLookup = std::move(memberLookup),
                                                            });
        return result;
    }

    auto AstContext::CreateInterfaceBlockType(AstInterfaceBlockDecl& decl) -> const Type*
    {
        std::vector<std::pair<std::string, const Type*>> memberDesc;
        for (auto memberDecl : decl.GetMembers()) {
            for (const auto& declarator : memberDecl->GetDeclarators()) {
                auto typeDesc = GetArrayType(memberDecl->GetQualType()->GetResolvedType(), declarator.arraySize);
                memberDesc.push_back({declarator.nameToken.text.Str(), typeDesc});
            }
        }

        StringView typeName = "__UnnamedBlockType";
        if (decl.GetNameToken().IsIdentifier()) {
            typeName = decl.GetNameToken().text.StrView();
        }

        UnorderedStringMap<DeclView> memberLookup;
        for (auto memberDecl : decl.GetMembers()) {
            size_t declIndex = 0;
            for (const auto& declarator : memberDecl->GetDeclarators()) {
                if (declarator.nameToken.IsIdentifier()) {
                    memberLookup.Insert({declarator.nameToken.text.Str(), DeclView{memberDecl, declIndex}});
                }
                declIndex += 1;
            }
        }

        auto result = arena.Construct<Type>(typeName.Str(), StructTypeDesc{
                                                                .name             = typeName.Str(),
                                                                .members          = std::move(memberDesc),
                                                                .decl             = &decl,
                                                                .memberDeclLookup = std::move(memberLookup),
                                                            });
        return result;
    }

    auto AstContext::GetArrayType(const Type* elementType, const AstArraySpec* arraySpec) -> const Type*
    {
        if (elementType->IsError()) {
            // We cannot construct arrays upon error type
            return elementType;
        }

        const Type* resultType = elementType;
        if (arraySpec != nullptr && !arraySpec->GetSizeList().empty()) {
            for (auto arrayDim : arraySpec->GetSizeList()) {
                int dimSizeValue = 0;
                if (arrayDim != nullptr) {
                    const auto& dimSize = EvalAstExpr(*arrayDim);
                    if (dimSize.IsScalarInt32()) {
                        dimSizeValue = dimSize.GetInt32Value();
                    }
                }

                resultType = GetArrayType(resultType, dimSizeValue);
            }
        }

        return resultType;
    }

    auto AstContext::GetArrayType(const Type* elementType, size_t dimSize) -> const Type*
    {
        if (elementType->IsError()) {
            // We cannot construct arrays upon error type
            return elementType;
        }

        auto& cachedItem = arrayTypes[std::pair{elementType, dimSize}];
        if (cachedItem == nullptr) {
            std::string debugName = elementType->GetDebugName().Str();
            // FIXME: error vs runtime-sized
            if (dimSize != 0) {
                debugName += fmt::format("[{}]", dimSize);
            }
            else {
                debugName += "[]";
            }

            cachedItem = arena.Construct<Type>(std::move(debugName),
                                               ArrayTypeDesc{.elementType = elementType, .dimSize = dimSize});
        }

        return cachedItem;
    }
}; // namespace glsld
#include "Ast/Misc.h"
#include "Compiler/SymbolTable.h"

namespace glsld
{
    auto SymbolTableLevel::AddFunctionDecl(AstFunctionDecl& decl) -> void
    {
        if (!decl.GetNameToken().IsIdentifier()) {
            return;
        }

        // FIXME: we need to deduplicate since a function could be declared multiple times
        auto name = decl.GetNameToken().text.Str();
        if (!name.empty()) {
            std::vector<FunctionParamSymbolEntry> paramEntries;
            for (auto paramDecl : decl.GetParams()) {
                auto quals = paramDecl->GetQualType()->GetQualifiers();
                paramEntries.push_back(FunctionParamSymbolEntry{
                    .type     = paramDecl->GetResolvedType(),
                    .isInput  = paramDecl->IsInputParam(),
                    .isOutput = paramDecl->IsOutputParam(),
                });
            }
            funcDeclLookup.Insert(
                {std::move(name), FunctionSymbolEntry{.decl = &decl, .paramEntries = std::move(paramEntries)}});
        }
    }

    auto SymbolTableLevel::AddStructDecl(AstStructDecl& decl) -> void
    {
        if (decl.GetNameToken() && decl.GetNameToken()->IsIdentifier()) {
            TryAddSymbol(*decl.GetNameToken(), decl);
        }
    }

    auto SymbolTableLevel::AddInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
    {
        if (decl.GetDeclarator()) {
            // For named interface block, add the decl token for the interface block
            TryAddSymbol(decl.GetDeclarator()->nameToken, decl);
        }
        else {
            // For unnamed interface block, names of internal members should be directly added to the current scope
            for (auto memberDecl : decl.GetMembers()) {
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    if (declarator.nameToken.IsIdentifier()) {
                        TryAddSymbol(declarator.nameToken, *memberDecl);
                    }
                }
            }
        }
    }

    auto SymbolTableLevel::AddVariableDecl(AstVariableDecl& decl) -> void
    {
        for (auto declarator : decl.GetDeclarators()) {
            if (declarator.nameToken.IsIdentifier()) {
                TryAddSymbol(declarator.nameToken, decl);
            }
        }
    }

    auto SymbolTableLevel::AddParamDecl(AstParamDecl& decl) -> void
    {
        if (decl.GetDeclarator() && decl.GetDeclarator()->nameToken.IsIdentifier()) {
            TryAddSymbol(decl.GetDeclarator()->nameToken, decl);
        }
    }

    auto SymbolTableLevel::TryAddSymbol(AstSyntaxToken nameToken, const AstDecl& decl) -> bool
    {
        if (freezed) {
            assert("Trying to add a symbol to a freezed symbol table");
            return false;
        }

        // FIXME: avoid string allocation
        auto name = nameToken.text.Str();
        if (name.empty()) {
            return false;
        }

        auto [it, success] = declLookup.Insert({std::move(name), DeclView{&decl}});
        return success;
    }

    auto SymbolTable::PushLevel(DeclScope scope) -> void
    {
        assert(scope != DeclScope::Global && "Global scope is not allowed to be pushed");
        levels.push_back(new SymbolTableLevel(scope));
    }

    auto SymbolTable::PopLevel() -> void
    {
        if (levels.size() > GetGlobalLevels().size()) {
            delete levels.back();
            levels.pop_back();
        }
        else {
            assert(false && "Poping global scope is not allowed");
        }
    }

    auto SymbolTable::FindSymbol(StringView name) const -> DeclView
    {
        for (auto level : std::views::reverse(levels)) {
            if (auto declView = level->FindSymbol(name); declView.IsValid()) {
                return declView;
            }
        }

        return DeclView{};
    }

    auto SymbolTable::FindFunction(StringView name, const std::vector<const Type*>& argTypes,
                                   bool requireExactMatch) const -> const AstFunctionDecl*
    {
        // First pass: filter out candidates that's invocable with the given argument types
        std::vector<const FunctionSymbolEntry*> candidateList;
        for (auto level : GetGlobalLevels()) {
            for (auto candidate : level->FindFunctionCandidate(name)) {
                if (candidate->paramEntries.size() == argTypes.size()) {
                    // Fast path for exact match
                    if (std::ranges::equal(candidate->paramEntries, argTypes,
                                           [](const FunctionParamSymbolEntry& entry, const Type* argType) {
                                               return entry.type->IsSameWith(argType);
                                           })) {
                        return candidate->decl;
                    }

                    auto convertible = true;
                    for (size_t i = 0; i < argTypes.size(); ++i) {
                        if (candidate->paramEntries[i].isInput &&
                            !argTypes[i]->IsConvertibleTo(candidate->paramEntries[i].type)) {
                            convertible = false;
                            break;
                        }
                        if (candidate->paramEntries[i].isOutput &&
                            !candidate->paramEntries[i].type->IsConvertibleTo(argTypes[i])) {
                            convertible = false;
                            break;
                        }
                    }

                    if (convertible && !requireExactMatch) {
                        candidateList.push_back(candidate);
                    }
                }
            }
        }

        // Second pass: find the single best candidate if any
        const AstFunctionDecl* bestCandidate = nullptr;
        for (auto candidate : candidateList) {
            bool isAlwaysBetter = true;
            for (auto otherCandidate : candidateList) {
                if (candidate == otherCandidate) {
                    continue;
                }

                // F1 is better than F2 if:
                // 1. No conversion in F1 is worse than F2
                // 2. At least one conversion in F1 is better than F2
                int numCandidateBetter      = 0;
                int numOtherCandidateBetter = 0;
                for (size_t i = 0; i < argTypes.size(); ++i) {
                    const auto& candidateParam      = candidate->paramEntries[i];
                    const auto& otherCandidateParam = otherCandidate->paramEntries[i];

                    if (candidateParam.isInput && otherCandidateParam.isInput) {
                        if (argTypes[i]->HasBetterConversion(candidateParam.type, otherCandidateParam.type)) {
                            numCandidateBetter += 1;
                        }
                        if (argTypes[i]->HasBetterConversion(otherCandidateParam.type, candidateParam.type)) {
                            numOtherCandidateBetter += 1;
                        }
                    }

                    // FIXME: Do we need special handling for output parameters?
                }
                if (numCandidateBetter == 0 || numOtherCandidateBetter > 0) {
                    isAlwaysBetter = false;
                    break;
                }
            }

            if (isAlwaysBetter) {
                if (bestCandidate) {
                    // Ambiguous
                    bestCandidate = nullptr;
                    break;
                }
                else {
                    bestCandidate = candidate->decl;
                }
            }
        }

        return bestCandidate;
    }
} // namespace glsld
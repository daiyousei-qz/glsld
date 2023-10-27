#include "Compiler/SymbolTable.h"

namespace glsld
{
    auto SymbolTableLevel::AddFunctionDecl(AstFunctionDecl& decl) -> void
    {
        if (!decl.GetDeclTok().IsIdentifier()) {
            return;
        }

        // deduplicate?
        auto name = decl.GetDeclTok().text.Str();
        if (!name.empty()) {
            std::vector<const Type*> paramTypes;
            for (auto paramDecl : decl.GetParams()) {
                paramTypes.push_back(paramDecl->GetResolvedType());
            }
            funcDeclLookup.Insert(
                {std::move(name), FunctionSymbolEntry{.decl = &decl, .paramTypes = std::move(paramTypes)}});
        }
    }

    auto SymbolTableLevel::AddStructDecl(AstStructDecl& decl) -> void
    {
        if (decl.GetDeclTok() && decl.GetDeclTok()->IsIdentifier()) {
            TryAddSymbol(*decl.GetDeclTok(), decl);
        }
    }

    auto SymbolTableLevel::AddInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
    {
        if (decl.GetDeclarator()) {
            // For named interface block, add the decl token for the interface block
            TryAddSymbol(decl.GetDeclarator()->declTok, decl);
        }
        else {
            // For unnamed interface block, names of internal members should be directly added to the current scope
            for (auto memberDecl : decl.GetMembers()) {
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    if (declarator.declTok.IsIdentifier()) {
                        TryAddSymbol(declarator.declTok, *memberDecl);
                    }
                }
            }
        }
    }

    auto SymbolTableLevel::AddVariableDecl(AstVariableDecl& decl) -> void
    {
        for (auto declarator : decl.GetDeclarators()) {
            if (declarator.declTok.IsIdentifier()) {
                TryAddSymbol(declarator.declTok, decl);
            }
        }
    }

    auto SymbolTableLevel::AddParamDecl(AstParamDecl& decl) -> void
    {

        if (decl.GetDeclarator().declTok.IsIdentifier()) {
            TryAddSymbol(decl.GetDeclarator().declTok, decl);
        }
    }

    auto SymbolTableLevel::TryAddSymbol(SyntaxToken declToken, const AstDecl& decl) -> bool
    {
        if (freezed) {
            assert("Trying to add a symbol to a freezed symbol table");
            return false;
        }

        // FIXME: avoid string allocation
        auto name = declToken.text.Str();
        if (name.empty()) {
            return false;
        }

        auto [it, success] = declLookup.Insert({std::move(name), DeclView{&decl}});
        return success;
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

    auto SymbolTable::FindFunction(StringView name, const std::vector<const Type*>& argTypes) const
        -> const AstFunctionDecl*
    {
        // First pass: filter out candidates that's invocable with the given argument types
        std::vector<const FunctionSymbolEntry*> candidateList;
        for (auto level : GetGlobalLevels()) {
            for (auto candidate : level->FindFunctionCandidate(name)) {
                if (candidate->paramTypes.size() == argTypes.size()) {
                    // Fast path for exact match
                    if (std::ranges::equal(candidate->paramTypes, argTypes)) {
                        return candidate->decl;
                    }

                    auto convertible = true;
                    for (size_t i = 0; i < argTypes.size(); ++i) {
                        if (!argTypes[i]->IsConvertibleTo(candidate->paramTypes[i])) {
                            convertible = false;
                            break;
                        }
                    }

                    if (convertible) {
                        candidateList.push_back(candidate);
                    }
                }
            }
        }

        if (candidateList.empty()) {
            return nullptr;
        }
        else if (candidateList.size() == 1) {
            return candidateList[0]->decl;
        }

        // Second pass: select the best match with partial order
        std::vector<const FunctionSymbolEntry*> currentBestList;
        std::vector<const FunctionSymbolEntry*> nextBestList;
        currentBestList.reserve(candidateList.size());
        nextBestList.reserve(candidateList.size());

        for (auto candidate : candidateList) {
            auto pickedCandidate = false;
            for (auto currentBest : currentBestList) {
                ArrayView<const Type*> candidateParamTypes   = candidate->paramTypes;
                ArrayView<const Type*> currentBestParamTypes = currentBest->paramTypes;
                int numCandidateBetter                       = 0;
                int numCurrentBetter                         = 0;
                for (size_t i = 0; i < argTypes.size(); ++i) {
                    if (argTypes[i]->HasBetterConversion(candidateParamTypes[i], currentBestParamTypes[i])) {
                        numCandidateBetter += 1;
                    }
                    if (argTypes[i]->HasBetterConversion(candidateParamTypes[i], currentBestParamTypes[i])) {
                        numCurrentBetter += 1;
                    }
                }

                // F1 is better than F2 if:
                // 1. No conversion in F1 is worse than F2
                // 2. At least one conversion in F1 is better than F2
                if (numCandidateBetter > 0 && numCurrentBetter == 0) {
                    // Candidate is better, but we still have to deduplicate
                    if (!pickedCandidate) {
                        pickedCandidate = true;
                        nextBestList.push_back(candidate);
                    }
                }
                else if (numCurrentBetter > 0 && numCandidateBetter == 0) {
                    // Current is better
                    nextBestList.push_back(currentBest);
                }
            }

            if (nextBestList.empty()) {
                // No better could be determined
                std::ranges::copy(currentBestList, std::back_inserter(nextBestList));
                nextBestList.push_back(candidate);
            }

            // swap buffer
            std::swap(currentBestList, nextBestList);
            nextBestList.clear();
        }

        if (currentBestList.size() == 1) {
            return currentBestList[0]->decl;
        }
        else {
            return nullptr;
        }
    }
} // namespace glsld
#include "Compiler/SymbolTable.h"

namespace glsld
{
    auto SymbolTable::AddFunctionDecl(AstFunctionDecl& decl) -> void
    {
        GLSLD_ASSERT(parent == nullptr);
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
            funcDeclLookup.insert(
                {std::move(name), FunctionSymbolEntry{.decl = &decl, .paramTypes = std::move(paramTypes)}});
        }
    }

    auto SymbolTable::AddStructDecl(AstStructDecl& decl) -> void
    {
        if (decl.GetDeclTok() && decl.GetDeclTok()->IsIdentifier()) {
            TryAddSymbol(*decl.GetDeclTok(), decl);
        }
    }

    auto SymbolTable::AddInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
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

    auto SymbolTable::AddVariableDecl(AstVariableDecl& decl) -> void
    {
        for (auto declarator : decl.GetDeclarators()) {
            if (declarator.declTok.IsIdentifier()) {
                TryAddSymbol(declarator.declTok, decl);
            }
        }
    }

    auto SymbolTable::AddParamDecl(AstParamDecl& decl) -> void
    {
        if (decl.GetDeclarator().declTok.IsIdentifier()) {
            TryAddSymbol(decl.GetDeclarator().declTok, decl);
        }
    }

    auto SymbolTable::FindFunction(const std::string& name, const std::vector<const Type*>& argTypes) const
        -> const AstFunctionDecl*
    {
        // FIXME: Move resolution to its own function
        // FIXME: impl correct resolution
        auto [itBegin, itEnd] = funcDeclLookup.equal_range(name);

        // First pass: resolve candidates and early return if exact match is found
        std::vector<const FunctionSymbolEntry*> candidateList;
        for (auto it = itBegin; it != itEnd; ++it) {
            const auto& funcEntry = it->second;

            ArrayView<const Type*> paramTypes = funcEntry.paramTypes;

            // FIXME: use proper compare, this could fail for composite type
            if (std::ranges::equal(paramTypes, argTypes)) {
                return funcEntry.decl;
            }

            if (paramTypes.size() == argTypes.size()) {
                auto convertible = true;
                for (size_t i = 0; i < argTypes.size(); ++i) {
                    if (!argTypes[i]->IsConvertibleTo(paramTypes[i])) {
                        convertible = false;
                        break;
                    }
                }

                if (convertible) {
                    candidateList.push_back(&funcEntry);
                }
            }
        }

        if (candidateList.empty()) {
            return nullptr;
        }

        // Second pass: select the best match with partial order
        std::vector<const FunctionSymbolEntry*> currentBestList;
        std::vector<const FunctionSymbolEntry*> nextBestList;
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

                if (numCandidateBetter > 0 && numCurrentBetter == 0) {
                    // Candidate is better, but we still have to deduplicate
                    if (!pickedCandidate) {
                        pickedCandidate = true;
                        nextBestList.push_back(candidate);
                    }
                }
                if (numCurrentBetter > 0 && numCandidateBetter == 0) {
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

    auto SymbolTable::FindSymbol(const std::string& name) const -> DeclView
    {
        if (auto it = declLookup.find(name); it != declLookup.end()) {
            return it->second;
        }
        else if (parent) {
            return parent->FindSymbol(name);
        }
        else {
            return DeclView{};
        }
    }

    auto SymbolTable::TryAddSymbol(SyntaxToken declToken, const AstDecl& decl) -> bool
    {
        // FIXME: avoid string allocation
        auto name = declToken.text.Str();
        if (name.empty()) {
            return false;
        }

        auto& entry = declLookup[std::move(name)];
        if (!entry.IsValid()) {
            entry = DeclView{&decl};
            return true;
        }
        else {
            return false;
        }
    }
} // namespace glsld
#include "SymbolTable.h"

namespace glsld
{
    auto SymbolTable::AddFunction(AstFunctionDecl& decl) -> void
    {
        GLSLD_ASSERT(levels.size() == 1);

        if (decl.GetName().klass != TokenKlass::Identifier) {
            return;
        }

        // deduplicate?
        auto name = decl.GetName().text.Str();
        if (!name.empty()) {
            std::vector<const TypeDesc*> paramTypes;
            for (auto paramDecl : decl.GetParams()) {
                paramTypes.push_back(paramDecl->GetType()->GetResolvedType());
            }

            funcDeclLookup.insert(
                {decl.GetName().text.Str(), FunctionEntry{.paramTypes = std::move(paramTypes), .decl = &decl}});
        }
    }

    auto SymbolTable::AddStructDecl(AstStructDecl& decl) -> void
    {
        if (decl.GetDeclToken() && decl.GetDeclToken()->klass == TokenKlass::Identifier) {
            TryAddSymbol(*decl.GetDeclToken(), decl);
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
                    if (declarator.declTok.klass == TokenKlass::Identifier) {
                        TryAddSymbol(declarator.declTok, *memberDecl);
                    }
                }
            }
        }
    }

    auto SymbolTable::AddVariableDecl(AstVariableDecl& decl) -> void
    {
        for (auto declarator : decl.GetDeclarators()) {
            if (declarator.declTok.klass == TokenKlass::Identifier) {
                TryAddSymbol(declarator.declTok, decl);
            }
        }
    }

    auto SymbolTable::AddParamDecl(AstParamDecl& decl) -> void
    {
        if (decl.GetDeclarator() && decl.GetDeclarator()->declTok.IsIdentifier()) {
            TryAddSymbol(decl.GetDeclarator()->declTok, decl);
        }
    }

    auto SymbolTable::FindFunction(const std::string& name, const std::vector<const TypeDesc*>& argTypes) const
        -> AstFunctionDecl*
    {
        // FIXME: impl correct resolution
        auto [itBegin, itEnd] = funcDeclLookup.equal_range(name);

        // First pass: resolve candidates and early return if exact match is found
        std::vector<FunctionEntry> candidateList;
        for (auto it = itBegin; it != itEnd; ++it) {
            auto funcEntry = it->second;

            // FIXME: use proper compare, this could fail for composite type
            if (funcEntry.paramTypes == argTypes) {
                return funcEntry.decl;
            }

            if (funcEntry.paramTypes.size() == argTypes.size()) {
                auto convertible = true;
                for (size_t i = 0; i < argTypes.size(); ++i) {
                    if (!argTypes[i]->IsConvertibleTo(funcEntry.paramTypes[i])) {
                        convertible = false;
                        break;
                    }
                }

                if (convertible) {
                    candidateList.push_back(funcEntry);
                }
            }
        }

        if (candidateList.empty()) {
            return nullptr;
        }

        // Second pass: select the best match with partial order
        std::vector<FunctionEntry> currentBestList;
        std::vector<FunctionEntry> nextBestList;
        for (auto candidate : candidateList) {
            auto pickedCandidate = false;
            for (auto currentBest : currentBestList) {
                int numCandidateBetter = 0;
                int numCurrentBetter   = 0;
                for (size_t i = 0; i < argTypes.size(); ++i) {
                    if (argTypes[i]->HasBetterConversion(candidate.paramTypes[i], currentBest.paramTypes[i])) {
                        numCandidateBetter += 1;
                    }
                    if (argTypes[i]->HasBetterConversion(currentBest.paramTypes[i], candidate.paramTypes[i])) {
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
            return currentBestList[0].decl;
        }
        else {
            return nullptr;
        }
    }

    auto SymbolTable::FindSymbol(const std::string& name) const -> DeclView
    {
        GLSLD_ASSERT(!levels.empty());
        for (const auto& level : levels | std::views::reverse) {
            if (auto it = level.declLookup.find(name); it != level.declLookup.end()) {
                return it->second;
            }
        }

        return DeclView{};
    }
} // namespace glsld
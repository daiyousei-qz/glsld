#pragma once
#include "Common.h"
#include "AstDecl.h"
#include "Typing.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <ranges>

namespace glsld
{
    class SymbolTable
    {
    public:
        SymbolTable()
        {
            PushScope();
        }

        auto PushScope() -> void
        {
            levels.push_back(SymbolTableLevel{});
        }
        auto PopScope() -> void
        {
            // global scope cannot be popped
            GLSLD_ASSERT(levels.size() > 1);
            levels.pop_back();
        }

        auto AddFunction(AstFunctionDecl& decl) -> void
        {
            GLSLD_ASSERT(levels.size() == 1);
            // deduplicate?
            auto name = decl.GetName().text.Str();
            if (!name.empty()) {
                std::vector<const TypeDesc*> paramTypes;
                for (auto paramDecl : decl.GetParams()) {
                    paramTypes.push_back(paramDecl->GetType()->GetTypeDesc());
                }

                funcDeclLookup.insert(
                    {decl.GetName().text.Str(), FunctionRegistry{.paramTypes = std::move(paramTypes), .decl = &decl}});
            }
        }

        auto FindFunction(const std::string& name, const std::vector<const TypeDesc*>& argTypes) -> AstFunctionDecl*
        {
            // FIXME: impl correct resolution
            auto [itBegin, itEnd] = funcDeclLookup.equal_range(name);

            // First pass: resolve candidates and early return if exact match is found
            std::vector<FunctionRegistry> candidateList;
            for (auto it = itBegin; it != itEnd; ++it) {
                auto funcEntry = it->second;

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
            std::vector<FunctionRegistry> currentBest;
            std::vector<FunctionRegistry> nextBest;
            for (auto candidate : candidateList) {
                auto pickedCandidate = false;
                for (auto best : currentBest) {
                    int numCandidateBetter = 0;
                    int numCurrentBetter   = 0;
                    for (size_t i = 0; i < argTypes.size(); ++i) {
                        if (argTypes[i]->HasBetterConversion(candidate.paramTypes[i], best.paramTypes[i])) {
                            numCandidateBetter += 1;
                        }
                        if (argTypes[i]->HasBetterConversion(best.paramTypes[i], candidate.paramTypes[i])) {
                            numCurrentBetter += 1;
                        }
                    }

                    if (numCandidateBetter > 0 && numCurrentBetter == 0) {
                        // Candidate is better, but we still have to deduplicate
                        if (!pickedCandidate) {
                            pickedCandidate = true;
                            nextBest.push_back(candidate);
                        }
                    }
                    if (numCurrentBetter > 0 && numCandidateBetter == 0) {
                        // Current is better
                        nextBest.push_back(candidate);
                    }
                }

                if (nextBest.empty()) {
                    // No better could be determined
                    std::ranges::copy(currentBest, std::back_inserter(nextBest));
                    nextBest.push_back(candidate);
                }

                // swap buffer
                std::swap(currentBest, nextBest);
                nextBest.clear();
            }

            if (currentBest.size() == 1) {
                return currentBest[0].decl;
            }
            else {
                return nullptr;
            }
        }

        auto AddSymbol(std::string name, AstDecl* decl) -> void
        {
            GLSLD_ASSERT(!levels.empty());
            levels.back().declLookup[name] = decl;
        }

        auto FindSymbol(const std::string& name) -> AstDecl*
        {
            GLSLD_ASSERT(!levels.empty());
            for (const auto& level : levels | std::views::reverse) {
                if (auto it = level.declLookup.find(name); it != level.declLookup.end()) {
                    return it->second;
                }
            }

            return nullptr;
        }

    private:
        struct FunctionRegistry
        {
            std::vector<const TypeDesc*> paramTypes;
            AstFunctionDecl* decl;
        };

        struct SymbolTableLevel
        {
            std::unordered_map<std::string, AstDecl*> declLookup;
        };

        std::unordered_multimap<std::string, FunctionRegistry> funcDeclLookup;
        std::vector<SymbolTableLevel> levels;
    };
} // namespace glsld
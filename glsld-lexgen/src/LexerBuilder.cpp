#include "FlatSet.h"
#include "LexerBuilder.h"

#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <algorithm>

namespace glsld
{
    // NOTE we assume state 0 is the start state
    auto RemoveEpsilon(const NfaAutomata& nfa) -> NfaAutomata
    {
        NfaAutomata result;

        // Map old nfa states to the one registered in the new automata
        std::unordered_map<const NfaState*, NfaState*> newStateLookup;
        auto getNewState = [&](const NfaState* oldState) {
            auto& newState = newStateLookup[oldState];
            if (newState == nullptr) {
                newState = result.NewState();
            }

            return newState;
        };

        // Collect states that 1) being start state provided 2) have at least one non-epsilon incoming edge
        std::vector<const NfaState*> startStates{nfa[0]};
        // Collect states that could be reached via only epsilon transitions
        std::vector<const NfaState*> equivalentStates;
        // Collect immediate transitions that could be reached from the current start state
        std::vector<std::pair<int, const NfaState*>> transitions;

        // NOTE we cannot use iterator based for-loop because of iterator invalidation
        for (size_t i = 0; i < startStates.size(); ++i) {
            equivalentStates.push_back(startStates[i]);

            for (size_t j = 0; j < equivalentStates.size(); ++j) {
                for (auto [ch, targetOld] : equivalentStates[j]->GetTransition()) {
                    if (ch == EpsilonCodepoint) {
                        // Add the target state to equalvalence set
                        if (std::ranges::find(equivalentStates, targetOld) == equivalentStates.end()) {
                            equivalentStates.push_back(targetOld);
                        }
                    }
                    else {
                        // Target state might be a new unique state in the new automata
                        if (std::ranges::find(startStates, targetOld) == startStates.end()) {
                            startStates.push_back(targetOld);
                        }

                        // Add the transition to the set of new transitions
                        if (std::ranges::find(transitions, std::pair{ch, targetOld}) == transitions.end()) {
                            transitions.emplace_back(ch, targetOld);
                        }
                    }
                }
            }

            // Collect accept id from equivalent states
            int acceptId = -1;
            for (auto state : equivalentStates) {
                if (state->GetAcceptId() != -1 && state->GetAcceptId() != acceptId) {
                    if (acceptId != -1) {
                        throw std::logic_error{"error: multiple accept id"};
                    }
                    acceptId = state->GetAcceptId();
                }
            }

            // Copy accept id and transitions to the newly mapped state
            NfaState* src = getNewState(startStates[i]);
            src->SetAcceptId(acceptId);
            for (auto [ch, oldTarget] : transitions) {
                src->AddTransition(ch, *getNewState(oldTarget));
            }

            // Clear buffer for next iteration
            equivalentStates.clear();
            transitions.clear();
        }

        return result;
    }

    // NOTE we assume state 0 is the start state
    auto CreateDfa(const NfaAutomata& nfa) -> NfaAutomata
    {
        NfaAutomata result;

        // map old nfa states to the one registered in the new automata
        std::map<FlatSet<const NfaState*>, NfaState*> old2new;
        auto getNewState = [&](const FlatSet<const NfaState*>& old) -> NfaState* {
            auto& newState = old2new[old];
            if (newState == nullptr) {
                int acceptId = -1;
                for (const NfaState* state : old) {
                    if (state->GetAcceptId() >= 0 && state->GetAcceptId() != acceptId) {
                        if (acceptId != -1) {
                            throw "error";
                        }

                        acceptId = state->GetAcceptId();
                    }
                }

                newState = result.NewState();
                newState->SetAcceptId(acceptId);
            }

            return newState;
        };

        std::vector<FlatSet<const NfaState*>> subsetStates{{nfa[0]}};
        std::vector<NfaState*> mappedSubsetStates{getNewState({nfa[0]})};
        auto newStartState = mappedSubsetStates.front();
        for (size_t i = 0; i < subsetStates.size(); ++i) {
            std::unordered_map<int, FlatSet<const NfaState*>> subsetTransitions;
            for (const NfaState* srcOld : subsetStates[i]) {
                for (auto [ch, targetOld] : srcOld->GetTransition()) {
                    subsetTransitions[ch].Insert(targetOld);
                }
            }

            auto srcState = getNewState(subsetStates[i]);
            for (auto& [ch, subset] : subsetTransitions) {
                auto targetState = getNewState(subset);

                if (std::ranges::find(mappedSubsetStates, targetState) == mappedSubsetStates.end()) {
                    subsetStates.push_back(std::move(subset)); // invalidating subset
                    mappedSubsetStates.push_back(targetState);
                }

                // assert(ch >= 0 && ch < num_symbols)
                srcState->AddTransition(ch, *targetState);
            }
        }

        return result;
    }

    // NOTE we assume state 0 is the start state
    auto MinimizeDfa(const NfaAutomata& dfa) -> NfaAutomata
    {
        // For each (s1, s2) in the set, s1 and s2 is distinguishable.
        // NOTE we require s1.index <= s2.index
        std::set<std::pair<const NfaState*, const NfaState*>> distinguishableSet;
        auto isDistinguishable = [&](const NfaState* lhs, const NfaState* rhs) -> bool {
            auto testPair = lhs->GetIndex() < rhs->GetIndex() ? std::pair{lhs, rhs} : std::pair{rhs, lhs};
            return distinguishableSet.contains(testPair);
        };
        auto markDistinguishable = [&](const NfaState* lhs, const NfaState* rhs) -> void {
            GLSLD_ASSERT(lhs != rhs);
            auto testPair = lhs->GetIndex() < rhs->GetIndex() ? std::pair{lhs, rhs} : std::pair{rhs, lhs};
            distinguishableSet.insert(testPair);
        };

        std::vector<std::pair<int, int>> unmarkedStatePairs;
        std::vector<std::pair<int, int>> nextUnmarkedPairs;

        // Initialize workset. States with different accept ids are distinguishable
        for (int i = 0; i < dfa.NumState(); ++i) {
            for (int j = i + 1; j < dfa.NumState(); ++j) {
                if (dfa[i]->GetAcceptId() != dfa[j]->GetAcceptId()) {
                    markDistinguishable(dfa[i], dfa[j]);
                }
                else {
                    unmarkedStatePairs.emplace_back(i, j);
                }
            }
        }

        // Iterate through the workset and mark any distinguishable pairs of states
        while (true) {
            for (auto [i, j] : unmarkedStatePairs) {
                bool toMark = false;

                auto transitionI = dfa[i]->GetTransition();
                auto transitionJ = dfa[j]->GetTransition();

                if (transitionI.size() != transitionJ.size()) {
                    toMark = true;
                }
                else {
                    for (size_t k = 0; k < transitionI.size(); ++k) {
                        // Verify this is a canonicalized DFA
                        GLSLD_ASSERT(k == 0 || (transitionI[k].first != EpsilonCodepoint &&
                                                transitionI[k].first >= transitionI[k - 1].first &&
                                                transitionJ[k].first != EpsilonCodepoint &&
                                                transitionJ[k].first >= transitionJ[k - 1].first));

                        if (transitionI[k].first != transitionJ[k].first ||
                            isDistinguishable(transitionI[k].second, transitionJ[k].second)) {
                            toMark = true;
                            break;
                        }
                    }
                }

                if (toMark) {
                    markDistinguishable(dfa[i], dfa[j]);
                }
                else {
                    nextUnmarkedPairs.emplace_back(i, j);
                }
            }

            if (nextUnmarkedPairs.size() == unmarkedStatePairs.size()) {
                // no state pair is marked
                break;
            }

            std::swap(unmarkedStatePairs, nextUnmarkedPairs);
            nextUnmarkedPairs.clear();
        }

        NfaAutomata result;
        std::unordered_map<const NfaState*, NfaState*> old2new;

        std::vector<int> normalizedStateLookup;
        normalizedStateLookup.resize(dfa.NumState());

        for (int stateId = 0; stateId < dfa.NumState(); ++stateId) {
            int mappedStateId = stateId;
            for (int i = 0; i < stateId; ++i) {
                if (!isDistinguishable(dfa[i], dfa[stateId])) {
                    mappedStateId = i;
                    break;
                }
            }

            normalizedStateLookup[stateId] = mappedStateId;
            if (mappedStateId == stateId) {
                old2new[dfa[stateId]] = result.NewState();
            }
        }

        for (int stateId = 0; stateId < dfa.NumState(); ++stateId) {
            if (normalizedStateLookup[stateId] == stateId) {
                auto oldState = dfa[stateId];
                auto newState = old2new.at(oldState);

                newState->SetAcceptId(oldState->GetAcceptId());
                for (auto [ch, targetOldState] : oldState->GetTransition()) {
                    newState->AddTransition(ch, *old2new.at(dfa[normalizedStateLookup[targetOldState->GetIndex()]]));
                }
            }
        }

        return result;
    }

    auto LexerBuilder::Build() -> NfaAutomata
    {
        auto nfa1 = RemoveEpsilon(nfa);
        auto nfa2 = CreateDfa(nfa1);
        nfa2.Canonicalize();
        auto nfa3 = MinimizeDfa(nfa2);

        return std::move(nfa3);
    }

} // namespace glsld

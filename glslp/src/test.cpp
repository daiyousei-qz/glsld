#include "Nfa.h"
#include "FlatSet.h"
#include <array>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <string_view>
#include <numeric>
#include <fmt/format.h>

auto NfaRemoveEpsilon(const NfaAutomata& nfa, const NfaState* start) -> std::pair<NfaAutomata, const NfaState*>
{
    NfaAutomata result;

    // map old nfa states to the one registered in the new automata
    // Map<const NfaState*, const NfaState*> old2new;
    std::vector<std::pair<const NfaState*, NfaState*>> old2new;
    auto get_new_state = [&](const NfaState* old) {
        auto it = std::ranges::find(old2new, old, [](auto x) { return x.first; });
        if (it != old2new.end()) {
            return it->second;
        }
        else {
            return old2new.emplace_back(old, result.NewState()).second;
        }
    };

    // collect states that 1. being start state provided 2. have at least one non-epsilon incoming edge
    std::vector<const NfaState*> startStates{start};
    // collect states that could be reached via only epsilon transitions
    std::vector<const NfaState*> equivalentStates;
    // collect immediate transitions that could be reached from the current start state
    std::vector<std::pair<int, NfaState*>> transitions;
    for (size_t i = 0; i < startStates.size(); ++i) {

        equivalentStates.push_back(startStates[i]);
        for (size_t j = 0; j < equivalentStates.size(); ++j) {
            for (auto [ch, targetOld] : equivalentStates[j]->next) {
                if (ch == EpsilonCodepoint) {
                    if (std::ranges::find(equivalentStates, targetOld) == equivalentStates.end()) {
                        equivalentStates.push_back(targetOld);
                    }
                }
                else {
                    if (std::ranges::find(startStates, targetOld) == startStates.end()) {
                        startStates.push_back(targetOld);
                    }
                    if (std::ranges::find(transitions, std::pair{ch, targetOld}) == transitions.end()) {
                        transitions.emplace_back(ch, targetOld);
                    }
                }
            }
        }

        // collect accept id from equivalent states
        int accept_id = -1;
        for (auto state : equivalentStates) {
            if (state->acceptId != -1 && state->acceptId != accept_id) {
                if (accept_id != -1) {
                    throw "error: multiple accept id";
                }
                accept_id = state->acceptId;
            }
        }

        // copy accept id and transitions to the newly mapped state
        NfaState* src = get_new_state(startStates[i]);
        src->acceptId = accept_id;
        for (auto [ch, old_target] : transitions) {
            src->next.emplace_back(ch, get_new_state(old_target));
        }

        // clear buffer for next iteration
        equivalentStates.clear();
        transitions.clear();
    }

    return {std::move(result), get_new_state(start)};
}

// auto NfaMinimizeDfa2(const NfaAutomata& dfa, const NfaState* start) -> NfaAutomata
// {
//     constexpr int kAlphabetSize = 128;

//     // state_mark_table[i*num_state+j] => (i, j) is marked where i < j should hold
//     // those marked are NOT indistinguishable, we assume indistinguishability by default
//     std::vector<int> stateMarkTable;
//     stateMarkTable.resize(dfa.NumState() * dfa.NumState(), 0);
//     auto isDistinguishable = [&](const NfaState* lhs, const NfaState* rhs) -> bool {
//         return stateMarkTable[std::min(lhs->index, rhs->index) * dfa.NumState() + std::max(lhs->index, rhs->index)];
//     };
//     auto markDistinguishable = [&](const NfaState* lhs, const NfaState* rhs) -> void {
//         stateMarkTable[std::min(lhs->index, rhs->index) * dfa.NumState() + std::max(lhs->index, rhs->index)] = 1;
//     };

//     std::vector<std::pair<int, int>> unmarkedStatePairs;
//     std::vector<std::pair<int, int>> nextUnmarkedPairs;

//     for (int i = 0; i < dfa.NumState(); ++i) {
//         for (int j = i + 1; j < dfa.NumState(); ++j) {
//             if (dfa[i]->acceptId != dfa[j]->acceptId) {
//                 stateMarkTable[i * dfa.NumState() + j] = 1;
//             }
//             else {
//                 unmarkedStatePairs.emplace_back(i, j);
//             }
//         }
//     }

//     while (true) {
//         for (auto [i, j] : unmarkedStatePairs) {
//             bool toMark = false;

//             // for each ch, where x -> p and x -> q
//             for (int ch = 0; ch < kAlphabetSize; ++ch) {
//                 // dfa[i]->next
//                 auto p = dfa[i]->FindNext(ch);
//                 auto q = dfa[j]->FindNext(ch);
//                 if (p == q) {
//                     continue;
//                 }

//                 if (p == nullptr || q == nullptr || isDistinguishable(p, q)) {
//                     toMark = true;
//                     break;
//                 }
//             }

//             if (toMark) {
//                 markDistinguishable(dfa[i], dfa[j]);
//             }
//             else {
//                 nextUnmarkedPairs.emplace_back(i, j);
//             }
//         }

//         if (nextUnmarkedPairs.size() == unmarkedStatePairs.size()) {
//             // no state pair is marked
//             break;
//         }

//         std::swap(unmarkedStatePairs, nextUnmarkedPairs);
//         nextUnmarkedPairs.clear();
//     }

//     std::vector<int> remappedDfaStateLookup;

//     int numState = 0;
//     for (int stateOld = 0; stateOld < dfa.NumState(); ++stateOld) {
//         int stateNew = -1;
//         for (int i = 0; i < stateOld; ++i) {
//             if (!stateMarkTable[i * dfa.NumState() + stateOld]) {
//                 // state_old ~ i
//                 stateNew = i;
//                 break;
//             }
//         }

//         if (stateNew == -1) {
//             stateNew = numState++;
//         }

//         remappedDfaStateLookup.push_back(stateNew);
//     }

//     std::vector<int> transitionLookup;
//     std::vector<int> acceptLookup;
//     transitionLookup.resize(kAlphabetSize * numState, -1);
//     acceptLookup.resize(numState);

//     std::vector<int> stateCopied;
//     stateCopied.resize(numState);
//     for (int state_old = 0; state_old < dfa.NumState(); ++state_old) {
//         int stateNew = remappedDfaStateLookup[state_old];
//         if (stateCopied[stateNew]) {
//             continue;
//         }

//         stateCopied[stateNew] = 1;
//         for (int ch = 0; ch < kAlphabetSize; ++ch) {
//             int target_old = dfa.transitionLookup[state_old * kAlphabetSize + ch];
//             if (target_old != -1) {
//                 transitionLookup[stateNew * kAlphabetSize + ch] = remappedDfaStateLookup[target_old];
//             }
//         }
//         acceptLookup[stateNew] = dfa[state_old]->acceptId;
//     }

//     return DfaAutomaton{
//         .numSymbols       = kAlphabetSize,
//         .numStates        = num_state,
//         .transitionLookup = std::move(transitionLookup),
//         .acceptLookup     = std::move(acceptLookup),
//     };
// }

// assuming state 0 being start state
struct DfaAutomaton
{
    int numSymbols = 128;
    int numStates  = 0;
    std::vector<int> transitionLookup; // .[state*num_symbol+symbol] = target_state
    std::vector<int> acceptLookup;

    constexpr auto Print() -> void
    {
        for (int state = 0; state < numStates; ++state) {
            if (acceptLookup[state] != -1) {
                fmt::print("[{}] @{}\n", state, acceptLookup[state]);
            }
            else {
                fmt::print("[{}]\n", state);
            }

            for (int ch = 0; ch < numSymbols; ++ch) {
                int target = transitionLookup[state * numSymbols + ch];
                if (target != -1) {
                    fmt::print("  {} -> {}\n", static_cast<char>(ch), target);
                }
            }
        }
    }
};

auto NfaCreateDfa(const NfaAutomata& nfa, const NfaState* start) -> DfaAutomaton
{
    DfaAutomaton result;
    auto& [numSymbols, numStates, transitionLookup, acceptLookup] = result;

    // map old nfa states to the one registered in the new automata
    std::map<FlatSet<const NfaState*>, int> old2new;
    auto getNewState = [&](const FlatSet<const NfaState*>& old) {
        if (auto it = old2new.find(old); it != old2new.end()) {
            return it->second;
        }

        int accept_id = -1;
        for (const NfaState* state : old) {
            if (state->acceptId != accept_id) {
                if (accept_id != -1) {
                    throw "error";
                }

                accept_id = state->acceptId;
            }
        }

        int state_id = numStates++;
        old2new.insert({old, state_id});
        transitionLookup.insert(transitionLookup.end(), numSymbols, -1);
        acceptLookup.push_back(accept_id);
        return state_id;
    };

    std::vector<FlatSet<const NfaState*>> subsetStates{{start}};
    std::vector<int> mappedSubsetStates{getNewState({start})};
    for (size_t i = 0; i < subsetStates.size(); ++i) {
        std::unordered_map<int, FlatSet<const NfaState*>> subsetTransitions;
        for (const NfaState* srcOld : subsetStates[i]) {
            for (auto [ch, targetOld] : srcOld->next) {
                subsetTransitions[ch].Insert(targetOld);
            }
        }

        int src_state = getNewState(subsetStates[i]);
        for (auto& [ch, subset] : subsetTransitions) {
            int target_state = getNewState(subset);

            if (std::ranges::find(mappedSubsetStates, target_state) == mappedSubsetStates.end()) {
                subsetStates.push_back(std::move(subset)); // invalidating subset
                mappedSubsetStates.push_back(target_state);
            }

            // assert(ch >= 0 && ch < num_symbols)
            transitionLookup[src_state * numSymbols + ch] = target_state;
        }
    }

    return result;
}

auto NfaMinimizeDfa(const DfaAutomaton& dfa) -> DfaAutomaton
{
    constexpr int kAlphabetSize = 128;

    // state_mark_table[i*num_state+j] => (i, j) is marked where i < j should hold
    // those marked are NOT indistinguishable, we assume indistinguishability by default
    std::vector<int> stateMarkTable;
    stateMarkTable.resize(dfa.numStates * dfa.numStates, 0);

    std::vector<std::pair<int, int>> unmarkedStatePairs;
    std::vector<std::pair<int, int>> nextUnmarkedPairs;

    for (int i = 0; i < dfa.numStates; ++i) {
        for (int j = i + 1; j < dfa.numStates; ++j) {
            if (dfa.acceptLookup[i] != dfa.acceptLookup[j]) {
                stateMarkTable[i * dfa.numStates + j] = 1;
            }
            else {
                unmarkedStatePairs.emplace_back(i, j);
            }
        }
    }

    while (true) {
        for (auto [i, j] : unmarkedStatePairs) {
            bool marked = false;
            for (int ch = 0; ch < kAlphabetSize; ++ch) {
                int p = dfa.transitionLookup[i * kAlphabetSize + ch];
                int q = dfa.transitionLookup[j * kAlphabetSize + ch];
                if (p == q) {
                    continue;
                }

                if (p == -1 || q == -1 || stateMarkTable[std::min(p, q) * dfa.numStates + std::max(p, q)]) {
                    marked = true;
                    break;
                }
            }

            if (marked) {
                stateMarkTable[i * dfa.numStates + j] = 1;
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

    std::vector<int> remappedDfaStateLookup;

    int num_state = 0;
    for (int stateOld = 0; stateOld < dfa.numStates; ++stateOld) {
        int stateNew = -1;
        for (int i = 0; i < stateOld; ++i) {
            if (!stateMarkTable[i * dfa.numStates + stateOld]) {
                // state_old ~ i
                stateNew = i;
                break;
            }
        }

        if (stateNew == -1) {
            stateNew = num_state++;
        }

        remappedDfaStateLookup.push_back(stateNew);
    }

    std::vector<int> transitionLookup;
    std::vector<int> acceptLookup;
    transitionLookup.resize(kAlphabetSize * num_state, -1);
    acceptLookup.resize(num_state);

    std::vector<int> stateCopied;
    stateCopied.resize(num_state);
    for (int state_old = 0; state_old < dfa.numStates; ++state_old) {
        int stateNew = remappedDfaStateLookup[state_old];
        if (stateCopied[stateNew]) {
            continue;
        }

        stateCopied[stateNew] = 1;
        for (int ch = 0; ch < kAlphabetSize; ++ch) {
            int target_old = dfa.transitionLookup[state_old * kAlphabetSize + ch];
            if (target_old != -1) {
                transitionLookup[stateNew * kAlphabetSize + ch] = remappedDfaStateLookup[target_old];
            }
        }
        acceptLookup[stateNew] = dfa.acceptLookup[state_old];
    }

    return DfaAutomaton{
        .numSymbols       = kAlphabetSize,
        .numStates        = num_state,
        .transitionLookup = std::move(transitionLookup),
        .acceptLookup     = std::move(acceptLookup),
    };
}

enum class TokenType
{
    A,
    B,
    C,
};

// CreateLexer({
//     TokenRule("a+", TokenType::A),
//     TokenRule("b+", TokenType::B),
//     TokenRule("c+", TokenType::C),
// }, SkipRule(), RecoveryRule());

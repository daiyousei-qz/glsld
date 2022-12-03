#pragma once
#include <fmt/format.h>
#include <vector>
#include <string_view>
#include <ranges>

static constexpr int EpsilonCodepoint = 0;

struct NfaState
{
    constexpr NfaState(int i) : index(i)
    {
    }
    constexpr ~NfaState()
    {
    }

    auto FindNext(int codepoint) const -> const NfaState*
    {
        auto it = std::ranges::find(next, codepoint, [](const auto pair) { return pair.first; });
        if (it != next.end()) {
            return it->second;
        }
        else {
            return nullptr;
        }
    }

    // (CodePoint, TargetState)
    std::vector<std::pair<int, NfaState*>> next = {};
    int index                                   = -1;
    int acceptId                                = -1;
};

class NfaAutomata
{
public:
    constexpr NfaAutomata()
    {
    }
    constexpr ~NfaAutomata()
    {
        Release();
    }

    constexpr NfaAutomata(const NfaAutomata&)                    = delete;
    constexpr auto operator=(const NfaAutomata&) -> NfaAutomata& = delete;

    constexpr NfaAutomata(NfaAutomata&& other)
    {
        std::swap(states, other.states);
    }
    constexpr auto operator=(NfaAutomata&& other) -> NfaAutomata&
    {
        Release();
        std::swap(states, other.states);

        return *this;
    }

    constexpr auto Release() -> void
    {
        for (NfaState* state : states) {
            delete state;
        }
        states.clear();
    }

    constexpr auto NewState() -> NfaState*
    {
        return states.emplace_back(new NfaState{static_cast<int>(states.size())});
    }

    constexpr auto NumState() const -> size_t
    {
        return states.size();
    }

    constexpr auto begin() const
    {
        return states.begin();
    }

    constexpr auto end() const
    {
        return states.end();
    }

    constexpr auto operator[](size_t index) const -> NfaState*
    {
        return states[index];
    }

    constexpr auto Print() -> void
    {
        for (auto src : states) {
            if (src->acceptId != -1) {
                fmt::print("[{}] @{}\n", src->index, src->acceptId);
            }
            else {
                fmt::print("[{}]\n", src->index);
            }

            for (auto [ch, target] : src->next) {
                if (ch == EpsilonCodepoint) {
                    fmt::print("    -> {}\n", target->index);
                }
                else {
                    fmt::print("  {} -> {}\n", static_cast<char>(ch), target->index);
                }
            }
        }
    }

private:
    std::vector<NfaState*> states = {};
};

template <typename T>
concept RegexT = requires(NfaAutomata& nfa, NfaState& start, const T& regex) {
                     {
                         BuildNfa(nfa, start, regex)
                         } -> std::convertible_to<NfaState*>;
                 };

struct RegexText
{
    std::string_view data;
};

template <RegexT... Ts>
struct RegexSeq
{
    std::tuple<Ts...> exprs;
};

template <RegexT... Ts>
struct RegexChoice
{
    std::tuple<Ts...> exprs;
};
template <RegexT T>
struct RegexRepeat
{
    T expr;
};

constexpr auto Text(std::string_view s)
{
    return RegexText{.data = s};
}
template <RegexT... Ts>
constexpr auto Seq(const Ts&... exprs)
{
    return RegexSeq{.exprs = std::tuple{exprs...}};
}
template <RegexT... Ts>
constexpr auto Choice(const Ts&... exprs)
{
    return RegexChoice{.exprs = std::tuple{exprs...}};
}
template <RegexT T>
constexpr auto Repeat(const T& expr)
{
    return RegexRepeat{.expr = expr};
}

// epsilon
constexpr auto BuildNfa(NfaAutomata& nfa, NfaState& start, NfaState& end) -> NfaState*
{
    return start.next.emplace_back(EpsilonCodepoint, &end).second;
}

// symbol
constexpr auto BuildNfa(NfaAutomata& nfa, NfaState& start, RegexText regex) -> NfaState*
{
    NfaState* cur = &start;
    for (char ch : regex.data) {
        cur = cur->next.emplace_back(ch, nfa.NewState()).second;
    }

    return cur;
}

// concatenation
template <RegexT... Ts>
constexpr auto BuildNfa(NfaAutomata& nfa, NfaState& start, const RegexSeq<Ts...>& regex) -> NfaState*
{
    constexpr auto fold = []<typename Self, typename... Us>(Self self, NfaAutomata& nfa, NfaState& start,
                                                            const std::tuple<Us...>& exprs) {
        NfaState* cur = BuildNfa(nfa, start, exprs.Head());
        if constexpr (sizeof...(Us) > 1) {
            cur = self(self, nfa, *cur, exprs.Tail());
        }

        return cur;
    };

    return fold(fold, nfa, start, regex.exprs);
}

// union
template <RegexT... Ts>
constexpr auto BuildNfa(NfaAutomata& nfa, NfaState& start, const RegexChoice<Ts...>& regex) -> NfaState*
{
    constexpr auto fold = []<typename Self, typename... Us>(Self self, NfaAutomata& nfa, NfaState& start, NfaState& end,
                                                            const std::tuple<Us...>& exprs) {
        BuildNfa(nfa, *BuildNfa(nfa, start, exprs.Head()), end);
        if constexpr (sizeof...(Us) > 1) {
            self(self, nfa, start, end, exprs.Tail());
        }
    };

    NfaState* out = nfa.NewState();
    fold(fold, nfa, start, *out, regex.exprs);
    return out;
}

// kleene star
template <RegexT T>
constexpr auto BuildNfa(NfaAutomata& nfa, NfaState& start, RegexRepeat<T> regex) -> NfaState*
{
    NfaState* in       = nfa.NewState();
    NfaState* out      = nfa.NewState();
    NfaState* childEnd = BuildNfa(nfa, *in, regex.expr);
    BuildNfa(nfa, *childEnd, *in);
    BuildNfa(nfa, *childEnd, *out);
    BuildNfa(nfa, start, *in);
    BuildNfa(nfa, start, *out);

    return out;
}

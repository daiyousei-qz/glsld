#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"
#include "Basic/Print.h"

#include <fmt/format.h>
#include <vector>
#include <string_view>
#include <ranges>
#include <concepts>
#include <algorithm>
#include <tuple>

namespace glsld
{
    static constexpr int EpsilonCodepoint = 0;

    class NfaState;
    class NfaAutomata;

    class NfaState
    {
    public:
        NfaState(int i) : index(i)
        {
        }
        ~NfaState()
        {
        }

        auto GetIndex() const noexcept -> int
        {
            return index;
        }

        auto GetAcceptId() const noexcept -> int
        {
            return acceptId;
        }
        auto SetAcceptId(int id) noexcept -> void
        {
            acceptId = id;
        }

        auto AddTransition(int codepoint, NfaState& target) -> void
        {
            next.emplace_back(codepoint, &target);
        }

        auto GetTransition() const -> ArrayView<std::pair<int, const NfaState*>>
        {
            return next;
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

    private:
        friend NfaAutomata;

        // (CodePoint, TargetState)
        std::vector<std::pair<int, const NfaState*>> next = {};
        int index                                         = -1;
        int acceptId                                      = -1;
    };

    class NfaAutomata
    {
    public:
        NfaAutomata()
        {
        }
        ~NfaAutomata()
        {
            Release();
        }

        NfaAutomata(const NfaAutomata&)                    = delete;
        auto operator=(const NfaAutomata&) -> NfaAutomata& = delete;

        NfaAutomata(NfaAutomata&& other)
        {
            std::swap(states, other.states);
        }
        auto operator=(NfaAutomata&& other) -> NfaAutomata&
        {
            Release();
            std::swap(states, other.states);

            return *this;
        }

        auto Release() -> void
        {
            for (NfaState* state : states) {
                delete state;
            }
            states.clear();
        }

        auto AddEpsilonTransition(NfaState& start, NfaState& end) -> NfaState*
        {
            start.AddTransition(EpsilonCodepoint, end);
            return &end;
        }

        auto Canonicalize()
        {
            for (auto state : states) {
                std::ranges::sort(state->next, {}, [](auto pair) { return pair.first; });
            }
        }

        auto NewState() -> NfaState*
        {
            return states.emplace_back(new NfaState{static_cast<int>(states.size())});
        }

        auto NumState() const -> size_t
        {
            return states.size();
        }

        auto operator[](size_t index) -> NfaState*
        {
            return states[index];
        }

        auto operator[](size_t index) const -> const NfaState*
        {
            return states[index];
        }

        auto PrintAutomata() -> void
        {
            for (auto src : states) {
                if (src->GetAcceptId() != -1) {
                    Print("[{}] @{}\n", src->GetIndex(), src->GetAcceptId());
                }
                else {
                    Print("[{}]\n", src->GetIndex());
                }

                for (auto [ch, target] : src->GetTransition()) {
                    if (ch == EpsilonCodepoint) {
                        Print("      -> {}\n", target->GetIndex());
                    }
                    else {
                        Print("    {} -> {}\n", static_cast<char>(ch), target->GetIndex());
                    }
                }
            }
        }

    private:
        std::vector<NfaState*> states = {};
    };

    template <typename T>
    concept RegexT = requires(const T& regex, NfaAutomata& nfa, NfaState& start) {
        {
            regex.BuildNfa(nfa, start)
        } -> std::convertible_to<NfaState*>;
    };

    struct RegexTextSeq
    {
        RegexTextSeq(StringView data) : data(data)
        {
        }

        auto BuildNfa(NfaAutomata& nfa, NfaState& start) const -> NfaState*
        {
            NfaState* cur = &start;
            for (char ch : data) {
                auto targetState = nfa.NewState();
                cur->AddTransition(ch, *targetState);
                cur = targetState;
            }

            return cur;
        }

        StringView data;
    };

    struct RegexTextChoice
    {
        RegexTextChoice(StringView data) : data(data)
        {
        }

        auto BuildNfa(NfaAutomata& nfa, NfaState& start) const -> NfaState*
        {
            NfaState* out = nfa.NewState();
            for (char ch : data) {
                start.AddTransition(ch, *out);
            }

            return out;
        }

        StringView data;
    };

    struct RegexTextRange
    {
        RegexTextRange(int lowChar, int highChar) : lowChar(lowChar), highChar(highChar)
        {
            GLSLD_ASSERT(lowChar <= highChar);
        }

        auto BuildNfa(NfaAutomata& nfa, NfaState& start) const -> NfaState*
        {
            NfaState* out = nfa.NewState();
            for (int ch = lowChar; ch <= highChar; ++ch) {
                start.AddTransition(ch, *out);
            }

            return out;
        }

        int lowChar;
        int highChar;
    };

    template <RegexT... Ts>
    struct RegexSeq
    {
        RegexSeq(const Ts&... args) : exprs{args...}
        {
        }

        auto BuildNfa(NfaAutomata& nfa, NfaState& start) const -> NfaState*
        {
            return std::apply(&BuildNfaHelper<Ts...>, std::tuple_cat(std::tie(nfa, start), exprs));
        }

        template <RegexT FirstRegex, RegexT... RestRegex>
        static auto BuildNfaHelper(NfaAutomata& nfa, NfaState& start, const FirstRegex& firstRegex,
                                   const RestRegex&... restRegex) -> NfaState*
        {
            NfaState* cur = firstRegex.BuildNfa(nfa, start);
            if constexpr (sizeof...(RestRegex) > 0) {
                return BuildNfaHelper(nfa, *cur, restRegex...);
            }
            else {
                return cur;
            }
        }

        std::tuple<Ts...> exprs;
    };
    template <RegexT... Ts>
    RegexSeq(const Ts&...) -> RegexSeq<std::decay_t<Ts>...>;

    template <RegexT... Ts>
    struct RegexChoice
    {
        RegexChoice(const Ts&... args) : exprs{args...}
        {
        }

        auto BuildNfa(NfaAutomata& nfa, NfaState& start) const -> NfaState*
        {
            auto out = nfa.NewState();
            std::apply(&BuildNfaHelper<Ts...>, std::tuple_cat(std::tie(nfa, start, *out), exprs));
            return out;
        }

        template <RegexT FirstRegex, RegexT... RestRegex>
        static auto BuildNfaHelper(NfaAutomata& nfa, NfaState& start, NfaState& end, const FirstRegex& firstRegex,
                                   const RestRegex&... restRegex) -> void
        {
            nfa.AddEpsilonTransition(*firstRegex.BuildNfa(nfa, start), end);
            if constexpr (sizeof...(RestRegex) > 0) {
                BuildNfaHelper(nfa, start, end, restRegex...);
            }
        }

        std::tuple<Ts...> exprs;
    };
    template <RegexT... Ts>
    RegexChoice(const Ts&...) -> RegexChoice<std::decay_t<Ts>...>;

    template <RegexT T>
    struct RegexRepeat
    {
        constexpr auto BuildNfa(NfaAutomata& nfa, NfaState& start) const -> NfaState*
        {
            NfaState* in       = nfa.NewState();
            NfaState* out      = nfa.NewState();
            NfaState* childEnd = expr.BuildNfa(nfa, *in);
            nfa.AddEpsilonTransition(*childEnd, *in);
            nfa.AddEpsilonTransition(*childEnd, *out);
            nfa.AddEpsilonTransition(start, *in);
            nfa.AddEpsilonTransition(start, *out);

            return out;
        }

        T expr;
    };
    template <RegexT T>
    RegexRepeat(const T&) -> RegexRepeat<std::decay_t<T>>;

    template <RegexT T>
    struct RegexOptional
    {
        // Optional regex
        constexpr auto BuildNfa(NfaAutomata& nfa, NfaState& start) const -> NfaState*
        {
            NfaState* in       = nfa.NewState();
            NfaState* out      = nfa.NewState();
            NfaState* childEnd = expr.BuildNfa(nfa, *in);
            nfa.AddEpsilonTransition(*childEnd, *out);
            nfa.AddEpsilonTransition(start, *in);
            nfa.AddEpsilonTransition(start, *out);

            return out;
        }

        T expr;
    };
    template <RegexT T>
    RegexOptional(const T&) -> RegexOptional<std::decay_t<T>>;

    class LexerBuilder
    {
    public:
        LexerBuilder()
        {
            Initialize();
        }

        auto Initialize() -> void
        {
            nfa.Release();
            nfa.NewState();
        }

        template <RegexT Regex>
        auto AddTokenRegex(int acceptId, const Regex& regex)
        {
            regex.BuildNfa(nfa, *nfa[0])->SetAcceptId(acceptId);
        }

        auto Build() -> NfaAutomata;

    private:
        NfaAutomata nfa;
    };

} // namespace glsld

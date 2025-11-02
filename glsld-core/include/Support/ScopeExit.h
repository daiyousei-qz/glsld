#pragma once

#include <utility>

namespace glsld
{
    template <typename Callback>
    class ScopeExit final
    {
    private:
        // FIXME: [[no_unique_address]]
        Callback callback;
        bool dismissed = false;

    public:
        explicit ScopeExit(Callback&& callback) : callback(std::forward<Callback>(callback))
        {
        }
        ~ScopeExit()
        {
            if (!dismissed) {
                callback();
            }
        }

        ScopeExit(ScopeExit&& other) noexcept
            : callback(std::move(other.callback)), dismissed(std::exchange(other.dismissed, true))
        {
        }

        ScopeExit(const ScopeExit&)                    = delete;
        auto operator=(const ScopeExit&) -> ScopeExit& = delete;
        auto operator=(ScopeExit&&) -> ScopeExit&      = delete;
    };

    template <typename Callback>
    ScopeExit(Callback&&) -> ScopeExit<std::decay_t<Callback>>;
} // namespace glsld
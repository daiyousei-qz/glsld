#pragma once
#include <chrono>
#include <functional>

namespace glsld
{
    class SimpleTimer
    {
    public:
        using Clock     = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;
        using Duration  = Clock::duration;

    private:
        TimePoint t0;
        std::function<void(SimpleTimer&)> callback;

    public:
        SimpleTimer(std::function<void(SimpleTimer&)> callback = {}) : callback(callback)
        {
            Reset();
        }
        ~SimpleTimer()
        {
            if (callback) {
                callback(*this);
            }
        }

        auto Reset() -> void
        {
            t0 = Clock::now();
        }

        template <typename Duration>
        auto GetElapsedTime() -> Duration
        {
            auto t1 = Clock::now();
            return std::chrono::duration_cast<Duration>(t1 - t0);
        }
    };
} // namespace glsld
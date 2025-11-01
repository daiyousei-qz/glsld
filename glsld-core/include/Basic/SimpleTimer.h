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

    public:
        SimpleTimer()
        {
            Reset();
        }

        auto Reset() -> void
        {
            t0 = Clock::now();
        }

        template <typename Duration>
        auto GetElapsedTime() const -> Duration
        {
            auto t1 = Clock::now();
            return std::chrono::duration_cast<Duration>(t1 - t0);
        }

        auto GetElapsedMilliseconds() const -> float
        {
            return GetElapsedTime<std::chrono::duration<float, std::milli>>().count();
        }
    };
} // namespace glsld
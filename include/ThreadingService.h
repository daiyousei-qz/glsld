#pragma once

#include <functional>
#include <thread>
#include <future>

namespace glsld
{
    // This class handles worker threads management and job scheduling
    class ThreadingService
    {
    public:
        // FIXME: use a thread pool
        template <typename F, typename... Args>
        auto ScheduleTask(F&& f, Args&&... args) -> void
        {
            std::thread thd{std::forward<F>(f), std::forward<Args>(args)...};
            thd.detach();
        }

    private:
    };
} // namespace glsld
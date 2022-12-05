#pragma once

#include "Common.h"
#include <BS_thread_pool.hpp>

namespace glsld
{
    // This class handles worker threads management and job scheduling
    class ThreadingService
    {
    public:
        auto Initialize(size_t numWorker)
        {
            GLSLD_ASSERT(numWorker > 0 && numWorker <= 256);
            threadPool.reset(numWorker);
        }

        // FIXME: use a thread pool
        template <typename F, typename... Args>
        auto ScheduleTask(F&& f, Args&&... args) -> void
        {
            threadPool.push_task(std::forward<F>(f), std::forward<Args>(args)...);
        }

    private:
        BS::thread_pool threadPool;
    };
} // namespace glsld
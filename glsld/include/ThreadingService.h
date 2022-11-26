#pragma once

#include "Common.h"
#include <functional>
#include <thread>
#include <future>

namespace glsld
{
    // This class handles worker threads management and job scheduling
    class ThreadingService
    {
    public:
        auto Initialize(size_t numWorker)
        {
            GLSLD_ASSERT(numWorker > 0 && numWorker <= 256);
            for (size_t i = 0; i < numWorker; ++i) {
                workers.emplace_back(&WorkerMain, this);
            }
        }

        // FIXME: use a thread pool
        template <typename F, typename... Args>
        auto ScheduleTask(F&& f, Args&&... args) -> void
        {
            std::thread thd{std::forward<F>(f), std::forward<Args>(args)...};
            thd.detach();
        }

    private:
        auto TerminateWorkers()
        {
        }

        static auto WorkerMain(ThreadingService* self) -> void
        {
        }

        std::mutex mu;
        std::vector<std::thread> workers;
    };
} // namespace glsld
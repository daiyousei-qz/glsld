#pragma once

#include "Basic/Common.h"
#include <stdexec/execution.hpp>
#include <mutex>

namespace glsld
{
    // This isn't the most efficient async latch implementation, but at least it's simple and works.
    class AsyncLatch
    {
    private:
        // Internal mutex to protect the state.
        std::mutex internalMutex;

        struct WaiterBase
        {
            void (*resume)(WaiterBase& self, bool stop) = nullptr;
            WaiterBase* next                            = nullptr;
        };

        // Queue of operation states that are waiting for the latch to reach zero.
        WaiterBase* signalResumeHead = nullptr;
        WaiterBase* signalResumeTail = nullptr;

        // The latch count.
        uint64_t counter;

        auto PushResumeWaiter(WaiterBase& waiter) noexcept -> void
        {
            if (signalResumeTail) {
                assert(signalResumeHead);
                signalResumeTail->next = &waiter;
                signalResumeTail       = &waiter;
            }
            else {
                signalResumeHead = &waiter;
                signalResumeTail = &waiter;
            }
        }

        auto PopResumeWaiter() noexcept -> WaiterBase*
        {
            if (!signalResumeHead) {
                assert(!signalResumeTail);
                return nullptr;
            }
            auto* waiter     = signalResumeHead;
            signalResumeHead = signalResumeHead->next;
            if (!signalResumeHead) {
                signalResumeTail = nullptr;
            }
            waiter->next = nullptr;
            return waiter;
        }

        class AsyncLatchWaitSender : public stdexec::sender_t
        {
        private:
            AsyncLatch* latch;

            template <stdexec::receiver Receiver>
            class OperationState : public stdexec::operation_state_t, private WaiterBase
            {
            private:
                AsyncLatch* latch;
                Receiver receiver;

            public:
                OperationState(AsyncLatch& latch, Receiver&& rcvr)
                    : latch(&latch), receiver(std::forward<Receiver>(rcvr))
                {
                    resume = [](WaiterBase& self, bool stop) {
                        auto& op = static_cast<OperationState&>(self);
                        if (stop) {
                            stdexec::set_stopped(std::move(op.receiver));
                        }
                        else {
                            stdexec::set_value(std::move(op.receiver));
                        }
                    };
                }

                auto start() noexcept -> void
                {
                    std::lock_guard _{latch->internalMutex};
                    if (latch->counter == 0) {
                        // Latch already reached zero, resume immediately
                        resume(*this, false);
                    }
                    else {
                        latch->PushResumeWaiter(*this);
                    }
                }
            };

        public:
            using completion_signatures =
                stdexec::completion_signatures<stdexec::set_value_t(), stdexec::set_stopped_t()>;

            explicit AsyncLatchWaitSender(AsyncLatch& latch) : latch(&latch)
            {
            }

            template <stdexec::receiver Receiver>
            auto connect(Receiver&& receiver) noexcept
            {
                return OperationState<Receiver>{*latch, std::forward<Receiver>(receiver)};
            }
        };

    public:
        AsyncLatch(uint64_t counter = 1) : counter(counter)
        {
        }
        AsyncLatch(const AsyncLatch&)                    = delete;
        AsyncLatch(AsyncLatch&&)                         = delete;
        auto operator=(const AsyncLatch&) -> AsyncLatch& = delete;
        auto operator=(AsyncLatch&&) -> AsyncLatch&      = delete;

        // User must ensure no waiters are present when destroying the latch
        ~AsyncLatch()
        {
#if defined(GLSLD_DEBUG)
            std::lock_guard _{internalMutex};
            GLSLD_ASSERT(signalResumeHead == nullptr && signalResumeTail == nullptr);
#endif
        }

        auto CountDown() noexcept -> void
        {
            std::lock_guard _{internalMutex};
            if (counter > 0) {
                --counter;
            }

            if (counter == 0) {
                while (auto resumeWaiter = PopResumeWaiter()) {
                    // Resume all waiters
                    resumeWaiter->resume(*resumeWaiter, false);
                }
            }
        }

        [[nodiscard]] auto TryWait() noexcept -> bool
        {
            std::lock_guard _{internalMutex};
            return counter == 0;
        }

        [[nodiscard]] auto AsyncWait() noexcept
        {
            return AsyncLatchWaitSender{*this};
        }

        auto Wait() noexcept -> void
        {
            stdexec::sync_wait(AsyncWait());
        }

        // Cancel all waiters and resume them with stopped signal
        auto Clear() noexcept -> void
        {
            std::lock_guard _{internalMutex};
            while (auto resumeWaiter = PopResumeWaiter()) {
                // Resume all waiters with stopped signal
                resumeWaiter->resume(*resumeWaiter, true);
            }
        }
    };
} // namespace glsld
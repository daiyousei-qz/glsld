#pragma once
#include "Basic/Common.h"

#include <stdexec/execution.hpp>
#include <mutex>

// This isn't the most efficient async mutex implementation, but at least it's simple and works.
class AsyncMutex
{
private:
    // Internal mutex to protect the state.
    std::mutex internalMutex;

    struct WaiterBase
    {
        void (*resume)(WaiterBase& self, bool stop) = nullptr;
        WaiterBase* next                            = nullptr;
    };

    // Queue of operation states that are waiting to acquire lock.
    WaiterBase* signalResumeHead = nullptr;
    WaiterBase* signalResumeTail = nullptr;

    // The mutex is currently locked.
    bool locked{false};

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

    template <bool UseUniqueLock>
    class AsyncLockSender : public stdexec::sender_t
    {
    private:
        AsyncMutex* mutex;

        template <stdexec::receiver Receiver>
        class OperationState : public stdexec::operation_state_t, private WaiterBase
        {
        private:
            AsyncMutex* mutex;
            Receiver receiver;

        public:
            OperationState(AsyncMutex& mutex, Receiver&& rcvr) : mutex(&mutex), receiver(std::forward<Receiver>(rcvr))
            {
                resume = [](WaiterBase& self, bool stop) {
                    auto& op = static_cast<OperationState&>(self);
                    if (stop) {
                        stdexec::set_stopped(std::move(op.receiver));
                    }
                    else {
                        if constexpr (UseUniqueLock) {
                            stdexec::set_value(std::move(op.receiver),
                                               std::unique_lock<AsyncMutex>{*op.mutex, std::adopt_lock});
                        }
                        else {
                            stdexec::set_value(std::move(op.receiver));
                        }
                    }
                };
            }

            auto start() noexcept -> void
            {
                std::lock_guard _{mutex->internalMutex};
                if (!mutex->locked) {
                    mutex->locked = true;
                    resume(*this, false);
                }
                else {
                    mutex->PushResumeWaiter(*this);
                }
            }
        };

        using SetValue = std::conditional_t<UseUniqueLock, stdexec::set_value_t(std::unique_lock<AsyncMutex>),
                                            stdexec::set_value_t()>;

    public:
        using completion_signatures = stdexec::completion_signatures<SetValue, stdexec::set_stopped_t()>;

        explicit AsyncLockSender(AsyncMutex& mutex) : mutex(&mutex)
        {
        }

        template <stdexec::receiver Receiver>
        auto connect(Receiver&& receiver) noexcept
        {
            return OperationState<Receiver>{*mutex, std::forward<Receiver>(receiver)};
        }
    };

public:
    AsyncMutex()                  = default;
    AsyncMutex(const AsyncMutex&) = delete;
    AsyncMutex(AsyncMutex&&)      = delete;

    ~AsyncMutex()
    {
        std::lock_guard _{internalMutex};
        if (locked) {
            // User must ensure this mutex is unlocked before destruction
            GLSLD_ASSERT(false && "Destroying a locked mutex");
            std::terminate();
        }
    }

    [[nodiscard]] auto TryLock() noexcept -> bool
    {
        std::lock_guard _{internalMutex};
        return std::exchange(locked, true) == false;
    }

    [[nodiscard]] auto AsyncLock() noexcept
    {
        return AsyncLockSender<false>{*this};
    }

    [[nodiscard]] auto AsyncLetLock() noexcept
    {
        return AsyncLockSender<true>{*this};
    }

    auto Lock() noexcept -> void
    {
        stdexec::sync_wait(AsyncLock());
    }

    auto Unlock() noexcept -> void
    {
        std::lock_guard _{internalMutex};
        if (!locked) {
            // Unlocking an unlocked mutex
            GLSLD_ASSERT(false && "Unlocking an unlocked mutex");
            std::terminate();
        }

        if (auto resumeWaiter = PopResumeWaiter(); resumeWaiter) {
            // Wake up the next waiter without releasing the lock
            resumeWaiter->resume(*resumeWaiter, false);
        }
        else {
            // No one is waiting, just mark as unlocked
            locked = false;
        }
    }

    // Cancel all waiters and resume them with stopped signal
    // Notably, we still need wait until the current lock owner to release the lock
    auto Clear() noexcept -> void
    {
        std::lock_guard _{internalMutex};
        while (auto resumeWaiter = PopResumeWaiter()) {
            // Resume all waiters with stopped signal
            resumeWaiter->resume(*resumeWaiter, true);
        }
    }

    // For compatibility with BasicLockable concept
    auto lock() noexcept -> void
    {
        Lock();
    }

    // For compatibility with BasicLockable concept
    auto unlock() noexcept -> void
    {
        Unlock();
    }
};

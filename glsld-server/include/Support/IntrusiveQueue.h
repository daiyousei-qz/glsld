#pragma once

#include "Basic/Common.h"

#include <iterator>
#include <cstddef>

namespace glsld
{
    template <typename T, T* T::* NextPtr = &T::next>
    class IntrusiveQueue
    {
    private:
        T* head = nullptr;
        T* tail = nullptr;

    public:
        IntrusiveQueue() = default;

        IntrusiveQueue(const IntrusiveQueue&)            = delete;
        IntrusiveQueue& operator=(const IntrusiveQueue&) = delete;

        IntrusiveQueue(IntrusiveQueue&& other) noexcept : head(other.head), tail(other.tail)
        {
            other.head = nullptr;
            other.tail = nullptr;
        }

        IntrusiveQueue& operator=(IntrusiveQueue&& other) noexcept
        {
            if (this != &other) {
                head       = other.head;
                tail       = other.tail;
                other.head = nullptr;
                other.tail = nullptr;
            }
            return *this;
        }

        auto empty() const noexcept -> bool
        {
            return head == nullptr;
        }

        auto push(T* value) noexcept -> void
        {
            value->*NextPtr = nullptr;
            if (tail) {
                tail->*NextPtr = value;
            }
            else {
                head = value;
            }
            tail = value;
        }

        auto pop() noexcept -> T*
        {
            if (!head) {
                GLSLD_ASSERT(!tail);
                return nullptr;
            }
            T* value = head;
            head     = head->*NextPtr;
            if (!head) {
                tail = nullptr;
            }
            value->*NextPtr = nullptr;
            return value;
        }

        auto front() const noexcept -> T*
        {
            return head;
        }

        auto back() const noexcept -> T*
        {
            return tail;
        }

        class Iterator
        {
        private:
            T* ptr;

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = T;
            using difference_type   = std::ptrdiff_t;
            using pointer           = T*;
            using reference         = T&;

            explicit Iterator(T* ptr) : ptr(ptr)
            {
            }

            T& operator*() const noexcept
            {
                return *ptr;
            }
            T* operator->() const noexcept
            {
                return ptr;
            }

            Iterator& operator++() noexcept
            {
                ptr = ptr->*NextPtr;
                return *this;
            }

            auto operator==(const Iterator& other) const noexcept -> bool = default;
        };

        auto begin() const noexcept -> Iterator
        {
            return Iterator(head);
        }
        auto end() const noexcept -> Iterator
        {
            return Iterator(nullptr);
        }
    };
} // namespace glsld
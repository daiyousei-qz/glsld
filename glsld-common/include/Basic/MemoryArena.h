#pragma once
#include "Basic/Common.h"

#include <deque>

namespace glsld
{
    namespace detail
    {
        template <bool EnableAutomaticDestruct>
        class MemoryArenaBase
        {
        public:
            MemoryArenaBase() = default;

            MemoryArenaBase(const MemoryArenaBase&)            = delete;
            MemoryArenaBase& operator=(const MemoryArenaBase&) = delete;

            MemoryArenaBase(MemoryArenaBase&&)            = default;
            MemoryArenaBase& operator=(MemoryArenaBase&&) = default;
        };

        template <>
        class MemoryArenaBase<true>
        {
        private:
            struct DestructHandle
            {
                void (*destruct)(void*, size_t);
                void* ptr;
                size_t count;
            };

            std::deque<DestructHandle> destructHandles;

        public:
            MemoryArenaBase() = default;

            MemoryArenaBase(const MemoryArenaBase&)            = delete;
            MemoryArenaBase& operator=(const MemoryArenaBase&) = delete;

            MemoryArenaBase(MemoryArenaBase&& other)
            {
                *this = std::move(other);
            }
            MemoryArenaBase& operator=(MemoryArenaBase&& other)
            {
                destructHandles       = std::move(other.destructHandles);
                other.destructHandles = {};

                return *this;
            }

        protected:
            template <typename T>
            auto CreateDestructHandle(void* ptr, size_t count) -> void
            {
                auto destructHelper = [](void* ptr, size_t size) {
                    auto typedPtr = static_cast<T*>(ptr);
                    typedPtr->~T();
                };
                auto destructArrayHelper = [](void* ptr, size_t size) {
                    auto typedPtr = static_cast<T*>(ptr);
                    for (size_t i = 0; i < size; ++i) {
                        typedPtr[i].~T();
                    }
                };

                destructHandles.push_back(DestructHandle{
                    .destruct = count == 1 ? destructHelper : destructArrayHelper,
                    .ptr      = ptr,
                    .count    = count,
                });
            }

            auto DoAutoDestruct() -> void
            {
                for (const auto handle : destructHandles) {
                    handle.destruct(handle.ptr, handle.count);
                }
                destructHandles.clear();
            }
        };
    } // namespace detail

    // TODO: Support customizing alignment size.
    // A monotonic memory arena.
    // All allocations from this arena are guaranteed to be aligned to the `alignof(std::max_align_t)`.
    template <bool EnableAutomaticDestruct>
    class BasicMemoryArena final : detail::MemoryArenaBase<EnableAutomaticDestruct>
    {
    private:
        using BaseType = detail::MemoryArenaBase<EnableAutomaticDestruct>;

        static constexpr size_t ArenaAllocationAlignment = alignof(std::max_align_t);
        static constexpr size_t RegularPageSize          = 4096 * 2 - 128;
        static constexpr size_t LargeObjectThreshold     = 1024;

        struct alignas(ArenaAllocationAlignment) BufferPageHeader
        {
            BufferPageHeader* next;
            uint32_t size;
            uint32_t used;
        };

        // For small allocations, we obtain memory from a fixed-size page for each allocation.
        BufferPageHeader* regularPageHead = nullptr;

        // For large allocations, we use a per-object page for each allocation.
        BufferPageHeader* largePageHead = nullptr;

    public:
        BasicMemoryArena() = default;
        ~BasicMemoryArena()
        {
            Clear();
        }

        BasicMemoryArena(const BasicMemoryArena&)            = delete;
        BasicMemoryArena& operator=(const BasicMemoryArena&) = delete;

        BasicMemoryArena(BasicMemoryArena&& other)
        {
            *this = std::move(other);
        }
        BasicMemoryArena& operator=(BasicMemoryArena&& other)
        {
            static_cast<BaseType&>(*this) = std::move(other);

            regularPageHead       = other.regularPageHead;
            largePageHead         = other.largePageHead;
            other.regularPageHead = nullptr;
            other.largePageHead   = nullptr;

            return *this;
        }

        //
        auto Allocate(size_t size) -> void*
        {
            auto allocSize = AlignAllocationSize(size);
            if (allocSize <= LargeObjectThreshold) {
                return AllocateSmall(allocSize);
            }
            else {
                return AllocateLarge(allocSize);
            }
        }

        template <typename T, typename... TArgs>
            requires(!std::is_array_v<T>)
        auto Construct(TArgs&&... args) -> T*
        {
            static_assert(alignof(T) <= ArenaAllocationAlignment);

            void* ptr = Allocate(sizeof(T));
            if constexpr (!std::is_trivially_constructible_v<T, TArgs...>) {
                new (ptr) T(std::forward<TArgs>(args)...);
            }

            if constexpr (!std::is_trivially_destructible_v<T>) {
                static_assert(EnableAutomaticDestruct, "Arena doesn't support automatic destruct.");
                if constexpr (EnableAutomaticDestruct) {
                    BaseType::template CreateDestructHandle<T>(ptr, 1);
                }
            }

            return reinterpret_cast<T*>(ptr);
        }

        template <typename T>
            requires(std::is_array_v<T>)
        auto Construct(size_t count) -> std::remove_extent_t<T>*
        {
            using ElemType = std::remove_extent_t<T>;
            static_assert(alignof(ElemType) <= ArenaAllocationAlignment);

            void* ptr = Allocate(sizeof(ElemType) * count);
            if constexpr (!std::is_trivially_constructible_v<ElemType>) {
                for (size_t i = 0; i < count; ++i) {
                    new (static_cast<ElemType*>(ptr) + i) ElemType();
                }
            }

            if constexpr (!std::is_trivially_destructible_v<ElemType>) {
                static_assert(EnableAutomaticDestruct, "Arena doesn't support automatic destruct.");
                if constexpr (EnableAutomaticDestruct) {
                    BaseType::template CreateDestructHandle<T>(ptr, count);
                }
            }

            return reinterpret_cast<ElemType*>(ptr);
        }

    private:
        auto Clear() -> void
        {
            if constexpr (EnableAutomaticDestruct) {
                BaseType::DoAutoDestruct();
            }

            for (auto p = regularPageHead; p != nullptr;) {
                auto next = p->next;
                ReleaseBufferPage(p);
                p = next;
            }
            for (auto p = largePageHead; p != nullptr;) {
                auto next = p->next;
                ReleaseBufferPage(p);
                p = next;
            }

            regularPageHead = nullptr;
            largePageHead   = nullptr;
        }

        static auto CreateBufferPage(BufferPageHeader*& head, size_t storeSize) -> BufferPageHeader*
        {
            static_assert(ArenaAllocationAlignment <= __STDCPP_DEFAULT_NEW_ALIGNMENT__);
            auto bufferPtr = ::operator new(sizeof(BufferPageHeader) + storeSize);
            auto pageHeader =
                new (bufferPtr) BufferPageHeader{.next = head, .size = static_cast<uint32_t>(storeSize), .used = 0};
            head = pageHeader;

            return pageHeader;
        }

        static auto ReleaseBufferPage(BufferPageHeader* pageHeader) -> void
        {
            ::operator delete(pageHeader);
        }

        static auto GetBufferStorePtr(BufferPageHeader* pageHeader) -> std::byte*
        {
            return reinterpret_cast<std::byte*>(pageHeader) + sizeof(BufferPageHeader);
        }

        static auto AlignAllocationSize(size_t size) -> size_t
        {
            return (size + ArenaAllocationAlignment - 1) & ~(ArenaAllocationAlignment - 1);
        }

        auto AllocateSmall(size_t size) -> void*
        {
            GLSLD_ASSERT(size < LargeObjectThreshold && size % ArenaAllocationAlignment == 0);
            if (regularPageHead == nullptr || regularPageHead->used + size > regularPageHead->size) {
                CreateBufferPage(regularPageHead, RegularPageSize);
            }

            auto result = GetBufferStorePtr(regularPageHead) + regularPageHead->used;
            regularPageHead->used += static_cast<uint32_t>(size);
            return result;
        }

        auto AllocateLarge(size_t size) -> void*
        {
            auto pageHeader  = CreateBufferPage(largePageHead, size);
            pageHeader->used = static_cast<uint32_t>(size);

            return GetBufferStorePtr(pageHeader);
        }
    };

    using MemoryArena = BasicMemoryArena<true>;
} // namespace glsld
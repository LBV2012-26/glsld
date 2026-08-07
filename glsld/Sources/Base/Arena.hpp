#pragma once

#include <cstddef>
#include <vector>

namespace glsld {
    class Arena {
    public:
        Arena() noexcept;
        Arena(const Arena&) = delete;
        Arena(Arena&& other) noexcept;
        ~Arena();

        Arena& operator=(const Arena&) = delete;
        Arena& operator=(Arena&& other) noexcept;

        std::byte* Allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t));
        void Reset() noexcept;

    private:
        void FreeMemory() noexcept;
        void SwitchTo(std::size_t index) noexcept;
        std::byte* AllocateBlock() noexcept;

        inline static constexpr auto kBlockSize = 2uz * 1024 * 1024; // 2MiB

        std::vector<std::byte*> blocks_;
        std::size_t             current_;
        std::byte*              memory_;
        std::byte*              last_;
    };

    inline thread_local Arena* thread_local_arena = nullptr;
} // namespace glsld

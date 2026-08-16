#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <type_traits>
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

        template <typename Ty, typename... Types>
        [[nodiscard]] Ty* Construct(Types&&... args);

        void Reset() noexcept;

        std::string_view CopyString(std::string_view text);

        template <typename Ty>
        std::span<const Ty> CopySpan(std::span<const Ty> span);

    private:
        template <typename Ty>
        friend class ArenaAllocator;

        struct Block {
            std::byte*  memory{};
            std::size_t size{};
        };

        std::byte* Allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t));
        void FreeMemory() noexcept;
        void SwitchTo(std::size_t index) noexcept;
        Block AllocateBlock(std::size_t size) noexcept;

        inline static constexpr auto kBlockSize = 2uz * 1024 * 1024; // 2MiB

        std::vector<Block> blocks_;
        std::size_t        current_;
        std::byte*         memory_;
        std::byte*         last_;
    };

    template <typename Ty>
    class ArenaAllocator {
    public:
        using value_type                             = Ty;
        using size_type                              = std::size_t;
        using difference_type                        = std::ptrdiff_t;

        using propagate_on_container_copy_assignment = std::false_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap            = std::true_type;

        using is_always_equal                        = std::false_type;

        template <typename Uy>
        struct rebind {
            using other = ArenaAllocator<Uy>;
        };

        explicit ArenaAllocator(Arena& arena) noexcept;
        explicit ArenaAllocator(Arena* arena) noexcept;

        template <typename Uy>
        ArenaAllocator(const ArenaAllocator<Uy>& other) noexcept;

        [[nodiscard]] Ty* allocate(std::size_t size);
        void deallocate(Ty*, std::size_t) noexcept;
        constexpr std::size_t max_size() const noexcept;
        ArenaAllocator select_on_container_copy_construction() const noexcept;

        Arena* arena() const noexcept;

    private:
        Arena* arena_{ nullptr };
    };

    template <typename Ty, typename Uy>
    bool operator==(const ArenaAllocator<Ty>& lhs, const ArenaAllocator<Uy>& rhs) noexcept;

    template <typename Ty>
    using ArenaVector = std::vector<Ty, ArenaAllocator<Ty>>;
} // namespace glsld

#include "Arena.inl"

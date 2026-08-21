#include "Arena.hpp"

#include <limits>
#include <memory>
#include <new>
#include <utility>

namespace glsld {
    template <typename Ty, typename... Types>
    Ty* Arena::Construct(Types&&... args) {
        static_assert(!std::is_array_v<Ty>);
        auto* memory = Allocate(sizeof(Ty), alignof(Ty));
        return std::construct_at(reinterpret_cast<Ty*>(memory), std::forward<Types>(args)...);
    }

    template <typename Ty>
    std::span<const Ty> Arena::CopySpan(std::span<const Ty> span) {
        using RawTy = std::remove_cv_t<Ty>;
        static_assert(std::is_trivially_destructible_v<RawTy>);

        if (span.empty()) {
            return {};
        }

        auto* memory    = Allocate(span.size() * sizeof(Ty), alignof(Ty));
        auto  dest_span = std::span<RawTy>(reinterpret_cast<RawTy*>(memory), span.size());

        std::ranges::uninitialized_copy(span, dest_span);
        return std::span<const Ty>(reinterpret_cast<Ty*>(memory), span.size());
    }

    template <typename Ty>
    ArenaAllocator<Ty>::ArenaAllocator(Arena& arena) noexcept
        : arena_{ std::addressof(arena) }
    {}

    template <typename Ty>
    ArenaAllocator<Ty>::ArenaAllocator(Arena* arena) noexcept
        : arena_{ arena }
    {}

    template <typename Ty>
    template <typename Uy>
    ArenaAllocator<Ty>::ArenaAllocator(const ArenaAllocator<Uy>& other) noexcept
        : arena_{ other.arena() }
    {}

    template <typename Ty>
    Ty* ArenaAllocator<Ty>::allocate(std::size_t size) {
        if (size == 0) {
            return nullptr;
        }

        if (arena_ == nullptr) {
            throw std::bad_alloc{};
        }

        if (size > max_size()) {
            throw std::bad_array_new_length{};
        }

        auto* memory = arena_->Allocate(size * sizeof(Ty), alignof(Ty));
        return reinterpret_cast<Ty*>(memory);
    }

    template <typename Ty>
    void ArenaAllocator<Ty>::deallocate(Ty*, std::size_t) noexcept {
        // do nothing
    }

    template <typename Ty>
    constexpr std::size_t ArenaAllocator<Ty>::max_size() const noexcept {
        return std::numeric_limits<std::size_t>::max() / sizeof(Ty);
    }

    template <typename Ty>
    ArenaAllocator<Ty> ArenaAllocator<Ty>::select_on_container_copy_construction() const noexcept {
        return *this;
    }

    template <typename Ty>
    Arena* ArenaAllocator<Ty>::arena() const noexcept {
        return arena_;
    }

    template <typename Ty, typename Uy>
    bool operator==(const ArenaAllocator<Ty>& lhs, const ArenaAllocator<Uy>& rhs) noexcept {
        return lhs.arena() == rhs.arena();
    }
}

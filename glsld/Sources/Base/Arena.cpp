#include "pch.hpp"
#include "Arena.hpp"

#include <cstdint>
#include <algorithm>
#include <bit>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#ifdef _WIN64
#include <Windows.h>
#else
#include <csignal>
#include <sys/mman.h>
#endif

namespace glsld {
    Arena::Arena() noexcept {
        blocks_.push_back(AllocateBlock(kDefaultBlockSize));
        SwitchTo(0);
    }

    Arena::Arena(Arena&& other) noexcept
        : blocks_{ std::move(other.blocks_) }
        , current_{ std::exchange(other.current_, 0uz) }
        , memory_{ std::exchange(other.memory_, nullptr) }
        , last_{ std::exchange(other.last_, nullptr) }
    {}

    Arena::~Arena() {
        FreeMemory();
    }

    Arena& Arena::operator=(Arena&& other) noexcept {
        if (this != &other) {
            FreeMemory();

            blocks_  = std::move(other.blocks_);
            current_ = std::exchange(other.current_, 0uz);
            memory_  = std::exchange(other.memory_, nullptr);
            last_    = std::exchange(other.last_, nullptr);
        }

        return *this;
    }

    void Arena::Reset() noexcept {
        if (!blocks_.empty()) {
            SwitchTo(0);
        }
    }

    std::string_view Arena::CopyString(std::string_view text) {
        if (text.empty()) {
            return {};
        }

        auto* memory = reinterpret_cast<char*>(Allocate(text.size(), alignof(char)));
        std::ranges::copy(text, memory);
        return std::string_view(memory, text.size());
    }

    std::byte* Arena::Allocate(std::size_t size, std::size_t alignment) {
        if (size == 0) {
            return nullptr;
        }

        if (alignment == 0 || !std::has_single_bit(alignment)) {
            throw std::bad_alloc{};
        }

        auto TryAllocateCurrent = [&]() -> std::byte* {
            void* memory    = memory_;
            auto  available = static_cast<std::size_t>(last_ - memory_);

            if (std::align(alignment, size, memory, available) == nullptr) {
                return nullptr;
            }

            auto* result = static_cast<std::byte*>(memory);
            memory_ = result + size;
            return result;
        };

        if (auto* memory = TryAllocateCurrent()) {
            return memory;
        }

        for (auto i = current_ + 1; i != blocks_.size(); ++i) {
            SwitchTo(i);
            if (auto* memory = TryAllocateCurrent()) {
                return memory;
            }
        }

        const auto padding = alignment - 1;
        if (size > std::numeric_limits<std::size_t>::max() - padding) {
            throw std::bad_alloc{};
        }

        const auto required_size = size + padding;
        const auto block_size    = std::max(kDefaultBlockSize, required_size);

        blocks_.push_back(AllocateBlock(block_size));
        SwitchTo(blocks_.size() - 1);

        auto* memory = TryAllocateCurrent();
        if (memory == nullptr) {
            throw std::bad_alloc{};
        }

        return memory;
    }

    void Arena::FreeMemory() noexcept {
        for (const auto& block : blocks_) {
            if (block.memory != nullptr) {
#ifdef _WIN64
                VirtualFree(block.memory, 0, MEM_RELEASE);
#else
                munmap(block.memory, block.size);
#endif
            }
        }
    }

    void Arena::SwitchTo(std::size_t index) noexcept {
        current_ = index;

        const auto& block = blocks_[index];
        memory_ = block.memory;
        last_   = block.memory + block.size;
    }

    Arena::Block Arena::AllocateBlock(std::size_t size) noexcept {
#ifdef _WIN64
        auto* memory = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (memory == nullptr) {
            __fastfail(1);
        }
#else
        auto* memory = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (memory == MAP_FAILED) {
            std::raise(SIGKILL);
        }
#endif

        return {
            .memory = static_cast<std::byte*>(memory),
            .size   = size
        };
    }
}

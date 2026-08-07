#include "pch.hpp"
#include "Arena.hpp"

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <utility>

#ifdef _WIN64
#include <Windows.h>
#else
#include <csignal>
#include <sys/mman.h>
#endif

namespace glsld {
    Arena::Arena() noexcept {
        blocks_.push_back(AllocateBlock());
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

    namespace {
        std::byte* AlignUp(std::byte* memory, std::size_t alignment) {
            auto address = reinterpret_cast<std::uintptr_t>(memory);
            address = (address + alignment - 1) & ~(alignment - 1);
            return reinterpret_cast<std::byte*>(address);
        };
    }

    std::byte* Arena::Allocate(std::size_t size, std::size_t alignment) {
        auto aligned = AlignUp(memory_, alignment);

        if (reinterpret_cast<std::uintptr_t>(aligned) + size > reinterpret_cast<std::uintptr_t>(last_)) {
            auto next = current_ + 1;
            if (next < blocks_.size()) {
                SwitchTo(next);
            } else {
                blocks_.push_back(AllocateBlock());
                SwitchTo(blocks_.size() - 1);
            }

            aligned = AlignUp(memory_, alignment);
        }

        memory_ = aligned + size;
        return aligned;
    }

    void Arena::Reset() noexcept {
        if (!blocks_.empty()) {
            SwitchTo(0);
        }
    }

    void Arena::FreeMemory() noexcept {
        for (auto* memory : blocks_) {
            if (memory != nullptr) {
#ifdef _WIN64
                VirtualFree(memory, 0, MEM_RELEASE);
#else
                mumap(memory, kBlockSize);
#endif
            }
        }
    }

    void Arena::SwitchTo(std::size_t index) noexcept {
        current_ = index;
        memory_  = blocks_[index];
        last_    = memory_ + kBlockSize;
    }

    std::byte* Arena::AllocateBlock() noexcept {
#ifdef _WIN64
        auto* memory = VirtualAlloc(nullptr, kBlockSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (memory == nullptr) {
            __fastfail(1);
        }
#else
        auto* memory = mmap(nullptr, kBlockSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (memory == MAP_FAILED) {
            std::raise(SIGKILL);
        }
#endif

        return static_cast<std::byte*>(memory);
    }
}

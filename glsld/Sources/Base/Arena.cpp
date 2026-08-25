#include "pch.hpp"
#include "Arena.hpp"

#include <cstdint>
#include <algorithm>
#include <bit>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <stdexcept>
#include <stop_token>
#include <utility>

#ifdef _WIN64
#include <Windows.h>
#else
#include <csignal>
#include <sys/mman.h>
#endif

#include "Base/Logger.hpp"

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

    std::size_t Arena::size() const noexcept {
        if (blocks_.empty()) {
            return 0;
        }

        auto total_size = 0uz;
        for (auto i = 0uz; i != current_; ++i) {
            total_size += blocks_[i].size;
        }

        const auto& current_block = blocks_[current_];
        total_size += static_cast<std::size_t>(memory_ - current_block.memory);

        return total_size;
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

    namespace {
        void InterruptibleSleep(std::stop_token stop_token, std::chrono::milliseconds duration) {
            std::mutex mutex;
            std::condition_variable_any condition;
            std::unique_lock lock(mutex);
            condition.wait_for(lock, stop_token, duration, []() -> bool { return false; });
        }
    }

    ArenaPool::ArenaPool()
        : recycle_thread_{
            [state = state_](std::stop_token stop_token) -> void {
                while (!stop_token.stop_requested()) {
                    InterruptibleSleep(stop_token, std::chrono::seconds(10));
                    std::lock_guard lock(state->mutex);

                    if (state->idle.empty()) {
                        continue;
                    }

                    std::erase_if(state->idle, [](const auto& arena) -> bool {
                        static constexpr auto kMaxIdleCapacity = 4uz * 1024 * 1024; // 4MiB
                        const auto capacity = arena->capacity();
                        if (capacity > kMaxIdleCapacity) {
                            GLSLD_LOG(info, "ArenaPool::recycle_thread_: recycling arena with capacity {} MiB", capacity / (1024 * 1024));
                            return true;
                        }

                        return false;
                    });

                    if (state->idle.size() > 3) {
                        GLSLD_LOG(info, "ArenaPool::recycle_thread_: reducing idle arenas from {} to 3", state->idle.size());
                        state->idle.resize(3);
                    }

                    for (const auto& arena : state->idle) {
                        GLSLD_LOG(info, "ArenaPool::recycle_thread_: remaining idle arena capacity: {} MiB", arena->capacity() / (1024 * 1024));
                    }
                }
            }
        }
    {
    }

    ArenaPool::~ArenaPool() {
        recycle_thread_.request_stop();
    }

    ArenaPool::Lease ArenaPool::Acquire() {
        std::unique_ptr<Arena> arena;
        {
            std::lock_guard lock(state_->mutex);
            if (!state_->idle.empty()) {
                arena = std::move(state_->idle.back());
                state_->idle.pop_back();
            }
        }

        if (arena == nullptr) {
            arena = std::make_unique<Arena>();
        }

        auto Recycle = [state = state_](Arena* arena) -> void {
            arena->Reset();
            std::lock_guard lock(state->mutex);
            state->idle.emplace_back(arena);
        };

        return Lease(arena.release(), Recycle);
    }
}

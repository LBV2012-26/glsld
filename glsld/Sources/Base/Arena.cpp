#include "pch.hpp"
#include "Arena.hpp"

#include <cstdint>
#include <algorithm>
#include <utility>
#include <Windows.h>

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
        for (auto* memory : blocks_) {
            if (memory != nullptr) {
                VirtualFree(memory, 0, MEM_RELEASE);
            }
        }
    }

    Arena& Arena::operator=(Arena&& other) noexcept {
        if (this != &other) {
            for (auto* memory : blocks_) {
                if (memory != nullptr) {
                    VirtualFree(memory, 0, MEM_RELEASE);
                }
            }

            blocks_  = std::move(other.blocks_);
            current_ = std::exchange(other.current_, 0uz);
            memory_  = std::exchange(other.memory_, nullptr);
            last_    = std::exchange(other.last_, nullptr);
        }

        return *this;
    }

    std::byte* Arena::Allocate(std::size_t size, std::size_t alignment) {
        auto address = (reinterpret_cast<std::uintptr_t>(memory_) + alignment - 1) & ~(alignment - 1);

        if (address + size > reinterpret_cast<std::uintptr_t>(last_)) {
            auto next = current_ + 1;
            if (next < blocks_.size()) {
                SwitchTo(next);
            } else {
                blocks_.push_back(AllocateBlock());
                SwitchTo(blocks_.size() - 1);
            }

            auto* memory = memory_;
            memory_ += size;
            return memory;
        }

        memory_ = reinterpret_cast<std::byte*>(address);
        auto* memory = memory_;
        memory_ += size;
        return memory;
    }

    void Arena::Reset() noexcept {
        if (!blocks_.empty()) {
            SwitchTo(0);
        }
    }

    void Arena::SwitchTo(std::size_t index) noexcept {
        current_ = index;
        memory_  = blocks_[index];
        last_    = memory_ + kBlockSize;
    }

    bool Arena::EnableLockMemoryPrivilege() noexcept {
        HANDLE token = nullptr;
        bool success = false;

        // if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        //     TOKEN_PRIVILEGES token_privileges{};
        //     LUID luid{};
        // 
        //     if (LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &luid)) {
        //         token_privileges.PrivilegeCount           = 1;
        //         token_privileges.Privileges[0].Luid       = luid;
        //         token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        //         AdjustTokenPrivileges(token, FALSE, &token_privileges, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr);
        // 
        //         if (GetLastError() == ERROR_SUCCESS) {
        //             success = true;
        //         }
        //     }
        // 
        //     CloseHandle(token);
        // }

        return success;
    }

    std::byte* Arena::AllocateBlock() noexcept {
        auto* memory = VirtualAlloc(nullptr, kBlockSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (memory == nullptr) {
            __fastfail(1);
        }

        return static_cast<std::byte*>(memory);
    }
}

#pragma once

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace glsld {
    class ThreadPool {
    public:
        explicit ThreadPool(std::uint32_t max_thread_count = 0);
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&)      = delete;
        ~ThreadPool();

        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&)      = delete;

        template <typename Func, typename... Types>
        auto Submit(Func&& pred, Types&&... args);

        std::uint32_t max_thread_count() const;

    private:
        struct Worker {
            std::queue<std::function<void()>> tasks;
            std::condition_variable_any       condition;
            std::mutex                        mutex;
        };

        std::vector<std::unique_ptr<Worker>> workers_;
        std::vector<std::jthread>            threads_;
        std::atomic<std::size_t>             next_thread_index_{};
        std::uint32_t                        max_thread_count_;
    };
}

#include "ThreadPool.inl"

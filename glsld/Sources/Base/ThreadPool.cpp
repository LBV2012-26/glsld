#include "stdafx.h"
#include "ThreadPool.hpp"

#include <cstdint>
#include <algorithm>
#include <stop_token>
#include <utility>

namespace glsld {
    ThreadPool::ThreadPool(std::uint32_t max_thread_count)
        : max_thread_count_{ std::clamp(max_thread_count, 1u, std::thread::hardware_concurrency()) }
    {
        workers_.reserve(max_thread_count_);
        threads_.reserve(max_thread_count_);

        for (auto i = 0uz; i != max_thread_count_; ++i) {
            workers_.emplace_back(std::make_unique<Worker>());
        }

        for (auto i = 0uz; i != max_thread_count_; ++i) {
            threads_.emplace_back([this, i](std::stop_token stop_token) -> void {
                Worker& worker = *workers_[i];
                while (!stop_token.stop_requested()) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(worker.mutex);
                        worker.condition.wait(lock, stop_token, [&worker]() -> bool {
                            return !worker.tasks.empty();
                        });

                        if (stop_token.stop_requested() && worker.tasks.empty()) {
                            return;
                        }

                        task = std::move(worker.tasks.front());
                        worker.tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    ThreadPool::~ThreadPool() {
        for (auto& thread : threads_) {
            thread.request_stop();
        }

        for (auto& worker : workers_) {
            std::lock_guard lock(worker->mutex);
            worker->condition.notify_one();
        }
    }
}

#include "stdafx.h"
#include "ThreadPool.hpp"

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <mutex>
#include <utility>

namespace glsld {
    ThreadPool::ThreadPool(int max_thread_count)
        : max_thread_count_{ std::clamp(max_thread_count, 0, static_cast<int>(std::thread::hardware_concurrency())) }
    {
        workers_.reserve(max_thread_count_);
        threads_.reserve(max_thread_count_);

        for (std::size_t i = 0; i != max_thread_count_; ++i) {
            workers_.emplace_back(std::make_unique<Worker>());
        }

        for (std::size_t i = 0; i != max_thread_count_; ++i) {
            threads_.emplace_back([this, i]() -> void {
                Worker& worker = *workers_[i];
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(worker.mutex);
                        worker.condition.wait(lock, [this, &worker]() -> bool {
                            return !worker.tasks.empty() || terminate_;
                        });

                        if (terminate_ && worker.tasks.empty()) {
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
        terminate_.store(true);
        for (auto& worker : workers_) {
            worker->condition.notify_one();
        }
    }
}

#include "ThreadPool.hpp"

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <type_traits>
#include <utility>

namespace glsld {
    struct ThreadPool::Worker {
        std::queue<std::function<void()>> tasks;
        std::condition_variable           condition;
        std::mutex                        mutex;
    };

    template <typename Func, typename... Types>
    auto ThreadPool::Submit(Func&& pred, Types&&... args) {
        using ReturnType = std::invoke_result_t<Func, Types...>;

        if (threads_.empty()) {
            return std::future<ReturnType>();
        }

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        [pred = std::forward<Func>(pred), ...args = std::forward<Types>(args)]() mutable -> ReturnType {
            return std::invoke(std::move(pred), std::move(args)...);
        });
        std::future<ReturnType> future = task->get_future();

        std::size_t thread_index = next_thread_index_.fetch_add(1) % max_thread_count_;
        auto& target_worker = workers_[thread_index];
        {
            std::unique_lock<std::mutex> lock(target_worker->mutex);
            target_worker->tasks.push([task]() -> void { (*task)(); });
        }
        target_worker->condition.notify_one();

        return future;
    }

    inline std::uint32_t ThreadPool::max_thread_count() const {
        return max_thread_count_;
    }
}

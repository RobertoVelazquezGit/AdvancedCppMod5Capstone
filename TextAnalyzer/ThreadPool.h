/**
 * @file ThreadPool.h
 * @brief Declares a fixed-size thread pool and its generic task submission API.
 */
#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

/**
 * @brief Executes queued tasks on a fixed number of worker threads.
 * @details Workers share a synchronized FIFO queue. Tasks may finish in a
 * different order from submission. Each submitted task provides a future
 * through which its result or exception can be retrieved.
 * @note Concurrent enqueue() calls are supported while the pool is alive.
 * The owner must prevent calls from overlapping destruction and must not
 * destroy the pool from one of its own worker threads.
 */
class ThreadPool {
public:
    /**
     * @brief Creates and starts the worker threads.
     * @param numThreads Number of workers to create; must be greater than zero.
     * @throws std::invalid_argument If numThreads is zero.
     * @throws std::system_error If a worker thread cannot be started.
     * @throws std::bad_alloc If required storage cannot be allocated.
     * @details If worker creation fails, all workers already started are
     * stopped and joined before the exception is propagated.
     */
    ThreadPool(std::size_t numThreads);

    /**
     * @brief Finishes queued work and joins all worker threads.
     * @details Signals shutdown, wakes waiting workers, and blocks until all
     * accepted tasks complete and all workers exit. Tasks are not cancelled.
     * @pre No other thread may access the pool during destruction.
     * @pre Destruction must not run on one of this pool's worker threads.
     */
    ~ThreadPool();

    /**
     * @brief Enqueues a callable and returns a future for its result.
     *
     * @tparam F Callable type, deduced from f.
     * @tparam Args Argument types, deduced from args.
     * @param f Callable to execute once on a worker thread.
     * @param args Arguments to store for the deferred invocation.
     * @return A future associated with the task's result, including void.
     * @throws std::runtime_error If the shutdown flag is set.
     * @throws std::bad_alloc If task or queue storage cannot be allocated.
     * @details Function and arguments are stored by value and moved into the
     * invocation. Use std::ref/std::cref to explicitly pass references; the
     * referenced objects must outlive the task. Exceptions reach future::get().
     * Exceptions raised while storing the callable or arguments propagate
     * directly to the caller. Submission does not wait for task completion.
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>> {
        using return_type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

        // Store the callable and arguments until a worker executes them once.
        auto boundTask = [function = std::forward<F>(f),
                          arguments = std::make_tuple(std::forward<Args>(args)...)]() mutable -> return_type {
            return std::apply(std::move(function), std::move(arguments));
        };

        // shared_ptr makes the queue wrapper copyable even for move-only tasks.
        auto packaged = std::make_shared<std::packaged_task<return_type()>>(
            std::move(boundTask));
        auto future = packaged->get_future();

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (stop) {
                throw std::runtime_error("Cannot enqueue on a stopped ThreadPool");
            }
            tasks.emplace([packaged] { (*packaged)(); });
        }
        condition.notify_one();
        return future;
    }

private:
    std::vector<std::thread> workers; ///< Worker threads joined during shutdown.
    std::queue<std::function<void()>> tasks; ///< Pending tasks, guarded by queueMutex.

    std::mutex queueMutex; ///< Protects the task queue and shutdown flag.
    std::condition_variable condition; ///< Wakes workers for tasks or shutdown.
    bool stop; ///< Shutdown flag; workers exit once the queue is also empty.
};


/**
 * @file ThreadPool.cpp
 * @brief Implements worker startup, task dispatch, and orderly pool shutdown.
 */
#include "ThreadPool.h"

/**
 * @details Each worker waits for queued work or a shutdown request. It removes
 * one task under the queue lock and executes it after releasing that lock,
 * allowing other workers and producers to access the queue concurrently.
 */
ThreadPool::ThreadPool(std::size_t numThreads) : stop(false) {
    if (numThreads == 0) {
        throw std::invalid_argument("ThreadPool requires at least one worker");
    }

    workers.reserve(numThreads);
    try {
        for (std::size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });

                        if (stop && tasks.empty()) {
                            return;
                        }

                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    // Execute outside the lock so other workers can run.
                    task();
                }
            });
        }
    }
    catch (...) {
        // A failed thread creation must not leave joinable threads behind.
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
        throw;
    }
}

/**
 * @details The shutdown flag is updated under the same mutex used by workers.
 * All workers are notified, and joining them keeps the pool's synchronization
 * objects alive until queued and running tasks have finished.
 */
ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    // Workers drain the queue before exiting.
    for (std::thread& worker : workers) {
        worker.join();
    }
}

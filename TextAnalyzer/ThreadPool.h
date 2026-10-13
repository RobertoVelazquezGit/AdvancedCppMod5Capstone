#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

// Thread pool for concurrent processing
class ThreadPool {
public:
    ThreadPool(std::size_t numThreads);

    ~ThreadPool();

    /**
     * Enqueues a task to be executed by the thread pool.
     *
     * @tparam F Function type
     * @tparam Args Argument types
     * @param f Function to execute
     * @param args Arguments to pass to the function
     * @return A future containing the result of the function
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        // TODO: Implement task enqueuing
        // 1. Create a packaged task with the function and arguments
        // 2. Get the future from the task
        // 3. Add the task to the queue
        // 4. Notify a worker thread

        // Placeholder to make the compiler happy
        std::promise<return_type> promise;
        return promise.get_future();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};


//class ThreadPool {
//public:
//    ThreadPool(size_t numThreads) {
//        for (size_t i = 0; i < numThreads; ++i) {
//            workers.emplace_back([this] {
//                while (true) {
//                    std::function<void()> task;
//
//                    {
//                        std::unique_lock<std::mutex> lock(queueMutex);
//                        condition.wait(lock, [this] {
//                            return stop || !tasks.empty();
//                            });
//
//                        if (stop && tasks.empty()) {
//                            return;
//                        }
//
//                        task = std::move(tasks.front());
//                        tasks.pop();
//                    }
//
//                    task();
//                }
//                });
//        }
//    }
//
//    template<class F>
//    void enqueue(F&& f) {
//        {
//            std::unique_lock<std::mutex> lock(queueMutex);
//            tasks.emplace(std::forward<F>(f));
//        }
//        condition.notify_one();
//    }
//
//    ~ThreadPool() {
//        {
//            std::unique_lock<std::mutex> lock(queueMutex);
//            stop = true;
//        }
//        condition.notify_all();
//        for (std::thread& worker : workers) {
//            worker.join();
//        }
//    }
//
//private:
//    std::vector<std::thread> workers;
//    std::queue<std::function<void()>> tasks;
//
//    std::mutex queueMutex;
//    std::condition_variable condition;
//    bool stop = false;
//};
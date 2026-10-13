#include "ThreadPool.h"

ThreadPool::ThreadPool(std::size_t numThreads) : stop(false) {
    // TODO: Implement thread pool constructor
    // 1. Start worker threads
    // 2. Set up task queue
}

ThreadPool::~ThreadPool() {
    // TODO: Implement thread pool destructor
    // 1. Set stop flag
    // 2. Notify all threads
    // 3. Join all threads
}

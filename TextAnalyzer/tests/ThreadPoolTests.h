/**
 * @file ThreadPoolTests.h
 * @brief Declares the optional thread pool test suite.
 */
#pragma once
#include "../RunConfig.h"

#if TREADPOOL_RUN_TESTS
namespace Tests {
    /**
     * @brief Checks task results, ownership, concurrency, and orderly shutdown.
     * @details Runs independent cases covering invalid worker counts, idle
     * shutdown, argument storage, futures, move-only resources, task exceptions,
     * overlapping workers, queue draining, and concurrent producers.
     * Each case prints PASS or FAIL; caught failures do not stop later cases.
     * @return Zero if all cases pass, or one if any case fails.
     * @note Declared and compiled only when TREADPOOL_RUN_TESTS is enabled.
     * @see ThreadPool
     */
    int runThreadPoolTests();
}
#endif

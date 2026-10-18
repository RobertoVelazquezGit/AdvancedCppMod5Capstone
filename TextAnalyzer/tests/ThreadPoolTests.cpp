/**
 * @file ThreadPoolTests.cpp
 * @brief Tests futures, argument storage, worker concurrency, and shutdown.
 */
#include "../RunConfig.h"

#if TREADPOOL_RUN_TESTS
#include "ThreadPoolTests.h"
#include "../ThreadPool.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>

namespace {
    /**
     * @brief Checks an expectation in both Debug and Release builds.
     * @param condition Expected truth value.
     * @param message Diagnostic reported when the expectation fails.
     * @throws std::runtime_error If condition is false.
     */
    void check(bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    /**
     * @brief Provides a plain function for testing mixed argument types.
     * @param quantity Number of units.
     * @param price Price per unit.
     * @return The product of quantity and price as a double.
     */
    double calculate(int quantity, double price) { return quantity * price; }
}

/**
 * @brief Runs the optional thread pool suite and reports individual outcomes.
 * @details Uses runtime checks rather than assertions so expectations remain
 * active in Release builds. Synchronization handshakes use bounded waits to
 * detect missing overlap without comparing task execution times.
 * @return Zero when all cases pass, or one when any case fails.
 * @note Compiled only when TREADPOOL_RUN_TESTS is enabled.
 */
int Tests::runThreadPoolTests() {
    int passed = 0; ///< Number of cases completed successfully.
    int failed = 0; ///< Number of cases that raised an exception.
    std::cout << "ThreadPool tests\n";
    /**
     * @brief Executes a case and records its result without aborting the suite.
     * @param name Human-readable label printed with the outcome.
     * @param action Callable containing the test and its expectations.
     * @details Standard and unknown exceptions are reported as failures.
     */
    const auto run = [&](const char* name, auto action) {
        try {
            action();
            ++passed;
            std::cout << "[PASS] " << name << '\n';
        }
        catch (const std::exception& error) {
            ++failed;
            std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
        }
        catch (...) {
            ++failed;
            std::cerr << "[FAIL] " << name << ": unknown exception\n";
        }
    };

    /** @brief Verifies that a pool with no workers throws invalid_argument. */
    run("Reject zero workers", [] {
        bool rejected = false;
        try { ThreadPool pool(0); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Expected invalid_argument");
    });

    /** @brief Checks that idle workers wake and join when the pool is destroyed. */
    run("Empty pool shuts down", [] { ThreadPool pool(2); });

    /**
     * @brief Checks mixed parameters, a parameterless task, and future consumption.
     * @details Verifies double and string results and that get() invalidates
     * the consumed future's association with its shared state.
     */
    run("Mixed arguments and no-argument result", [] {
        ThreadPool pool(2);
        auto number = pool.enqueue(calculate, 3, 2.5);
        auto text = pool.enqueue([] { return std::string("hello"); });
        check(number.get() == 7.5, "Incorrect numeric result");
        check(text.get() == "hello", "Incorrect string result");
        check(!number.valid(), "get must consume the future state");
    });

    /**
     * @brief Checks void results and explicit mutable and constant references.
     * @details Waits through future::get() before inspecting the modified value.
     */
    run("Void tasks and explicit references", [] {
        int value = 0;
        const int increment = 7;
        ThreadPool pool(1);
        auto result = pool.enqueue([](int& n, const int& delta) { n += delta; },
                                   std::ref(value), std::cref(increment));
        result.get();
        check(value == 7, "Reference argument was not updated");
    });

    /**
     * @brief Verifies that ordinary lvalue arguments are stored as independent copies.
     * @details A gate delays result production until the caller has changed its
     * original string. The task must still return the value submitted earlier.
     */
    run("Arguments are copied at submission", [] {
        /** @brief Promise used to send a completion signal without a payload. */
        std::promise<void> release;
        /**
         * @brief Copyable std::shared_future<void> observing release's shared state.
         * @details get_future() obtains a std::future<void>; share() transfers
         * its state association into a shared_future. Copies of gate observe
         * the same state, rather than creating independent signals.
         */
        auto gate = release.get_future().share();
        ThreadPool pool(1);
        std::string input = "original";
        /**
         * @brief Submits a task with a by-value gate capture and a copied argument.
         * @details [gate] copies the shared_future into this lambda. That copy
         * remains connected to release's shared state. Separately, enqueue
         * stores a copy of input and later supplies it as the text parameter.
         * The returned result future carries the string produced by the task;
         * it is distinct from the gate future used only for synchronization.
         */
        auto result = pool.enqueue([gate](std::string text) {
            /**
             * @brief Waits for the signal or for the five-second timeout to expire.
             * @details Returns ready when release.set_value() makes the shared
             * state ready, or timeout if the signal does not arrive in time.
             * If already signaled, it returns ready immediately; five seconds
             * is not a mandatory delay. Scheduling may extend the actual wait.
             */
            if (gate.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
                throw std::runtime_error("Gate timed out");
            }
            return text;
        }, input);
        /** @brief Changes the caller's string after enqueue has stored its copy. */
        input = "changed";
        /**
         * @brief Signals the promise associated with the lambda's copied gate.
         * @details Marks their shared state as ready so the worker can continue.
         * The signal remains available even if the worker has not started
         * waiting yet; copying gate does not disconnect it from this promise.
         */
        release.set_value();
        /** @brief Retrieves the task result and verifies that the original copy survived. */
        check(result.get() == "original", "Task did not retain its own copy");
    });

    /**
     * @brief Checks ownership transfer for arguments, results, and mutable callables.
     * @details Uses unique_ptr to ensure these paths do not require copying.
     */
    run("Move-only arguments, callables, and results", [] {
        ThreadPool pool(1);
        auto argument = std::make_unique<int>(9);
        auto result = pool.enqueue([](std::unique_ptr<int> p) { return p; },
                                   std::move(argument));
        check(!argument, "Argument ownership was not transferred");
        check(*result.get() == 9, "Incorrect move-only result");
        auto callable = [p = std::make_unique<int>(10)]() mutable { return ++*p; };
        check(pool.enqueue(std::move(callable)).get() == 11,
              "Move-only mutable callable failed");
    });

    /**
     * @brief Checks exception propagation and continued use of the same worker.
     * @details The first future must rethrow the task's diagnostic; a subsequent
     * task on the single-worker pool must still produce its expected result.
     */
    run("Task exceptions reach futures and worker continues", [] {
        ThreadPool pool(1);
        auto result = pool.enqueue([]() -> int { throw std::runtime_error("task failed"); });
        bool caught = false;
        try { result.get(); }
        catch (const std::runtime_error& error) {
            caught = std::string(error.what()) == "task failed";
        }
        check(caught, "Original task exception was not propagated");
        check(pool.enqueue([] { return 42; }).get() == 42, "Worker did not continue");
    });

    /**
     * @brief Checks that two tasks can be active at the same time.
     * @details Each task announces its start and waits up to five seconds for
     * the other task. A serialized execution cannot satisfy both expectations.
     */
    run("Two workers execute concurrently", [] {
        // A handshake verifies overlap without relying on execution speed.
        std::promise<void> firstStarted, secondStarted;
        auto firstReady = firstStarted.get_future().share();
        auto secondReady = secondStarted.get_future().share();
        ThreadPool pool(2);
        auto first = pool.enqueue([&] {
            firstStarted.set_value();
            return secondReady.wait_for(std::chrono::seconds(5)) == std::future_status::ready;
        });
        auto second = pool.enqueue([&] {
            secondStarted.set_value();
            return firstReady.wait_for(std::chrono::seconds(5)) == std::future_status::ready;
        });
        const bool firstOverlapped = first.get();
        const bool secondOverlapped = second.get();
        check(firstOverlapped && secondOverlapped, "Workers did not overlap");
    });

    /**
     * @brief Checks completion of accepted work and future lifetime after shutdown.
     * @details Submits 100 tasks, destroys the pool before retrieving results,
     * then verifies the completion count, ready states, and every returned value.
     */
    run("Destructor drains queue and futures survive pool", [] {
        std::atomic<int> completed{0};
        std::vector<std::future<int>> results;
        {
            ThreadPool pool(1);
            for (int i = 0; i < 100; ++i) {
                results.push_back(pool.enqueue([&completed](int n) {
                    ++completed;
                    return n * n;
                }, i));
            }
        }
        check(completed == 100, "Destructor left unfinished work");
        for (int i = 0; i < 100; ++i) {
            check(results[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready,
                  "Future was not ready after destruction");
            check(results[i].get() == i * i, "Incorrect queued result");
        }
    });

    /**
     * @brief Checks concurrent submission without lost or duplicated tasks.
     * @details Four producers each submit 25 tasks to three workers. Separate
     * atomic counters record executions, and all producers finish before the
     * counters are inspected or their exceptions are propagated.
     */
    run("Concurrent producers execute each task once", [] {
        std::vector<std::atomic<int>> counts(100);
        for (auto& count : counts) { count.store(0); }
        ThreadPool pool(3);
        std::vector<std::future<void>> producers;
        for (int producer = 0; producer < 4; ++producer) {
            producers.push_back(std::async(std::launch::async, [&, producer] {
                std::vector<std::future<void>> results;
                for (int i = 0; i < 25; ++i) {
                    const int index = producer * 25 + i;
                    results.push_back(pool.enqueue([&, index] { ++counts[index]; }));
                }
                for (auto& result : results) { result.get(); }
            }));
        }
        // Wait for every producer before inspecting results or propagating errors.
        for (auto& producer : producers) { producer.wait(); }
        for (auto& producer : producers) { producer.get(); }
        for (auto& count : counts) { check(count == 1, "Task lost or executed twice"); }
    });

    std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
#endif

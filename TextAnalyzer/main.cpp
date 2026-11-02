/**
 * Advanced Text Analysis System
 * C++ Programming Fundamentals Capstone Project
 *
 * This file provides the starter code structure for implementing
 * a multi-threaded text analysis system using modern C++ features.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <future>
#include <queue>
#include <functional>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <regex>
#include <filesystem>
#include <optional>
#include <any>
#include <variant>

#include "FileHandling.h"
#include "RunConfig.h"
#include "TextAnalyzer.h"
#include "TextAnalysisSystem.h"

#if FILE_HANDLING_RUN_TESTS
#include "tests/FileHandlingTests.h"
#endif

#if WORDFREQUENCYANALYZER_RUN_TESTS
#include "tests/WordFrequencyAnalyzerTest.h"
#endif

#if READABILITYANALYZER_RUN_TESTS
#include "tests/ReadabilityAnalyzerTests.h"
#endif

#if SENTIMENTANALYZER_RUN_TESTS
#include "tests/SentimentAnalizerTest.h"
#endif

#if FACTORYANALYZER_RUN_TESTS
#include "tests/FactoryAnalyzerTests.h"
#endif

#if TREADPOOL_RUN_TESTS
#include "tests/ThreadPoolTests.h"
#endif

/**
 * @brief Runs enabled test suites or starts the text analysis application.
 * @param argc Number of command-line arguments, including the program name.
 * @param argv Arguments containing the input filename and optional thread count.
 * @details Test suites are selected at compile time through RunConfig.h macros.
 * If any suite is selected, all enabled suites run before main returns; normal
 * application startup and command-line parsing are skipped.
 * @return Zero if all selected suites succeed or normal startup completes;
 * one if a selected suite fails, arguments are missing, or startup throws.
 */
int main(int argc, char* argv[]) {
    int failures = 0; ///< Sum of suite failure statuses (each suite returns 0 or 1).
    bool testsSelected = false; ///< True once any compiled-in suite is selected.
/** @brief Selects and runs the FileHandling suite when its macro is enabled. */
#if FILE_HANDLING_RUN_TESTS
    testsSelected = true;
    failures += Tests::runFileHandlingTests();
#endif
    /** @brief Runs the word frequency suite when its macro is enabled. */
#if WORDFREQUENCYANALYZER_RUN_TESTS
    testsSelected = true;
    failures += Tests::runWordFrequencyAnalyzerTests();
#endif
    /** @brief Runs the readability suite when its macro is enabled. */
#if READABILITYANALYZER_RUN_TESTS
    testsSelected = true;
    failures += Tests::runReadabilityAnalyzerTests();
#endif
    /** @brief Runs the sentiment suite when its macro is enabled. */
#if SENTIMENTANALYZER_RUN_TESTS
    testsSelected = true;
    failures += Tests::runSentimentAnalyzerTests();
#endif
    /** @brief Runs the factory and engine suite when its macro is enabled. */
#if FACTORYANALYZER_RUN_TESTS
    testsSelected = true;
    failures += Tests::runFactoryAnalyzerTests();
#endif
    /**
     * @brief Returns the combined test status when at least one suite is selected.
     * @details Add each additional suite in its own macro-controlled block above
     * this check, set testsSelected, and accumulate its 0/1 result in failures.
     * With no suites selected, execution continues into the application below.
     */
#if TREADPOOL_RUN_TESTS
    // Run the optional thread pool suite.
    testsSelected = true;
    failures += Tests::runThreadPoolTests();
#endif
    if (testsSelected) {
        return failures == 0 ? 0 : 1;
    }

    try {
        // Parse command line arguments
        if (argc < 2) {
            std::cout << "Usage: " << argv[0] << " <filename> [num_threads]" << std::endl;
            return 1;
        }

        std::string filename = argv[1];
        size_t numThreads = (argc > 2) ? std::stoul(argv[2]) : std::thread::hardware_concurrency();

        // Create and run the text analysis system
        TextAnalysisSystem system(numThreads);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

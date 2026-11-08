/**
 * @file TextAnalysisSystemTests.h
 * @brief Declares the optional TextAnalysisSystem integration test suite.
 * @details The implementation and main.cpp call are selected through
 * TEXTANALYSISSYSTEM_RUN_TESTS in RunConfig.h.
 */
#pragma once

/** @brief Entry points for the application test suites. */
namespace Tests {
    /**
     * @brief Runs isolated text, file, and batch integration tests.
     * @details Exercises the public API with real analyzers and one or several
     * worker threads. Covers typed results, repeated calls, empty input, real
     * files, generated reports, filtering, overwrite, setup errors, and
     * continued processing after a report write failure.
     * Each case receives a separate temporary directory. Owned fixtures are
     * removed afterward, with destructor cleanup as a fallback on exceptions.
     * Reports individual outcomes and a pass/fail summary. Expected application
     * errors may also appear on standard error during successful test cases.
     * No command-line arguments or user-provided files are required.
     * @return Zero if all cases, setup, and checked cleanup succeed; one otherwise.
     * This is a suite status, not the number of failed cases.
     * @note Call this function only when TEXTANALYSISSYSTEM_RUN_TESTS is enabled;
     * the implementation is excluded from compilation when the macro is zero.
     */
    int runTextAnalysisSystemTests();
}

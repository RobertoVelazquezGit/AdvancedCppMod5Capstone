/**
 * @file FileHandlingTests.h
 * @brief Declares the optional FileHandling test suite entry point.
 */
#pragma once

/** @brief Entry points for the application's test suites. */
namespace Tests {
    /**
     * @brief Runs repeatable FileHandling tests in isolated temporary directories.
     * @details Covers reading, writing, overwriting, empty files, extension
     * filtering, and expected errors for missing files or directories. Reports
     * individual results and a summary to standard output/error. Each failed
     * case is caught so that the remaining cases can run.
     * @return Zero if all tests, setup, and checked cleanup succeed; one otherwise.
     * This is a suite status, not the number of failed test cases.
     * @note The implementation is compiled only when FILE_HANDLING_RUN_TESTS
     * is enabled in RunConfig.h. Callers must use the same conditional guard.
     */
    int runFileHandlingTests();
}

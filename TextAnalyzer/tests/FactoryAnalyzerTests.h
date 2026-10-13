/**
 * @file FactoryAnalyzerTests.h
 * @brief Declares the optional factory and analysis engine test suite.
 */
#pragma once

namespace Tests {
    /**
     * @brief Runs factory and ownership checks and reports each result.
     * @details Checks the three supported analyzer types, invalid type diagnostics,
     * independent object lifetimes, ownership transfer, replacement, and delegation.
     * Each case reports PASS or FAIL; a failed case does not stop later cases.
     * @return Zero if every check passes, or one if any check fails.
     * @see AnalyzerFactory
     * @see AnalysisEngine
     * @note Call only when FACTORYANALYZER_RUN_TESTS is enabled.
     */
    int runFactoryAnalyzerTests();
}

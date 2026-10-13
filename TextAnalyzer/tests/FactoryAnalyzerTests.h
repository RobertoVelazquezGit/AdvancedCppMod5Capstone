/**
 * @file FactoryAnalyzerTests.h
 * @brief Declares the optional factory and analysis engine test suite.
 */
#pragma once

namespace Tests {
    /**
     * @brief Runs factory and ownership checks and reports each result.
     * @return Zero if every check passes, or one if any check fails.
     * @note Call only when FACTORYANALYZER_RUN_TESTS is enabled.
     */
    int runFactoryAnalyzerTests();
}

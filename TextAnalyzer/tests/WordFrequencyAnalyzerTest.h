/**
 * @file WordFrequencyAnalyzerTest.h
 * @brief Declares the optional word frequency test suite.
 */
#pragma once

namespace Tests {
    /**
     * @brief Runs word frequency checks and reports each result.
     * @return Zero if every check passes, or one if any check fails.
     * @note Call only when WORDFREQUENCYANALYZER_RUN_TESTS is enabled.
     */
    int runWordFrequencyAnalyzerTests();
}

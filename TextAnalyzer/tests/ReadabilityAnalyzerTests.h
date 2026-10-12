/**
 * @file ReadabilityAnalyzerTests.h
 * @brief Declares the optional readability test suite.
 */
#pragma once

namespace Tests {
    /**
     * @brief Runs readability checks and reports each result.
     * @return Zero if every check passes, or one if any check fails.
     * @note Call only when READABILITYANALYZER_RUN_TESTS is enabled.
     */
    int runReadabilityAnalyzerTests();
}

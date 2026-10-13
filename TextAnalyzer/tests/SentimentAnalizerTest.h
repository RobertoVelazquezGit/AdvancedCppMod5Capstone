/**
 * @file SentimentAnalizerTest.h
 * @brief Declares the optional sentiment test suite.
 */
#pragma once

namespace Tests {
    /**
     * @brief Runs sentiment checks and reports each result.
     * @return Zero if every check passes, or one if any check fails.
     * @note Call only when SENTIMENTANALYZER_RUN_TESTS is enabled.
     */
    int runSentimentAnalyzerTests();
}

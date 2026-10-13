/**
 * @file SentimentAnalizerTest.h
 * @brief Declares the optional sentiment test suite.
 */
#pragma once

namespace Tests {
    /**
     * @brief Runs sentiment checks and reports each result.
     * @details Covers dictionary entries, normalization, neutral and repeated
     * words, score bounds, the std::any result type, input preservation,
     * independent calls, and the analyzer name. Results are printed to the
     * standard output and error streams; failures do not stop later checks.
     * @return Zero if every check passes, or one if any check fails.
     * @note Call only when SENTIMENTANALYZER_RUN_TESTS is enabled.
     */
    int runSentimentAnalyzerTests();
}

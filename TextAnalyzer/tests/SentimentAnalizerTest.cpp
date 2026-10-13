/**
 * @file SentimentAnalizerTest.cpp
 * @brief Tests sentiment scores, normalization, and the public analyzer contract.
 */
#include "../RunConfig.h"

#if SENTIMENTANALYZER_RUN_TESTS
#include "SentimentAnalizerTest.h"
#include "../SentimentAnalyzer.h"

#include <any>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    /**
     * @brief Checks an expectation in Debug and Release builds.
     * @param[in] condition Expectation that must evaluate to true.
     * @param[in] message Diagnostic used when the expectation fails.
     * @throws std::runtime_error If condition is false.
     */
    void check(bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    /**
     * @brief Checks a finite result with absolute error strictly below 1e-6.
     * @param[in] actual Computed sentiment score.
     * @param[in] expected Reference score for the test case.
     * @param[in] message Diagnostic used when the comparison fails.
     * @throws std::runtime_error If actual is non-finite or outside tolerance.
     */
    void checkNear(double actual, double expected, const char* message) {
        check(std::isfinite(actual) && std::abs(actual - expected) < 1e-6, message);
    }
}

/** @details Reports each failure without stopping the remaining cases. */
int Tests::runSentimentAnalyzerTests() {
    /// @brief Input and expected score for a named sentiment check.
    struct TestCase {
        const char* name; ///< Label displayed in the test report.
        std::string input; ///< Text passed to the analyzer.
        double expected; ///< Expected normalized sentiment score.
    };
    const TestCase cases[]{
        {"Empty input", "", 0.0},
        {"Whitespace only", " \t\r\n ", 0.0},
        {"Punctuation only", "!!! ... ,; --", 0.0},
        {"Neutral words", "the table is here", 0.0},
        {"Positive word", "good", 1.0},
        {"Negative word", "bad", -1.0},
        {"All positive dictionary entries",
            "good great excellent amazing wonderful happy love like fantastic awesome", 1.0},
        {"All negative dictionary entries",
            "bad terrible awful horrible poor sad hate dislike disappointing worst", -1.0},
        {"Balanced sentiment", "good bad", 0.0},
        {"Repeated positive words", "good good bad", 1.0 / 3.0},
        {"Repeated negative words", "bad bad good", -1.0 / 3.0},
        {"Neutral words do not dilute score", "good table bad chair great", 1.0 / 3.0},
        {"Case normalization", "GoOd GREAT bAd", 1.0 / 3.0},
        {"Surrounding punctuation", "(GOOD!), \"bad\"; great.", 1.0 / 3.0},
        {"Mixed whitespace", "good\tbad\r\ngreat", 1.0 / 3.0},
        {"Whole word matching", "goodness badly dislike", -1.0},
        {"Digits are preserved", "good123 bad456", 0.0}
    };

    int passed = 0;
    int failed = 0;
    std::cout << "SentimentAnalyzer tests\n";
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

    for (const auto& test : cases) {
        run(test.name, [&] {
            SentimentAnalyzer analyzer;
            std::string input = test.input;
            const auto result = analyzer.analyze(input);
            const auto* score = std::any_cast<double>(&result);
            check(score != nullptr, "Expected double in std::any");
            check(input == test.input, "Analysis must preserve the input");
            checkNear(*score, test.expected, "Unexpected sentiment score");
            check(*score >= -1.0 && *score <= 1.0, "Score must be in [-1.0, 1.0]");
        });
    }

    run("Independent calls", [] {
        SentimentAnalyzer analyzer;
        checkNear(std::any_cast<double>(analyzer.analyze("good good")), 1.0,
            "Unexpected first score");
        checkNear(std::any_cast<double>(analyzer.analyze("bad")), -1.0,
            "Scores must not persist between calls");
        checkNear(std::any_cast<double>(analyzer.analyze("")), 0.0,
            "Empty input must return zero after previous calls");
    });
    run("Analyzer name", [] {
        SentimentAnalyzer analyzer;
        check(analyzer.getName() == "Sentiment Analyzer", "Unexpected analyzer name");
    });

    std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
#endif

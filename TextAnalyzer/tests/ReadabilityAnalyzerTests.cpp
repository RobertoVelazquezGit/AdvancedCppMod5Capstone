/**
 * @file ReadabilityAnalyzerTests.cpp
 * @brief Tests readability counts, scores, and the public analyzer contract.
 */
#include "../RunConfig.h"

#if READABILITYANALYZER_RUN_TESTS
#include "ReadabilityAnalyzerTests.h"
#include "../ReadabilityAnalyzer.h"

#include <any>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    using Metrics = ReadabilityAnalyzer::ReadabilityMetrics;

    /// @brief Checks an expectation in Debug and Release builds.
    /// @param condition Expected truth value.
    /// @param message Diagnostic to report on failure.
    /// @throws std::runtime_error If the condition is false.
    void check(bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    /// @brief Checks a finite floating-point result within absolute tolerance.
    /// @param actual Computed value.
    /// @param expected Reference value.
    /// @param message Diagnostic to report on failure.
    void checkNear(double actual, double expected, const char* message) {
        check(std::isfinite(actual) && std::abs(actual - expected) < 1e-6, message);
    }
}

/**
 * @details Uses public analyze() results to exercise the private counting helpers.
 * Cases reflect the documented heuristics, including punctuation-only tokens and
 * abbreviation limitations. Each failure is reported without stopping later cases.
 */
int Tests::runReadabilityAnalyzerTests() {
    /// @brief Named input and independently specified expected counts.
    struct TestCase {
        const char* name;
        std::string input;
        int words;
        int sentences;
        int syllables;
    };
    const TestCase cases[]{
        {"Empty input", "", 0, 0, 0},
        {"Whitespace only", " \t\r\n ", 0, 0, 0},
        {"Punctuation only", "!!! ... ,; --", 4, 0, 0},
        {"Syllable accumulation", "The cat is sleeping", 4, 1, 5},
        {"Vowel groups", "reading boat", 2, 1, 3},
        {"Silent final e", "make home", 2, 1, 2},
        {"Consonant le", "table little", 2, 1, 4},
        {"Vowel before le", "sale", 1, 1, 1},
        {"Single vowel group", "the", 1, 1, 1},
        {"Y as vowel", "rhythm", 1, 1, 1},
        {"Minimum syllable count", "brr", 1, 1, 1},
        {"Case and punctuation", "(TABLE), MAKE!", 2, 1, 3},
        {"Mixed whitespace", "cat\treading\r\nmake", 3, 1, 4},
        {"Sentence endings", "Cat. Dog? Bird!", 3, 3, 3},
        {"Repeated terminal signs", "Cat?! Dog!!! Bird...", 3, 3, 3},
        {"Unterminated final sentence", "Cat. Dog", 2, 2, 2},
        {"Trailing quotation mark", "\"Cat.\"", 1, 1, 1},
        {"Digits without syllables", "123.", 1, 1, 0},
        {"Abbreviation heuristic", "Mr. Smith", 2, 2, 2},
        {"Decimal heuristic", "3.14", 1, 2, 0}
    };

    int passed = 0;
    int failed = 0;
    std::cout << "ReadabilityAnalyzer tests\n";
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
            ReadabilityAnalyzer analyzer;
            std::string input = test.input;
            const auto result = analyzer.analyze(input);
            const auto* metrics = std::any_cast<Metrics>(&result);
            check(metrics != nullptr, "Expected ReadabilityMetrics in std::any");
            check(input == test.input, "Analysis must preserve the input");
            check(metrics->totalWords == test.words, "Unexpected word count");
            check(metrics->totalSentences == test.sentences, "Unexpected sentence count");
            check(metrics->totalSyllables == test.syllables, "Unexpected syllable count");
            check(std::isfinite(metrics->fleschReadingEase) &&
                  std::isfinite(metrics->fleschKincaidGrade), "Scores must be finite");
            if (test.words == 0 || test.sentences == 0) {
                checkNear(metrics->averageSentenceLength, 0.0, "Expected zero sentence average");
                checkNear(metrics->averageWordLength, 0.0, "Expected zero syllable average");
                checkNear(metrics->fleschReadingEase, 0.0, "Expected zero reading ease");
                checkNear(metrics->fleschKincaidGrade, 0.0, "Expected zero grade level");
            }
        });
    }

    run("Reference readability scores", [] {
        ReadabilityAnalyzer analyzer;
        const auto metrics = std::any_cast<Metrics>(analyzer.analyze("The cat is sleeping."));
        checkNear(metrics.averageSentenceLength, 4.0, "Unexpected words per sentence");
        checkNear(metrics.averageWordLength, 1.25, "Unexpected syllables per word");
        checkNear(metrics.fleschReadingEase, 97.025, "Unexpected reading ease");
        checkNear(metrics.fleschKincaidGrade, 0.72, "Unexpected grade level");
    });
    run("Multiple sentence averages", [] {
        ReadabilityAnalyzer analyzer;
        const auto metrics = std::any_cast<Metrics>(analyzer.analyze("The cat. Is sleeping."));
        checkNear(metrics.averageSentenceLength, 2.0, "Unexpected words per sentence");
        checkNear(metrics.averageWordLength, 1.25, "Unexpected syllables per word");
        checkNear(metrics.fleschReadingEase, 99.055, "Unexpected reading ease");
        checkNear(metrics.fleschKincaidGrade, -0.06, "Negative grades must not be clamped");
    });
    run("Independent calls", [] {
        ReadabilityAnalyzer analyzer;
        analyzer.analyze("The cat is sleeping.");
        const auto second = std::any_cast<Metrics>(analyzer.analyze("Cat!"));
        check(second.totalWords == 1 && second.totalSentences == 1 &&
              second.totalSyllables == 1, "Counts must not persist between calls");
        const auto empty = std::any_cast<Metrics>(analyzer.analyze(""));
        check(empty.totalWords == 0 && empty.totalSentences == 0 &&
              empty.totalSyllables == 0, "Empty input must reset counts");
        checkNear(empty.fleschReadingEase, 0.0, "Empty input must reset scores");
    });
    run("Analyzer name", [] {
        ReadabilityAnalyzer analyzer;
        check(analyzer.getName() == "Readability Analyzer", "Unexpected analyzer name");
    });

    std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
#endif

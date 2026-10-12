/**
 * @file WordFrequencyAnalyzerTest.cpp
 * @brief Tests the public word frequency analyzer interface.
 */
#include "../RunConfig.h"

#if WORDFREQUENCYANALYZER_RUN_TESTS
#include "WordFrequencyAnalyzerTest.h"
#include "../WordFrequencyAnalyzer.h"

#include <any>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {
    using Counts = std::unordered_map<std::string, int>;

    /**
     * @brief Checks an expectation in both Debug and Release builds.
     * @param condition Expected truth value.
     * @param message Diagnostic to report on failure.
     * @throws std::runtime_error If the expectation is false.
     */
    void check(bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }
}

/**
 * @details Checks exact result types and counts, input preservation, independent
 * calls, and the display name. A failed case does not prevent later cases from
 * running. ASCII fixtures follow the current byte-based cleanup contract.
 */
int Tests::runWordFrequencyAnalyzerTests() {
    /** @brief Associates a named input with its complete expected result. */
    struct TestCase {
        const char* name;
        std::string input;
        Counts expected;
    };
    const TestCase cases[]{
        {"Empty input", "", {}},
        {"Whitespace only", " \t\r\n ", {}},
        {"Repeated words", "one two one", {{"one", 2}, {"two", 1}}},
        {"Mixed case", "Hello HELLO hello", {{"hello", 3}}},
        {"Surrounding punctuation", "(Hello), world!", {{"hello", 1}, {"world", 1}}},
        {"Punctuation only", "!!! ... ,; --", {}},
        {"Mixed whitespace", "one\ttwo\r\none", {{"one", 2}, {"two", 1}}},
        {"Internal punctuation", "can't co-operate", {{"cant", 1}, {"cooperate", 1}}},
        {"Digits retained", "123 abc123 123", {{"123", 2}, {"abc123", 1}}}
    };

    int passed = 0;
    int failed = 0;
    std::cout << "WordFrequencyAnalyzer tests\n";

    // Catch failures per case so the remaining checks still run.
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
            WordFrequencyAnalyzer analyzer;
            std::string input = test.input;
            const auto result = analyzer.analyze(input);
            // Passing &result selects the pointer overload: <Counts> is the stored type.
            // It returns a pointer to that value without copying, or nullptr if the type differs.
            const auto* counts = std::any_cast<Counts>(&result);
            check(counts != nullptr, "Expected unordered_map<string, int> in std::any");
            check(*counts == test.expected, "Unexpected word counts");
            check(input == test.input, "Analysis must preserve the input");
        });
    }

    run("Independent calls", [] {
        WordFrequencyAnalyzer analyzer;
        const auto first = std::any_cast<Counts>(analyzer.analyze("first first"));
        const auto second = std::any_cast<Counts>(analyzer.analyze("second"));
        check(first == Counts{{"first", 2}}, "Unexpected first result");
        check(second == Counts{{"second", 1}}, "Counts must not persist between calls");
        check(std::any_cast<Counts>(analyzer.analyze("")).empty(),
              "Empty input must not retain previous counts");
    });
    run("Analyzer name", [] {
        WordFrequencyAnalyzer analyzer;
        check(analyzer.getName() == "Word Frequency Analyzer", "Unexpected analyzer name");
    });

    std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
#endif

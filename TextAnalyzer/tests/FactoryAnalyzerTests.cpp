/**
 * @file FactoryAnalyzerTests.cpp
 * @brief Tests analyzer selection, ownership transfer, and engine delegation.
 */
#include "../RunConfig.h"

#if FACTORYANALYZER_RUN_TESTS
#include "FactoryAnalyzerTests.h"
#include "../FactoryAnalyzer.h"
#include "../ReadabilityAnalyzer.h"
#include "../SentimentAnalyzer.h"
#include "../WordFrequencyAnalyzer.h"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace {
    /**
     * @brief Checks expectations in both Debug and Release builds.
     * @param condition Expected truth value.
     * @param message Diagnostic used when the expectation fails.
     * @throws std::runtime_error If condition is false.
     */
    void check(bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    /** @brief Tracks destruction and echoes input to verify engine delegation. */
    class TrackingAnalyzer : public TextAnalyzer {
    public:
        /**
         * @brief Binds an external destruction counter.
         * @param[in,out] destroyed Counter incremented when this analyzer is destroyed.
         * @pre The counter must outlive this analyzer.
         */
        explicit TrackingAnalyzer(int& destroyed) : destroyed_(destroyed) {}
        /** @brief Records destruction through the virtual base-class interface. */
        ~TrackingAnalyzer() override { ++destroyed_; }
        /**
         * @brief Echoes the input to expose the engine's forwarding behavior.
         * @param[in] text Text to copy into the result without modification.
         * @return A std::any containing a copy of text as std::string.
         */
        std::any analyze(const std::string& text) override { return text; }
        /**
         * @brief Gets the test analyzer's display name.
         * @return The string "Tracking Analyzer".
         */
        std::string getName() const override { return "Tracking Analyzer"; }
    private:
        int& destroyed_; ///< External counter; must remain alive until destruction.
    };
}

/**
 * @brief Runs the optional factory and engine test suite.
 * @details Verifies concrete analyzer selection and usable results, rejected type
 * strings, independent ownership, and delegation through AnalysisEngine. A tracking
 * analyzer checks destruction on replacement and on engine scope exit.
 * @return Zero when all cases pass; one when any case fails.
 * @note Compiled only when FACTORYANALYZER_RUN_TESTS is enabled.
 */
int Tests::runFactoryAnalyzerTests() {
    int passed = 0; ///< Number of successful test cases.
    int failed = 0; ///< Number of test cases that threw an exception.
    std::cout << "FactoryAnalyzer tests\n";
    /**
     * @brief Executes one case, records its outcome, and lets later cases continue.
     * @param name Human-readable label printed with the outcome.
     * @param action Callable containing the test expectations.
     * @details Reports standard and unknown exceptions as test failures.
     */
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

    /** @brief Checks frequency selection and normalized word counts. */
    run("Frequency selection and analysis", [] {
        AnalyzerFactory factory;
        auto analyzer = factory.createAnalyzer("frequency");
        check(dynamic_cast<WordFrequencyAnalyzer*>(analyzer.get()) != nullptr,
              "Expected WordFrequencyAnalyzer");
        const auto result = std::any_cast<std::unordered_map<std::string, int>>(
            analyzer->analyze("Hello hello!"));
        check(result.size() == 1 && result.at("hello") == 2, "Unexpected word counts");
    });
    /** @brief Checks readability selection and basic text metrics. */
    run("Readability selection and analysis", [] {
        AnalyzerFactory factory;
        auto analyzer = factory.createAnalyzer("readability");
        check(dynamic_cast<ReadabilityAnalyzer*>(analyzer.get()) != nullptr,
              "Expected ReadabilityAnalyzer");
        const auto result = std::any_cast<ReadabilityAnalyzer::ReadabilityMetrics>(
            analyzer->analyze("Cat."));
        check(result.totalWords == 1 && result.totalSentences == 1 &&
              result.totalSyllables == 1, "Unexpected readability counts");
    });
    /** @brief Checks sentiment selection and a known positive score. */
    run("Sentiment selection and analysis", [] {
        AnalyzerFactory factory;
        auto analyzer = factory.createAnalyzer("sentiment");
        check(dynamic_cast<SentimentAnalyzer*>(analyzer.get()) != nullptr,
              "Expected SentimentAnalyzer");
        check(std::any_cast<double>(analyzer->analyze("good")) == 1.0,
              "Unexpected sentiment score");
    });
	/** @brief Checks rejection and diagnostics for unknown, empty, or mismatched types. */
    for (const std::string type : {"unknown", "", "Frequency", " frequency"}) {  // all of them are incorrect, the correct one is "frequency"
        const std::string name = "Reject invalid type: [" + type + "]";
        run(name.c_str(), [&] {
            AnalyzerFactory factory;
            bool thrown = false;
            try {
                factory.createAnalyzer(type);
            }
            catch (const std::runtime_error& error) {
                thrown = true;
                check(std::string(error.what()) == "Unknown analyzer type: " + type,
                      "Unexpected diagnostic");
            }
            check(thrown, "Expected std::runtime_error");
        });
    }
    /** @brief Checks that separate results survive the factory and each other. */
    run("Independent factory results outlive factory", [] {
        std::unique_ptr<TextAnalyzer> first;
        std::unique_ptr<TextAnalyzer> second;
        {
            AnalyzerFactory factory;
            first = factory.createAnalyzer("sentiment");
            second = factory.createAnalyzer("sentiment");
        }
        check(first && second && first.get() != second.get(), "Expected distinct objects");
        first.reset();
        check(std::any_cast<double>(second->analyze("good")) == 1.0,
              "Remaining analyzer must stay usable");
    });
    /** @brief Checks ownership transfer, exact input forwarding, and final destruction. */
    run("Engine takes ownership, delegates, and destroys analyzer", [] {
        int destroyed = 0;
        {
            AnalysisEngine engine;
            auto analyzer = std::make_unique<TrackingAnalyzer>(destroyed);
            engine.setAnalyzer(std::move(analyzer));
            check(!analyzer, "Source pointer must be empty after transfer");
            check(destroyed == 0, "Transferred analyzer must stay alive");
            check(std::any_cast<std::string>(engine.analyze("Exact input!")) == "Exact input!",
                  "Engine must forward input and result");
            check(std::any_cast<std::string>(engine.analyze("")).empty(),
                  "Engine must forward empty input");
        }
        check(destroyed == 1, "Engine must destroy its analyzer exactly once");
    });
    /** @brief Checks immediate destruction on replacement and use of the replacement. */
    run("Replacing analyzer releases previous owner", [] {
        int destroyed = 0;
        {
            AnalysisEngine engine;
            engine.setAnalyzer(std::make_unique<TrackingAnalyzer>(destroyed));
            engine.setAnalyzer(std::make_unique<TrackingAnalyzer>(destroyed));
            check(destroyed == 1, "Previous analyzer must be destroyed on replacement");
            check(std::any_cast<std::string>(engine.analyze("replacement")) == "replacement",
                  "Replacement analyzer must be used");
        }
        check(destroyed == 2, "Both analyzers must be destroyed exactly once");
    });
    /** @brief Checks direct factory-to-engine initialization and switching analyzer types. */
    run("Factory result passed directly to engine", [] {
        AnalyzerFactory factory;
        AnalysisEngine engine;
        /// @brief Explains parameter initialization and the subsequent ownership transfer.
        /// @details In C++17 and later, createAnalyzer returns a prvalue unique_ptr<TextAnalyzer>
        /// that directly initializes setAnalyzer's by-value parameter, analyzer.
        /// Guaranteed copy elision means no additional unique_ptr move constructor
        /// is called between the function result and that parameter.
        /// Inside the factory, converting unique_ptr<WordFrequencyAnalyzer> to
        /// unique_ptr<TextAnalyzer> does use a converting move constructor.
        /// Inside setAnalyzer, analyzer_ = std::move(analyzer) then transfers
        /// ownership to the existing member using move assignment.
        /// By contrast, passing a named pointer as setAnalyzer(std::move(ptr))
        /// invokes the move constructor to initialize the parameter, leaving ptr empty.
        engine.setAnalyzer(factory.createAnalyzer("frequency"));
        const auto counts = std::any_cast<std::unordered_map<std::string, int>>(
            engine.analyze("cat cat"));
        check(counts.size() == 1 && counts.at("cat") == 2, "Expected frequency result");
        engine.setAnalyzer(factory.createAnalyzer("sentiment"));
        check(std::any_cast<double>(engine.analyze("good")) == 1.0,
              "Engine must use the newly selected analyzer");
    });

    std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
#endif

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
    /** @brief Checks expectations in both Debug and Release builds. */
    void check(bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    /** @brief Tracks destruction and echoes input to verify engine delegation. */
    class TrackingAnalyzer : public TextAnalyzer {
    public:
        explicit TrackingAnalyzer(int& destroyed) : destroyed_(destroyed) {}
        ~TrackingAnalyzer() override { ++destroyed_; }
        std::any analyze(const std::string& text) override { return text; }
        std::string getName() const override { return "Tracking Analyzer"; }
    private:
        int& destroyed_;
    };
}

int Tests::runFactoryAnalyzerTests() {
    int passed = 0;
    int failed = 0;
    std::cout << "FactoryAnalyzer tests\n";
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

    run("Frequency selection and analysis", [] {
        AnalyzerFactory factory;
        auto analyzer = factory.createAnalyzer("frequency");
        check(dynamic_cast<WordFrequencyAnalyzer*>(analyzer.get()) != nullptr,
              "Expected WordFrequencyAnalyzer");
        const auto result = std::any_cast<std::unordered_map<std::string, int>>(
            analyzer->analyze("Hello hello!"));
        check(result.size() == 1 && result.at("hello") == 2, "Unexpected word counts");
    });
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
    run("Sentiment selection and analysis", [] {
        AnalyzerFactory factory;
        auto analyzer = factory.createAnalyzer("sentiment");
        check(dynamic_cast<SentimentAnalyzer*>(analyzer.get()) != nullptr,
              "Expected SentimentAnalyzer");
        check(std::any_cast<double>(analyzer->analyze("good")) == 1.0,
              "Unexpected sentiment score");
    });
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
    run("Factory result passed directly to engine", [] {
        AnalyzerFactory factory;
        AnalysisEngine engine;
        // In C++17 and later, createAnalyzer returns a prvalue unique_ptr<TextAnalyzer>
        // that directly initializes setAnalyzer's by-value parameter, analyzer.
        // Guaranteed copy elision means no additional unique_ptr move constructor
        // is called between the function result and that parameter.
        // Inside the factory, converting unique_ptr<WordFrequencyAnalyzer> to
        // unique_ptr<TextAnalyzer> does use a converting move constructor.
        // Inside setAnalyzer, analyzer_ = std::move(analyzer) then transfers
        // ownership to the existing member using move assignment.
        // By contrast, passing a named pointer as setAnalyzer(std::move(ptr))
        // invokes the move constructor to initialize the parameter, leaving ptr empty.
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

/**
 * @file FactoryAnalyzer.cpp
 * @brief Implements analyzer creation and analysis delegation.
 */
#include "FactoryAnalyzer.h"

#include <stdexcept>
#include <utility>

#include "ReadabilityAnalyzer.h"
#include "SentimentAnalyzer.h"
#include "WordFrequencyAnalyzer.h"

std::unique_ptr<TextAnalyzer> AnalyzerFactory::createAnalyzer(const std::string& type) {
    if (type == "frequency") {
        return std::make_unique<WordFrequencyAnalyzer>();
    }
    else if (type == "readability") {
        return std::make_unique<ReadabilityAnalyzer>();
    }
    else if (type == "sentiment") {
        return std::make_unique<SentimentAnalyzer>();
    }

    throw std::runtime_error("Unknown analyzer type: " + type);
}

void AnalysisEngine::setAnalyzer(std::unique_ptr<TextAnalyzer> analyzer) {
    analyzer_ = std::move(analyzer);
}

std::any AnalysisEngine::analyze(const std::string& text) {
    return analyzer_->analyze(text);
}

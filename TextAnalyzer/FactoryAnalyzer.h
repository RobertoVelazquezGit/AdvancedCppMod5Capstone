/**
 * @file FactoryAnalyzer.h
 * @brief Declares the analyzer factory and analysis context.
 */
#pragma once

#include <any>
#include <memory>
#include <string>

#include "TextAnalyzer.h"

/** @brief Creates text analyzers selected by type. */
class AnalyzerFactory {
public:
    /**
     * @brief Creates an analyzer of the specified type.
     * @param type "frequency", "readability", or "sentiment".
     * @return Exclusive ownership of the created analyzer.
     * @throws std::runtime_error if the type is not recognized.
     */
    std::unique_ptr<TextAnalyzer> createAnalyzer(const std::string& type);
};

/** @brief Owns an analyzer and delegates text analysis to it. */
class AnalysisEngine {
public:
    /** @brief Takes ownership of the supplied analyzer. */
    void setAnalyzer(std::unique_ptr<TextAnalyzer> analyzer);

    /**
     * @brief Analyzes text using the selected analyzer.
     * @pre A non-null analyzer has been supplied through setAnalyzer().
     */
    std::any analyze(const std::string& text);

private:
    std::unique_ptr<TextAnalyzer> analyzer_;
};

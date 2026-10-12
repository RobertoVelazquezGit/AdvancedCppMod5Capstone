/**
 * @file TextAnalyzer.h
 * @brief Declares the abstract interface shared by text analyzers.
 */
#pragma once

#include <any>
#include <string>

/**
 * @brief Abstract base class for text analysis algorithms.
 * @details Derived classes implement analyze() and getName(). Each analyzer
 * defines and documents the concrete result type stored in std::any.
 */
class TextAnalyzer {
public:
    /** @brief Allows safe destruction of derived analyzers through a base pointer. */
    virtual ~TextAnalyzer();

    /**
     * @brief Analyzes the supplied text.
     * @param[in] text Input text; this reference does not permit modification.
     * @return An analysis result stored in std::any. The concrete type is
     * specified by the derived analyzer and can be retrieved with std::any_cast.
     */
    virtual std::any analyze(const std::string& text) = 0;

    /**
     * @brief Gets the human-readable name of the analyzer.
     * @return The analyzer name.
     */
    virtual std::string getName() const = 0;
};

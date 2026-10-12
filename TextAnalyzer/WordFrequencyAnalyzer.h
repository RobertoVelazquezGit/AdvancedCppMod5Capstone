/**
 * @file WordFrequencyAnalyzer.h
 * @brief Declares the word frequency analyzer.
 */
#pragma once

#include "TextAnalyzer.h"

#include <any>
#include <string>

/**
 * @brief Counts occurrences of words after punctuation removal and lowercasing.
 * @details Input is split on whitespace. Punctuation is removed within each
 * token, and tokens that become empty are ignored.
 * @note Character cleanup is byte-based and does not provide full Unicode
 * punctuation recognition or case conversion for UTF-8 text.
 */
class WordFrequencyAnalyzer : public TextAnalyzer {
public:
    /**
     * @brief Counts normalized words in the supplied text.
     * @param[in] text Text to split into whitespace-separated tokens.
     * @return A std::any containing std::unordered_map<std::string, int>, mapping
     * each cleaned word to its occurrence count. Empty or punctuation-only
     * input produces an empty map under the active character classification.
     * @details Each call starts a new count; results from earlier calls are
     * not retained. For example, "Hello, HELLO!" yields the entry "hello": 2.
     */
    std::any analyze(const std::string& text) override;

    /**
     * @brief Gets the display name of this analyzer.
     * @return The string "Word Frequency Analyzer".
     */
    std::string getName() const override;

private:
    /**
     * @brief Removes punctuation and converts a copy of a word to lowercase.
     * @param[in] word Original token, which remains unchanged.
     * @return The cleaned copy, possibly empty if no characters remain.
     * @details Punctuation is deleted rather than replaced with spaces:
     * "co-operate" becomes "cooperate". Classification and lowercasing use
     * std::ispunct and std::tolower with the current C locale.
     * @note This helper does not trim whitespace or decode UTF-8 characters.
     */
    std::string cleanWord(const std::string& word);
};

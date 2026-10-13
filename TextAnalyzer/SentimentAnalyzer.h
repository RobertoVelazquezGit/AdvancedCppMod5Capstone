/**
 * @file SentimentAnalyzer.h
 * @brief Declares the dictionary-based sentiment analyzer.
 */
#pragma once

#include "TextAnalyzer.h"

#include <any>
#include <string>
#include <unordered_set>

/**
 * @brief Scores text using built-in positive and negative English word lists.
 * @details Input is split on whitespace, stripped of punctuation, and converted
 * to lowercase before whole-token dictionary lookup. Repeated matches count
 * separately; words outside the dictionaries do not contribute to the score.
 * @note Normalization uses byte-based std::ispunct and std::tolower with the
 * current C locale; it does not provide full Unicode normalization.
 */
class SentimentAnalyzer : public TextAnalyzer {
public:
    /** @brief Initializes the built-in lowercase sentiment dictionaries. */
    SentimentAnalyzer();

    /**
     * @brief Computes the normalized sentiment score of the supplied text.
     * @param[in] text Input text, which remains unchanged.
     * @return A std::any containing a double in [-1.0, 1.0], calculated as
     * (positive matches - negative matches) / total dictionary matches.
     * Returns 0.0 when no dictionary words match, including empty input.
     * @details Each call computes an independent score. Punctuation is removed
     * rather than replaced with spaces, and digits are preserved. For example,
     * "GOOD! good bad" produces 1.0 / 3.0.
     */
    std::any analyze(const std::string& text) override;

    /**
     * @brief Gets the display name of this analyzer.
     * @return The string "Sentiment Analyzer".
     */
    std::string getName() const override;

private:
    /// @brief Lowercase words that contribute +1 for each matching token.
    std::unordered_set<std::string> positiveWords;
    /// @brief Lowercase words that contribute -1 for each matching token.
    std::unordered_set<std::string> negativeWords;

};

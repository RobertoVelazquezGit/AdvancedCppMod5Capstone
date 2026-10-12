/// @file ReadabilityAnalyzer.h
/// @brief Declares an English readability analyzer and its result metrics.
#pragma once

#include <any>
#include <string>

#include "TextAnalyzer.h"

/// @brief Estimates English readability using Flesch formulas.
/// @details Uses whitespace-delimited words and heuristic syllable and sentence
/// counts. Results are approximate and depend on these counting rules.
class ReadabilityAnalyzer : public TextAnalyzer {
public:
    /// @brief Counts, averages, and readability scores returned by analyze().
    struct ReadabilityMetrics {
        double fleschReadingEase = 0.0; ///< Reading ease score; higher means easier. Not clamped.
        double fleschKincaidGrade = 0.0; ///< Estimated US school grade level. Not clamped.
        double averageSentenceLength = 0.0; ///< Mean number of words per sentence.
        double averageWordLength = 0.0; ///< Mean syllables per word, not characters per word.
        int totalWords = 0; ///< Number of whitespace-delimited tokens, including punctuation-only tokens.
        int totalSentences = 0; ///< Number of sentences estimated from terminal punctuation.
        int totalSyllables = 0; ///< Sum of estimated syllables across all tokens.
    };

    /// @brief Computes counts and Flesch readability metrics for the supplied text.
    /// @param[in] text English text to analyze without modifying it.
    /// @return A std::any containing ReadabilityMetrics, retrievable with std::any_cast.
    /// @details Accumulates syllables one word at a time. If the word or sentence
    /// count is zero, returns the counts with averages and scores left at zero.
    /// Otherwise, with W = words per sentence and S = syllables per word:
    /// - Reading ease = 206.835 - 1.015 * W - 84.6 * S.
    /// - Grade level = 0.39 * W + 11.8 * S - 15.59.
    std::any analyze(const std::string& text) override;

    /// @brief Gets the analyzer's display name.
    /// @return The string "Readability Analyzer".
    std::string getName() const override;

private:
    /// @brief Counts tokens separated by whitespace using stream extraction.
    /// @param[in] text Text whose tokens are counted.
    /// @return Token count, or zero for empty or whitespace-only input.
    /// @note Punctuation and digits are not removed; punctuation-only tokens count.
    int countWords(const std::string& text);

    /// @brief Estimates the number of English syllables in one word.
    /// @param[in] word Token to normalize and analyze.
    /// @return Zero if no ASCII letters remain; otherwise at least one syllable.
    /// @details Keeps only a-z and A-Z, converts uppercase letters to lowercase,
    /// and counts consecutive groups of a, e, i, o, u, and y. A final e removes
    /// one syllable when the count exceeds one, except for consonant + le endings.
    /// @note This heuristic does not resolve all English pronunciation exceptions.
    int countSyllables(const std::string& word);

    /// @brief Estimates sentence count using periods, question marks, and exclamation marks.
    /// @param[in] text Text whose sentence boundaries are examined.
    /// @return Estimated sentence count; zero when there is no alphanumeric content.
    /// @details A boundary counts only after alphanumeric content, as recognized
    /// by std::isalnum in the current C locale. Repeated terminal signs do not
    /// add sentences. Remaining content without terminal punctuation counts once.
    /// @note Abbreviations and decimal points are treated as sentence boundaries.
    /// Line breaks alone do not end sentences.
    int countSentences(const std::string& text);
};


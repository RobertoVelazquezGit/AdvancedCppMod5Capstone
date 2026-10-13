/**
 * @file SentimentAnalyzer.cpp
 * @brief Implements dictionary initialization and normalized sentiment scoring.
 */
#include "SentimentAnalyzer.h"

#include <algorithm>
#include <cctype>
#include <sstream>

SentimentAnalyzer::SentimentAnalyzer()
    : positiveWords{
        "good", "great", "excellent", "amazing", "wonderful",
        "happy", "love", "like", "fantastic", "awesome"
    },
    negativeWords{
        "bad", "terrible", "awful", "horrible", "poor",
        "sad", "hate", "dislike", "disappointing", "worst"
    } {
    // Keep dictionary entries lowercase to match normalized input words.
}

std::any SentimentAnalyzer::analyze(const std::string& text) {
    double score = 0.0;
    std::size_t matchedWords = 0;
    std::istringstream stream(text);
    std::string word;

    while (stream >> word) {
        // Normalize tokens in the same way as WordFrequencyAnalyzer.
        word.erase(std::remove_if(word.begin(), word.end(),
            [](unsigned char c) { return std::ispunct(c) != 0; }),
            word.end());
        std::transform(word.begin(), word.end(), word.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (positiveWords.find(word) != positiveWords.end()) {
            score += 1.0;
            ++matchedWords;
        }
        else if (negativeWords.find(word) != negativeWords.end()) {
            score -= 1.0;
            ++matchedWords;
        }
    }

    // Ignore neutral words and avoid division by zero when nothing matches.
    if (matchedWords > 0) {
        score /= static_cast<double>(matchedWords);
    }

    return score; // std::any stores a double in the range [-1.0, 1.0].
}

std::string SentimentAnalyzer::getName() const {
    return "Sentiment Analyzer";
}

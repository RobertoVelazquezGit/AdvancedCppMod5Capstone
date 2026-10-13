#pragma once

#include "TextAnalyzer.h"

#include <any>
#include <string>
#include <unordered_set>

// Sentiment analyzer implementation
class SentimentAnalyzer : public TextAnalyzer {
public:
    // Constructor that loads positive and negative word lists
    SentimentAnalyzer();

    // Analyze sentiment and return a score between -1.0 (negative) and 1.0 (positive)
    std::any analyze(const std::string& text) override;
    std::string getName() const override;

private:
    // Store dictionaries of positive and negative words
    std::unordered_set<std::string> positiveWords;
    std::unordered_set<std::string> negativeWords;
    // Helper methods
    void loadWordLists();
    std::vector<std::string> tokenizeText(const std::string& text);

};



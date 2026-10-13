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

    // Returns std::any containing a double: (positive - negative) / matched words.
    // Range: [-1.0, 1.0]; returns 0.0 when no dictionary words match.
    std::any analyze(const std::string& text) override;
    std::string getName() const override;

private:
    // Store dictionaries of positive and negative words
    std::unordered_set<std::string> positiveWords;
    std::unordered_set<std::string> negativeWords;

};



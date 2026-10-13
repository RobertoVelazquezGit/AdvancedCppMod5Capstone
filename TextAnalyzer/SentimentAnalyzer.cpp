#include "SentimentAnalyzer.h"

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

    // TODO: Implement sentiment analysis
    // 1. Split the text into words
    // 2. Clean each word
    // 3. Check if the word is in the positive or negative list
    // 4. Calculate a sentiment score

    return score;
}

std::string SentimentAnalyzer::getName() const {
    return "Sentiment Analyzer";
}

void SentimentAnalyzer::loadWordLists()
{
}

std::vector<std::string> SentimentAnalyzer::tokenizeText(const std::string& text)
{
    return std::vector<std::string>();
}



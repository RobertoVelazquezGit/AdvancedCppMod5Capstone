/**
 * @file WordFrequencyAnalyzer.cpp
 * @brief Implements word counting and byte-based token cleanup.
 */
#include "WordFrequencyAnalyzer.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_map>

/**
 * @details Extracts tokens with std::istringstream, cleans each token, and
 * increments its map entry only when the cleaned token is nonempty.
 */
std::any WordFrequencyAnalyzer::analyze(const std::string& text) {
    std::unordered_map<std::string, int> wordCounts;
    std::istringstream stream(text);
    std::string word;

    while (stream >> word) {
        // Clean the word (remove punctuation, convert to lowercase)
        word = cleanWord(word);

        if (!word.empty()) {
            wordCounts[word]++;
        }
    }

    return wordCounts;
}

/** @details Returns a fixed label independent of the analyzed text. */
std::string WordFrequencyAnalyzer::getName() const {
    return "Word Frequency Analyzer";
}

/**
 * @details Applies the erase-remove idiom followed by an in-place transform
 * to a local copy. Both character predicates receive unsigned char values
 * to satisfy the input requirements of the C character functions.
 */
std::string WordFrequencyAnalyzer::cleanWord(const std::string& word) {
    // Work on a copy because word is a const reference to the original string.
    std::string cleaned = word;

    // Punctuation includes signs such as '.', ',', '!', '?', and '-'.
    // ispunct returns nonzero for punctuation, and zero for letters, digits,
    // or spaces. Its classification depends on the current C locale.
    // Signs are removed anywhere in the word: "hello!" becomes "hello",
    // and "co-operate" becomes "cooperate".
    // remove_if moves the characters we keep to the front and returns their
    // new logical end. It does not change the string's size by itself.
    // erase then removes the leftover range from that iterator to the old end.
    // unsigned char ensures that ispunct receives a valid character value,
    // even on systems where plain char can hold negative values.
    cleaned.erase(std::remove_if(cleaned.begin(), cleaned.end(),
        [](unsigned char c) { return std::ispunct(c) != 0; }),
        cleaned.end());

    // transform reads the range [begin, end) and writes the converted characters
    // back into the same string, starting at the third argument (begin).
    // The lambda converts each character to lowercase. tolower returns int,
    // so static_cast<char> converts its result back to the string's char type.
    // unsigned char also provides a valid input value for tolower.
    // These operations process bytes, not complete Unicode characters, so they
    // do not provide full punctuation removal or case conversion for UTF-8.
    std::transform(cleaned.begin(), cleaned.end(), cleaned.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    // Return the cleaned copy; for example, "HELLO!" becomes "hello".
    return cleaned;
}

//class WordFrequencyAnalyzer {
//public:
//    std::map<std::string, int> analyzeText(const std::string& text) {
//        std::map<std::string, int> wordCounts;
//        std::istringstream stream(text);
//        std::string word;
//
//        while (stream >> word) {
//            // Clean the word (remove punctuation, convert to lowercase)
//            cleanWord(word);
//
//            if (!word.empty()) {
//                wordCounts[word]++;
//            }
//        }
//
//        return wordCounts;
//    }
//
//private:
//    void cleanWord(std::string& word) {
//        // Remove punctuation
//        word.erase(std::remove_if(word.begin(), word.end(),
//            [](char c) { return std::ispunct(c); }),
//            word.end());
//
//        // Convert to lowercase
//        std::transform(word.begin(), word.end(), word.begin(),
//            [](char c) { return std::tolower(c); });
//    }
//};

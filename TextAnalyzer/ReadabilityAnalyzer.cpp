/// @file ReadabilityAnalyzer.cpp
/// @brief Implements heuristic English text counts and Flesch readability metrics.
#include "ReadabilityAnalyzer.h"

#include <cctype>
#include <sstream>

std::any ReadabilityAnalyzer::analyze(const std::string& text) {
    ReadabilityMetrics metrics;

    // Count words, syllables, and sentences
    int wordCount = countWords(text);
    int syllableCount = 0;
    std::istringstream input(text);
    std::string word;
    while (input >> word) {
        syllableCount += countSyllables(word);
    }
    int sentenceCount = countSentences(text);

    metrics.totalWords = wordCount;
    metrics.totalSyllables = syllableCount;
    metrics.totalSentences = sentenceCount;

    if (wordCount == 0 || sentenceCount == 0) {
        return metrics;
    }

    // Calculate metrics
    metrics.averageSentenceLength = static_cast<double>(wordCount) / sentenceCount;
    metrics.averageWordLength = static_cast<double>(syllableCount) / wordCount;

    // Flesch Reading Ease
    metrics.fleschReadingEase = 206.835 - 1.015 * metrics.averageSentenceLength - 84.6 * metrics.averageWordLength;

    // Flesch-Kincaid Grade Level
    metrics.fleschKincaidGrade = 0.39 * metrics.averageSentenceLength + 11.8 * metrics.averageWordLength - 15.59;

    return metrics;
}

std::string ReadabilityAnalyzer::getName() const {
    return "Readability Analyzer";
}

int ReadabilityAnalyzer::countWords(const std::string& text) {
    std::istringstream input(text);
    std::string word;
    int count = 0;

    while (input >> word) {
        ++count;
    }

    return count;
}

int ReadabilityAnalyzer::countSyllables(const std::string& word) {
    // Keep English letters only and normalize uppercase letters.
    std::string letters;
    for (char ch : word) {
        if (ch >= 'A' && ch <= 'Z') {
            ch = static_cast<char>(ch - 'A' + 'a');
        }
        if (ch >= 'a' && ch <= 'z') {
            letters += ch;
        }
    }

    if (letters.empty()) {
        return 0;
    }

    const auto isVowel = [](char ch) {
        return ch == 'a' || ch == 'e' || ch == 'i' ||
               ch == 'o' || ch == 'u' || ch == 'y';
    };

    // Approximate one syllable per consecutive group of vowels.
    int syllables = 0;
    bool previousWasVowel = false;
    for (char ch : letters) {
        const bool currentIsVowel = isVowel(ch);
        if (currentIsVowel && !previousWasVowel) {
            ++syllables;
        }
        previousWasVowel = currentIsVowel;
    }

    // Discount a silent final e, except consonant + le (for example, table).
    const auto length = letters.size();
    const bool consonantLe = length >= 3 && letters[length - 2] == 'l' &&
                             !isVowel(letters[length - 3]);
    if (letters.back() == 'e' && syllables > 1 && !consonantLe) {
        --syllables;
    }

    return syllables > 0 ? syllables : 1;
}

int ReadabilityAnalyzer::countSentences(const std::string& text) {
    int sentences = 0;
    bool hasContent = false;

    // Approximate sentence boundaries using terminal punctuation.
    for (unsigned char ch : text) {
        if (ch == '.' || ch == '?' || ch == '!') {
            if (hasContent) {
                ++sentences;
                hasContent = false;
            }
        }
        else if (std::isalnum(ch)) {
            hasContent = true;
        }
    }

    // Count a final sentence even when it has no terminal punctuation.
    return sentences + (hasContent ? 1 : 0);
}



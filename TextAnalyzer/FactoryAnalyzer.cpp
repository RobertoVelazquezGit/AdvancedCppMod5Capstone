/**
 * @file FactoryAnalyzer.cpp
 * @brief Implements analyzer creation and analysis delegation.
 */
#include "FactoryAnalyzer.h"

#include <stdexcept>
#include <utility>

#include "ReadabilityAnalyzer.h"
#include "SentimentAnalyzer.h"
#include "WordFrequencyAnalyzer.h"

std::unique_ptr<TextAnalyzer> AnalyzerFactory::createAnalyzer(const std::string& type) {
    if (type == "frequency") {
        return std::make_unique<WordFrequencyAnalyzer>();
    }
    else if (type == "readability") {
        return std::make_unique<ReadabilityAnalyzer>();
    }
    else if (type == "sentiment") {
        return std::make_unique<SentimentAnalyzer>();
    }

    throw std::runtime_error("Unknown analyzer type: " + type);
}

/*
 * How unique_ptr transfers ownership:
 *
 * std::move does not move anything by itself. It casts its argument to an
 * rvalue reference, allowing a move constructor or move assignment operator
 * to be selected.
 *
 * auto other = std::move(analyzer);
 * This creates a new unique_ptr by calling its move constructor. The new
 * pointer takes ownership, and analyzer becomes empty (nullptr). The pointed-to
 * TextAnalyzer stays at the same memory address; its move constructor is not
 * called. The unique_ptr also transfers its deleter as appropriate.
 *
 * Conceptually, for a simple owning pointer, move construction looks like:
 *     UniquePtr(UniquePtr&& source) : ptr(source.ptr) {
 *         source.ptr = nullptr;
 *     }
 * This is only an illustration, not the actual std::unique_ptr implementation.
 * Copying instead (auto other = analyzer;) is forbidden because unique_ptr
 * represents exclusive ownership.
 *
 * other = std::move(analyzer);
 * Here other already exists, so this calls move assignment, not the move
 * constructor. Any object previously owned by other is destroyed, and other
 * takes ownership of the incoming object. The source pointer becomes empty.
 * This is what happens to the existing analyzer_ member in setAnalyzer below.
 *
 * Passing a named unique_ptr to setAnalyzer(std::move(analyzer)) first moves
 * ownership into the by-value parameter; the assignment in the function then
 * moves ownership from that parameter into analyzer_. A temporary returned by
 * createAnalyzer can be passed directly, without an explicit std::move.
 * Returning from the factory likewise requires no copy: ownership can be
 * moved or the result can be constructed directly, depending on the expression.
 */
void AnalysisEngine::setAnalyzer(std::unique_ptr<TextAnalyzer> analyzer) {
    analyzer_ = std::move(analyzer);
}

std::any AnalysisEngine::analyze(const std::string& text) {
    return analyzer_->analyze(text);
}

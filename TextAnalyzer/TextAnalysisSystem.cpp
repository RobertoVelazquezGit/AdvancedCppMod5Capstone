/**
 * @file TextAnalysisSystem.cpp
 * @brief Implements parallel text analysis and English report formatting.
 * @details File analysis delegates to the thread-pool-backed text analysis path.
 * Batch processing saves one formatted report per matching input file.
 */
#include "TextAnalysisSystem.h"
#include "FileHandling.h"
#include "ReadabilityAnalyzer.h"

#include <exception>
#include <future>
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <unordered_map>
#include <utility>

/** @brief Initializes the worker pool and takes ownership of the default analyzers. */
TextAnalysisSystem::TextAnalysisSystem(size_t numThreads)
    : threadPool(numThreads) {
    // Initialize analyzers
    analyzers.push_back(factory.createAnalyzer("frequency"));
    analyzers.push_back(factory.createAnalyzer("readability"));
    analyzers.push_back(factory.createAnalyzer("sentiment"));
}

/** @brief Reads and analyzes a file, reporting standard exceptions to standard error. */
std::map<std::string, std::any> TextAnalysisSystem::analyzeFile(const std::string& filename) {
    try {
        std::string content = FileHandling::readTextFile(filename);
        return analyzeText(content);
    }
    catch (const std::exception& e) {
        std::cerr << "Error analyzing file " << filename << ": " << e.what() << std::endl;
        return {};
    }
}

/** @brief Runs analyzer tasks and waits for accepted work before leaving this scope. */
std::map<std::string, std::any> TextAnalysisSystem::analyzeText(const std::string& text) {
    std::map<std::string, std::any> results;
    std::vector<std::future<std::pair<std::string, std::any>>> futures;

    futures.reserve(analyzers.size());

    try {
        // Submit every task before waiting so available workers can run in parallel.
        for (const auto& analyzer : analyzers) {
            /**
             * @brief Captures a non-owning pointer to the analyzer for this task.
             * @details In [analyzer = analyzer.get()], the right-hand analyzer
             * is the vector's unique_ptr<TextAnalyzer>. get() returns its raw
             * TextAnalyzer* without copying the unique_ptr or transferring ownership.
             * The left-hand analyzer is a new lambda capture storing that pointer
             * by value; inside the lambda, analyzer refers to this raw pointer.
             * The vector's unique_ptr remains the owner. The lambda must not
             * delete the object. text is captured by reference rather than copied.
             * analyzeText waits for all accepted tasks, including on exception,
             * before leaving. The caller must keep this system alive and must
             * not modify text during the call, so these borrowed accesses stay valid.
             */
            futures.push_back(threadPool.enqueue([analyzer = analyzer.get(), &text] {
                return std::make_pair(analyzer->getName(), analyzer->analyze(text));
            }));
        }

        for (auto& future : futures) {
            auto result = future.get();
            /**
             * @brief Moves the collected name and result into the map entry.
             * @details std::move does not move data itself: it casts each member
             * to an rvalue reference, allowing emplace to construct the entry's
             * key and value using the std::string and std::any move constructors.
             * The map stores pair<const std::string, std::any>; the const key
             * can still be initialized by moving from the non-const source string.
             * Without std::move, these lvalue members would be copied, including
             * the object held by std::any, which may be a large frequency map.
             * Moving std::any does not necessarily call the contained object's
             * move constructor; it may transfer ownership of allocated storage.
             * result is no longer needed and is destroyed at the iteration's end.
             */
            results.emplace(std::move(result.first), std::move(result.second));
        }
    }
    catch (...) {
        // Finish accepted tasks before text or the owned analyzers can be destroyed.
        // get() consumes its future even when it rethrows a task exception.
        for (auto& future : futures) {
            // Here, valid() returns true if get() has not yet consumed this future.
            if (future.valid()) {
                future.wait();
            }
        }
        throw;
    }

    return results;
}

/**
 * @brief Analyzes matching files and saves a separate report for each one.
 * @details Files are processed on the calling thread; analyzeText submits the
 * individual analyzers to the pool. Enqueuing analyzeFile in that same pool
 * could deadlock when workers wait for nested tasks with no free worker.
 */
void TextAnalysisSystem::batchProcess(const std::string& directory, const std::string& extension, const std::string& outputDir) {
    namespace fs = std::filesystem;

    // Snapshot the input list before creating any reports.
    // Setup errors affect the entire batch and propagate to the caller.
    const auto files = FileHandling::listFiles(directory, extension);
    const fs::path outputDirectory(outputDir);
    fs::create_directories(outputDirectory);
    if (fs::equivalent(fs::path(directory), outputDirectory)) {
        throw std::invalid_argument("Input and output directories must be different");
    }

    for (const auto& filename : files) {
        try {
            const auto results = analyzeFile(filename);
            // analyzeFile already reports standard exceptions and returns an
            // empty map on failure. An empty input file still has analyzer results.
            if (results.empty()) {
                continue;
            }

            // path::stem removes the last extension; operator/ joins path components.
            // Example: input/document.txt becomes output/document_report.txt.
            const fs::path reportPath =
                outputDirectory / (fs::path(filename).stem().string() + "_report.txt");
            const auto report = generateReport(results);
            FileHandling::writeTextFile(reportPath.string(), report);
        }
        catch (const std::exception& error) {
            // A failed report must not prevent processing the remaining files.
            std::cerr << "Error processing file " << filename << ": "
                      << error.what() << std::endl;
        }
    }
}

/** @brief Builds report sections from the named, typed analyzer results. */
std::string TextAnalysisSystem::generateReport(const std::map<std::string, std::any>& results) {
    std::ostringstream report;
    /**
     * @brief Accumulates the report using locale-independent numeric formatting.
     * @details fixed and setprecision(2) affect floating-point values only;
     * word, sentence, and syllable counts retain integer formatting.
     */
    report.imbue(std::locale::classic());
    report << std::fixed << std::setprecision(2);
    report << "Text Analysis Report\n====================\n";

    if (results.empty()) {
        report << "No analysis results available.\n";
        return report.str();
    }

    /**
     * @brief Uses a structured binding, available in C++17 and later.
     * @details Each map element is a pair<const std::string, std::any>.
     * [name, result] names its first (key) and second (value) members.
     * const auto& accesses the element without copying it and prevents
     * modification through these names. Before C++17, use entry.first
     * and entry.second inside a loop over const auto& entry instead.
     */
    for (const auto& [name, result] : results) {
        report << "\n" << name << "\n";
        if (name == "Word Frequency Analyzer") {
            /**
             * @brief Borrows the frequency map stored in the result.
             * @details any_cast requires the exact payload type and throws
             * std::bad_any_cast on mismatch. The const reference avoids a copy
             * during extraction and remains valid while results is unchanged.
             */
            const auto& frequencies =
                std::any_cast<const std::unordered_map<std::string, int>&>(result);
            /**
             * @brief Copies counts into an ordered map for reproducible output.
             * @details This sorting copy is separate from the reference-based
             * extraction above; the original unordered_map remains unchanged.
             */
            const std::map<std::string, int> sorted(frequencies.begin(), frequencies.end());
            if (sorted.empty()) {
                report << "No words found.\n";
            }
            for (const auto& [word, count] : sorted) {
                report << "  " << word << ": " << count << '\n';
            }
        }
        else if (name == "Readability Analyzer") {
            const auto& metrics =
                std::any_cast<const ReadabilityAnalyzer::ReadabilityMetrics&>(result);
            report << "Total words: " << metrics.totalWords << '\n'
                   << "Total sentences: " << metrics.totalSentences << '\n'
                   << "Total syllables: " << metrics.totalSyllables << '\n'
                   << "Average sentence length (words): " << metrics.averageSentenceLength << '\n'
                   << "Average word length (syllables): " << metrics.averageWordLength << '\n'
                   << "Flesch reading ease: " << metrics.fleschReadingEase << '\n'
                   << "Flesch-Kincaid grade: " << metrics.fleschKincaidGrade << '\n';
        }
        else if (name == "Sentiment Analyzer") {
            const double score = std::any_cast<double>(result);
            report << "Sentiment score [-1, 1]: " << score << '\n';
        }
        else {
            report << "Report formatting is not supported for this analyzer.\n";
        }
    }
    return report.str();
}


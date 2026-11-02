/**
 * @file TextAnalysisSystem.h
 * @brief Declares the text analysis system and its owned resources.
 */
#pragma once

#include "FactoryAnalyzer.h"
#include "TextAnalyzer.h"
#include "ThreadPool.h"

#include <any>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief Owns a thread pool and the frequency, readability, and sentiment analyzers.
 * @details Runs the three analyzers through the thread pool and collects typed
 * results in std::any. A private formatter produces English text reports.
 * Batch processing writes one report per matching file to a separate directory.
 */
class TextAnalysisSystem {
public:
    /**
     * @brief Starts the thread pool and creates the three default analyzers.
     * @param numThreads Worker count; defaults to hardware_concurrency().
     * @throws std::invalid_argument If numThreads is zero, including when the
     * default hardware_concurrency() value is zero.
     * @details The vector takes exclusive ownership of each analyzer returned
     * by the factory. Thread pool and analyzer creation errors propagate.
     */
    TextAnalysisSystem(size_t numThreads = std::thread::hardware_concurrency());

    /**
     * @brief Reads a text file and delegates to analyzeText().
     * @param filename Path to the input text file.
     * @return Results keyed by analyzer name, or an empty map after catching
     * a standard exception.
     * @details Caught standard exceptions from reading or analysis are reported
     * to standard error. Non-standard exceptions propagate to the caller.
     * An empty input file is analyzed normally; it is not treated as a failure.
     */
    std::map<std::string, std::any> analyzeFile(const std::string& filename);

    /**
     * @brief Submits all analyzers to the thread pool and collects their results.
     * @param text Text shared by reference with the analysis tasks.
     * @return Results keyed by getName(), preserving each concrete type in std::any.
     * @details Submits every task before collecting results with future::get().
     * Waits for all accepted tasks before returning or propagating an exception,
     * keeping the referenced text and owned analyzers alive during execution.
     * Submission, analysis, and result storage exceptions propagate to the caller.
     * @pre The caller must not modify text or destroy this system during the call.
     */
    std::map<std::string, std::any> analyzeText(const std::string& text);

    /**
     * @brief Analyzes matching files and writes one English report per file.
     * @param[in] directory Input directory, scanned without recursion.
     * @param[in] extension Exact, case-sensitive extension including the dot,
     * e.g. ".txt". An empty string selects files without an extension.
     * @param[in] outputDir Separate output directory, created with any missing parents.
     * @details Processes files sequentially while each file's analyzers use the
     * thread pool. Reports use the input stem followed by "_report.txt";
     * existing report files are overwritten. Input order is unspecified.
     * An empty match list produces no reports but still prepares the output directory.
     * Standard exceptions for individual files are logged and processing continues;
     * an empty result from analyzeFile is skipped. Non-standard exceptions propagate.
     * @throws std::filesystem::filesystem_error If input listing, output directory
     * creation, or directory equivalence checking fails.
     * @throws std::invalid_argument If input and output refer to the same directory.
     * @note Uses FileHandling's read/write error checks. Does not return a success count.
     */
    void batchProcess(const std::string& directory, const std::string& extension, const std::string& outputDir);

private:
    ThreadPool threadPool; ///< Worker pool used for parallel analysis.
    AnalyzerFactory factory; ///< Factory used to create the default analyzers.
    std::vector<std::unique_ptr<TextAnalyzer>> analyzers; ///< Exclusively owned analyzers.

    /**
     * @brief Formats available analysis results into an English text report.
     * @param[in] results Results keyed by the exact analyzer display names.
     * @return A report headed "Text Analysis Report", with one section per entry.
     * @details Recognized names and their required std::any payload types are:
     * - "Word Frequency Analyzer": std::unordered_map<std::string, int>.
     * - "Readability Analyzer": ReadabilityAnalyzer::ReadabilityMetrics.
     * - "Sentiment Analyzer": double.
     *
     * Sections follow the input map's key order. Word counts are listed in
     * lexicographic word order, and an empty frequency map prints "No words found.".
     * Readability includes all seven metrics; average word length is measured
     * in syllables, not characters. Floating-point values use fixed notation
     * with two decimal places and the classic locale's decimal point.
     *
     * An empty results map prints "No analysis results available." after the
     * heading. Missing analyzers are omitted. Unknown names receive an
     * unsupported-format message without inspecting their payload.
     * @throws std::bad_any_cast If a recognized name has a missing or wrong payload type.
     * @note Does not modify results, run analyzers, write files, or print to the console.
     */
    std::string generateReport(const std::map<std::string, std::any>& results);
};

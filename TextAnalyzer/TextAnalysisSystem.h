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
 * @details Runs the analyzers through the thread pool. Batch processing and
 * report generation remain unfinished.
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
     * @details Caught standard exceptions are reported to standard error.
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
     * @brief Provides the entry point for batch file processing.
     * @param directory Input directory; currently unused.
     * @param extension File extension filter, such as ".txt"; currently unused.
     * @param outputDir Output directory for results; currently unused.
     * @note This placeholder currently performs no operations.
     */
    void batchProcess(const std::string& directory, const std::string& extension, const std::string& outputDir);

private:
    ThreadPool threadPool; ///< Worker pool used for parallel analysis.
    AnalyzerFactory factory; ///< Factory used to create the default analyzers.
    std::vector<std::unique_ptr<TextAnalyzer>> analyzers; ///< Exclusively owned analyzers.

    /**
     * @brief Provides the placeholder for formatting analysis results.
     * @param results Results to format; currently unused.
     * @return The literal string "Report placeholder".
     * @note Report formatting is not implemented yet.
     */
    std::string generateReport(const std::map<std::string, std::any>& results);
};

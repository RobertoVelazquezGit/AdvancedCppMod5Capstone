/**
 * @file TextAnalysisSystem.cpp
 * @brief Implements system initialization and the existing analysis scaffolding.
 */
#include "TextAnalysisSystem.h"
#include "FileHandling.h"

#include <exception>
#include <future>
#include <iostream>
#include <utility>

TextAnalysisSystem::TextAnalysisSystem(size_t numThreads)
    : threadPool(numThreads) {
    // Initialize analyzers
    analyzers.push_back(factory.createAnalyzer("frequency"));
    analyzers.push_back(factory.createAnalyzer("readability"));
    analyzers.push_back(factory.createAnalyzer("sentiment"));
}

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

void TextAnalysisSystem::batchProcess(const std::string& directory, const std::string& extension, const std::string& outputDir) {
    // TODO: Implement batch processing
    // 1. List files in the directory
    // 2. Process each file
    // 3. Save results to output directory
}

std::string TextAnalysisSystem::generateReport(const std::map<std::string, std::any>& results) {
    // TODO: Implement report generation
    // Format the results into a readable report
    return "Report placeholder";
}


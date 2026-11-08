/** @file TextAnalysisSystemTests.cpp
 * @brief Integration tests using the public system API and isolated real files.
 */
#include "../RunConfig.h"
#if TEXTANALYSISSYSTEM_RUN_TESTS
#include "TextAnalysisSystemTests.h"
#include "../TextAnalysisSystem.h"
#include "../ReadabilityAnalyzer.h"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
/**
 * @brief Keeps helper functions and types local to this translation unit.
 * @details Functions have internal linkage, like file-scope static functions.
 * TemporaryDirectory is also a local type; its members do not become static.
 * Other translation units can define independent helpers with the same names.
 */
namespace {
    namespace fs = std::filesystem;

    /**
     * @brief Reports a failed expectation in both Debug and Release builds.
     * @param condition Expectation that must evaluate to true.
     * @param message Diagnostic used when the expectation fails.
     * @throws std::runtime_error If condition is false, stopping the current test.
     */
    void check(bool condition, const std::string& message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    /**
     * @brief Requires the callable to throw the specified exception type.
     * @tparam Exception Expected exception type; derived types also match.
     * @tparam Function Callable type accepting no arguments.
     * @param action Operation expected to throw.
     * @throws std::runtime_error If action returns without throwing.
     * @details Other exception types propagate to the test runner unchanged.
     */
    template <typename Exception, typename Function>
    void expectThrows(Function action) {
        try {
            action();
        }
        catch (const Exception&) {
            return;
        }
        throw std::runtime_error("Expected exception was not thrown");
    }

    /**
     * @brief Owns a newly created temporary directory and cleans it on exit.
     * @details Uses RAII to own a unique temporary directory. Destruction attempts
     * cleanup even during exception unwinding. Explicit cleanup reports errors
     * before the runner prints success. Copying is forbidden to avoid shared ownership.
     */
    class TemporaryDirectory {
    public:
        /**
         * @brief Creates and owns a new directory for temporary test files.
         * @throws std::filesystem::filesystem_error If a filesystem operation fails.
         * @throws std::runtime_error If the parent check fails or all names are taken.
         */
        TemporaryDirectory() {
            /** @brief Get the system temporary directory as an absolute, normalized path. */
            /** @brief Lexical normalization simplifies the path without resolving symlinks. */
            const auto root = fs::absolute(fs::temp_directory_path()).lexically_normal();
            /** @brief Use a clock tick count to reduce name collisions, not as a calendar date. */
            /** @brief This value alone does not guarantee a unique directory name. */
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            /** @brief Try up to 100 names, changing the numeric suffix after a collision. */
            for (int attempt = 0; attempt < 100; ++attempt) {
                /** @brief fs::path overloads operator / to compose paths, not divide numbers. */
                /** @brief Here it appends a relative folder name to root, handling separators. */
                /** @brief Example: fs::path("C:/Temp") / "test" represents C:/Temp/test. */
                /** @brief It returns a new path without modifying root or creating a directory. */
                const auto candidate = root / ("TextAnalyzer-tests-" +
                    std::to_string(stamp) + "-" + std::to_string(attempt));
                /** @brief parent_path() extracts the candidate's parent, without the new name. */
                /** @brief equivalent() queries the filesystem to check that both paths refer */
                /** @brief to the same directory, rather than comparing their text spelling. */
                /** @brief We compare the existing parent with root, not the uncreated candidate. */
                /** @brief A query error can throw; a false result makes check() throw. */
                check(fs::equivalent(candidate.parent_path(), root),
                      "Invalid temporary directory");
                /** @brief true: a new directory was created; false: a directory already exists. */
                /** @brief Other creation errors throw instead of retrying with a different name. */
                if (fs::create_directory(candidate)) {
                    /** @brief Own only a directory created by this instance, for later cleanup. */
                    path_ = candidate;
                    /** @brief Finish construction successfully; constructors return no value. */
                    return;
                }
            }
            /** @brief All candidate names were already occupied by existing directories. */
            throw std::runtime_error("Could not create a unique test directory");
        }

        /** @brief Prevents two instances from owning the same directory. */
        TemporaryDirectory(const TemporaryDirectory&) = delete;
        /** @brief Prevents replacing ownership through copy assignment. */
        TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

        /** @brief Removes the owned temporary directory and reports cleanup errors. */
        ~TemporaryDirectory() {
            std::error_code error;
            /** @brief Recursively remove the directory, its files, and its subdirectories. */
            /** @brief error is passed by non-const reference (std::error_code&), allowing */
            /** @brief remove_all to report filesystem errors through it instead of throwing */
            /** @brief a filesystem_error. On success, the error code is cleared. */
            /** @brief If path_ no longer exists (e.g. cleanup() already removed it), that is */
            /** @brief not an error: remove_all removes nothing and returns zero. */
            fs::remove_all(path_, error);
            /** @brief Report a cleanup failure without throwing a filesystem exception. */
            if (error) {
                std::cerr << "[WARN] Cleanup failed for " << path_ << ": "
                          << error.message() << '\n';
            }
        }

        /**
         * @brief Provides the owned temporary directory path.
         * @return Reference valid while this owner exists.
         */
        const fs::path& path() const { return path_; }

        /**
         * @brief Performs checked cleanup before reporting suite success.
         * @throws fs::filesystem_error If removal fails.
         * @details Later destructor cleanup is harmless if the directory is gone.
         */
        void cleanup() { fs::remove_all(path_); }

    private:
        fs::path path_; ///< Absolute path of the directory owned by this instance.
    };


    /**
     * @brief Writes independent input fixtures, checking stream errors.
     * @param[in] path Destination inside the current test directory.
     * @param[in] text Contents to write in text mode.
     * @throws std::ios_base::failure If opening, writing, or closing fails.
     * @details failbit and badbit enable stream exceptions. Existing contents are
     * truncated; parent directories must exist. Standard streams create fixtures
     * independently of the FileHandling implementation under test.
     */
    void write(const fs::path& path, const std::string& text) {
        std::ofstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);
        file.open(path);
        file << text;
        file.close();
    }

    /**
     * @brief Reads generated reports independently of FileHandling.
     * @param[in] path Report or fixture file to read in text mode.
     * @return Characters accumulated in the output string stream.
     * @throws std::runtime_error If opening fails or file.bad() is set afterward.
     * @details The local file stream closes automatically when the function exits.
     */
    std::string read(const fs::path& path) {
        std::ifstream file(path);
        check(file.is_open(), "Report was not created: " + path.string());
        std::ostringstream text;
        /**
         * @brief Copies the remaining file characters into the string stream.
         * @details file.rdbuf() returns a pointer to the file's stream buffer,
         * not a string. The operator<< overload for std::streambuf* extracts
         * characters from that buffer and inserts them into text; it does not
         * print the pointer's address. Extraction starts at the current read
         * position and normally continues until end-of-file, preserving spaces
         * and line breaks as delivered by the input stream (text-mode newline
         * translation may apply). Here the file has just been opened, so the
         * transfer starts at the beginning. Finally, text.str() below returns
         * the accumulated characters as a std::string.
         */
		text << file.rdbuf();  // rdbuf returns a pointer to the stream buffer, not a string, 
		                        // and operator<< for std::streambuf* extracts characters from that buffer. 
        check(!file.bad(), "Could not read report");
        return text.str();
    }

    /**
     * @brief Checks all three payload types and independently calculated values.
     * @param[in] results Named payloads returned by the system under test.
     * @param[in] empty True for empty or whitespace input; otherwise expect the
     * results for Good good. Bad! (a newline may replace a separating space).
     * @throws std::runtime_error If any expected value or result size differs.
     * @throws std::out_of_range If a required analyzer name is missing.
     * @throws std::bad_any_cast If an analyzer payload has an unexpected type.
     * @details The sample has three words, two sentences, and three syllables.
     * W = 1.5 words/sentence and S = 1 syllable/word give Flesch ease
     * 206.835 - 1.015 * W - 84.6 * S = 120.7125 and grade
     * 0.39 * W + 11.8 * S - 15.59 = -3.205. Sentiment is (2 - 1) / 3.
     * Floating-point checks use an absolute tolerance of 1e-9. Const-reference
     * any_cast borrows the stored map and metrics without copying their contents.
     */
    void verify(const std::map<std::string, std::any>& results, bool empty = false) {
        check(results.size() == 3, "Expected exactly three analyzer results");
        /**
         * @brief Borrows the frequency map stored inside std::any without copying it.
         * @details Requesting const Map& from any_cast returns a reference to the
         * stored map, avoiding copies of its strings and counters. const prevents
         * modification through this reference. Both the cast type and const auto&
         * preserve reference access: const auto& alone would not prevent a copy
         * if any_cast<Map> returned a value; it would bind to that temporary copy.
         * The reference remains valid while the stored object exists and is not
         * replaced. Here results remains alive and unchanged throughout verify.
         */
        const auto& words = std::any_cast<const std::unordered_map<std::string, int>&>(
            results.at("Word Frequency Analyzer"));
        const std::unordered_map<std::string, int> expected = empty
            ? std::unordered_map<std::string, int>{}
            : std::unordered_map<std::string, int>{{"good", 2}, {"bad", 1}};
        check(words == expected, "Unexpected word frequencies");
        /**
         * @brief Borrows the stored metrics structure for read-only checks.
         * @details As with words, any_cast<const ReadabilityMetrics&> accesses
         * the original payload rather than returning a copy. The structure is
         * small, but these checks only need to inspect its fields. const auto&
         * retains the reference, with the same lifetime requirements as words.
         */
        const auto& m = std::any_cast<const ReadabilityAnalyzer::ReadabilityMetrics&>(
            results.at("Readability Analyzer"));
        check(m.totalWords == (empty ? 0 : 3) && m.totalSentences == (empty ? 0 : 2)
            && m.totalSyllables == (empty ? 0 : 3), "Unexpected readability counts");
        check(std::abs(m.averageSentenceLength - (empty ? 0.0 : 1.5)) < 1e-9
            && std::abs(m.averageWordLength - (empty ? 0.0 : 1.0)) < 1e-9
            && std::abs(m.fleschReadingEase - (empty ? 0.0 : 120.7125)) < 1e-9
            && std::abs(m.fleschKincaidGrade - (empty ? 0.0 : -3.205)) < 1e-9,
            "Unexpected readability averages or scores");
        /**
         * @brief Copies the scalar sentiment score for the arithmetic check.
         * @details any_cast<double> returns the stored value by value. Copying
         * one double is inexpensive and avoids retaining a borrowed reference.
         * any_cast<const double&> would also be valid, but provides no practical
         * benefit here. Reference casts above are useful for avoiding copies of
         * compound objects; a scalar value is convenient for this calculation.
         */
        check(std::abs(std::any_cast<double>(results.at("Sentiment Analyzer"))
            - (empty ? 0.0 : 1.0 / 3.0)) < 1e-9, "Unexpected sentiment score");
    }

    /**
     * @brief Exercises queued tasks with one worker and parallel workers, repeatedly.
     * @details The unnamed fixture path is unused. One worker exercises queued execution;
     * three workers allow concurrent analyzer tasks. Five rounds alternate normal,
     * empty, and whitespace input to check that repeated calls retain no stale results.
     * @throws std::runtime_error If an expectation fails.
     * @note Unexpected exceptions propagate to the per-case runner.
     */
    void testText(const fs::path&) {
        for (const size_t workers : {1u, 3u}) {
            TextAnalysisSystem system(workers);
            for (int repeat = 0; repeat < 5; ++repeat) {
                verify(system.analyzeText("Good good. Bad!"));
                verify(system.analyzeText(""), true);
                verify(system.analyzeText(" \t\n"), true);
            }
        }
    }

    /**
     * @brief Rejects a pool without workers.
     * @details The unnamed fixture path is unused. expectThrows consumes the expected
     * std::invalid_argument from construction with zero workers.
     * @throws std::runtime_error If an expectation fails.
     * @note Unexpected exceptions propagate to the per-case runner.
     */
    void testZeroWorkers(const fs::path&) {
        expectThrows<std::invalid_argument>([] { TextAnalysisSystem system(0); });
    }

    /**
     * @brief Reads real files, including a valid empty file.
     * @param[in] dir Isolated, initially empty directory reserved for this case.
     * @details A multiline fixture must match direct text analysis. An empty file must
     * still return three typed payloads, rather than the empty map used for errors.
     * @throws std::runtime_error If an expectation fails.
     * @note Unexpected exceptions propagate to the per-case runner.
     */
    void testFiles(const fs::path& dir) {
        TextAnalysisSystem system(2);
        write(dir / "sample.txt", "Good good.\nBad!");
        write(dir / "empty.txt", "");
        verify(system.analyzeFile((dir / "sample.txt").string()));
        verify(system.analyzeFile((dir / "empty.txt").string()), true);
    }

    /**
     * @brief Checks a missing file returns no results and the system remains usable.
     * @param[in] dir Isolated, initially empty directory reserved for this case.
     * @details The missing file deliberately triggers an application error message.
     * A subsequent valid analysis checks recovery using the same system.
     * @throws std::runtime_error If an expectation fails.
     * @note Unexpected exceptions propagate to the per-case runner.
     */
    void testMissingFile(const fs::path& dir) {
        TextAnalysisSystem system(1);
        check(system.analyzeFile((dir / "missing.txt").string()).empty(),
            "Missing file must return an empty map");
        verify(system.analyzeText("Good good. Bad!"));
    }

    /**
     * @brief Checks reports, extension filtering, non-recursion, and overwrite behavior.
     * @param[in] dir Isolated, initially empty directory reserved for this case.
     * @details Checks all report sections, two decimal places, sorted word frequencies,
     * and empty-file output through the public batch API. Only top-level .txt
     * inputs may generate reports; nested files and other extensions are excluded.
     * One worker exercises batch execution without spare workers. A second run
     * must replace stale output deterministically and preserve the input file.
     * @throws std::runtime_error If an expectation fails.
     * @note Unexpected exceptions propagate to the per-case runner.
     */
    void testBatch(const fs::path& dir) {
        const auto input = dir / "input";
        const auto output = dir / "nested" / "reports";
        fs::create_directories(input / "subdir");
        write(input / "sample.txt", "Good good. Bad!");
        write(input / "empty.txt", "");
        write(input / "ignored.csv", "bad");
        write(input / "subdir" / "hidden.txt", "bad");
        TextAnalysisSystem system(1);
        system.batchProcess(input.string(), ".txt", output.string());
        check(std::distance(fs::directory_iterator(output), fs::directory_iterator{}) == 2,
            "Expected only two reports, without nested files or other extensions");  // ToDo
        const auto report = read(output / "sample_report.txt");
        for (const auto* fragment : {
            "Text Analysis Report\n====================\n", "Readability Analyzer\n",
            "Total words: 3\n", "Total sentences: 2\n", "Total syllables: 3\n",
            "Average sentence length (words): 1.50\n",
            "Average word length (syllables): 1.00\n", "Flesch reading ease: 120.71\n",
            "Sentiment Analyzer\n",
            "Sentiment score [-1, 1]: 0.33\n", "Word Frequency Analyzer\n  bad: 1\n  good: 2\n"}) {
            check(report.find(fragment) != std::string::npos,
                std::string("Missing report content: ") + fragment);
        }
        /** @brief The exact grade is -3.205; binary rounding can land on either side. */
        check(report.find("Flesch-Kincaid grade: -3.20\n") != std::string::npos
            || report.find("Flesch-Kincaid grade: -3.21\n") != std::string::npos,
            "Unexpected formatted grade");
        const auto empty = read(output / "empty_report.txt");
        check(empty.find("No words found.\n") != std::string::npos
            && empty.find("Total words: 0\n") != std::string::npos
            && empty.find("Sentiment score [-1, 1]: 0.00\n") != std::string::npos,
            "Empty input must still produce an analysis report");
        write(output / "sample_report.txt", "STALE CONTENT");
        system.batchProcess(input.string(), ".txt", output.string());
        check(read(output / "sample_report.txt") == report, "Report was not overwritten deterministically");
        check(read(input / "sample.txt") == "Good good. Bad!", "Input was modified");
    }

    /**
     * @brief Prepares the output directory even when the filter matches no files.
     * @param[in] dir Isolated, initially empty directory reserved for this case.
     * @details Only a .csv file exists. Selecting .txt must still create an empty output folder.
     * @throws std::runtime_error If an expectation fails.
     * @note Unexpected exceptions propagate to the per-case runner.
     */
    void testNoMatches(const fs::path& dir) {
        fs::create_directory(dir / "input");
        write(dir / "input" / "skip.csv", "good");
        TextAnalysisSystem system(1);
        system.batchProcess((dir / "input").string(), ".txt", (dir / "output").string());
        check(fs::is_directory(dir / "output") && fs::is_empty(dir / "output"),
            "No matches must create an empty output directory");
    }

    /**
     * @brief Propagates batch setup errors and rejects input/output aliases.
     * @param[in] dir Isolated, initially empty directory reserved for this case.
     * @details Checks missing input, equivalent input/output paths (dir and dir/.),
     * and a regular file occupying the output-directory path. Each expected
     * exception is checked independently using expectThrows.
     * @throws std::runtime_error If an expectation fails.
     * @note Unexpected exceptions propagate to the per-case runner.
     */
    void testBatchErrors(const fs::path& dir) {
        TextAnalysisSystem system(1);
        expectThrows<fs::filesystem_error>([&] {
            system.batchProcess((dir / "missing").string(), ".txt", (dir / "out").string());
        });
        expectThrows<std::invalid_argument>([&] {
            system.batchProcess(dir.string(), ".txt", (dir / ".").string());
        });
        write(dir / "file", "occupied");
        expectThrows<fs::filesystem_error>([&] {
            system.batchProcess(dir.string(), ".txt", (dir / "file").string());
        });
    }

    /**
     * @brief A blocked report destination does not prevent other reports being saved.
     * @param[in] dir Isolated, initially empty directory reserved for this case.
     * @details A directory named blocked_report.txt causes a deterministic write failure
     * without platform-specific permission changes. The batch must log this error
     * and still save the other report, regardless of input enumeration order.
     * @throws std::runtime_error If an expectation fails.
     * @note Unexpected exceptions propagate to the per-case runner.
     */
    void testContinueAfterError(const fs::path& dir) {
        fs::create_directory(dir / "input");
        fs::create_directories(dir / "output" / "blocked_report.txt");
        write(dir / "input" / "blocked.txt", "bad");
        write(dir / "input" / "valid.txt", "good");
        TextAnalysisSystem system(2);
        system.batchProcess((dir / "input").string(), ".txt", (dir / "output").string());
        check(read(dir / "output" / "valid_report.txt").find("  good: 1\n") != std::string::npos,
            "A failed report prevented another file from being processed");
        check(fs::is_directory(dir / "output" / "blocked_report.txt"), "Blocked destination changed");
    }
}
/**
 * @brief Executes all registered integration cases and reports their outcomes.
 * @details Runs each registered case in its own numbered subdirectory. An
 * exception fails that case, is reported, and does not stop subsequent cases.
 * Setup or explicit cleanup errors also mark the suite as failed. Temporary
 * directory ownership provides fallback cleanup during stack unwinding.
 * @return Zero on success, or one if any case, setup, or checked cleanup fails.
 */
int Tests::runTextAnalysisSystemTests() {
    /** @brief Associates a diagnostic name with a test function. */
    struct TestCase {
        const char* name; ///< Name displayed in pass/fail diagnostics.
        /**
         * @brief Pointer to a function returning void and taking const fs::path&.
         * @details Invoked as test.run(directory). The argument identifies the
         * isolated fixture directory. Returning normally indicates success;
         * throwing an exception reports failure to the surrounding runner.
         */
        void (*run)(const fs::path&);
    };
    /** @brief Registers a diagnostic name and callback for every test case. */
    const TestCase cases[]{
        {"Text, empty input, repeated calls and worker counts", testText},
        {"Zero workers throws", testZeroWorkers},
        {"Real and empty files", testFiles},
        {"Missing file and recovery", testMissingFile},
        {"Batch reports, filtering and overwrite", testBatch},
        {"Batch without matching files", testNoMatches},
        {"Batch setup errors", testBatchErrors},
        {"Continue after a report write failure", testContinueAfterError}
    };

    int passed = 0; ///< Cases that returned without throwing.
    int failed = 0; ///< Failed cases plus setup or checked-cleanup failures.
    std::cout << "TextAnalysisSystem tests\n";
    try {
        TemporaryDirectory temporary;
        for (const auto& test : cases) {
            try {
                /**
                 * @brief Gives each case a separate numbered fixture directory.
                 * @details Every completed case increments exactly one counter,
                 * so passed + failed identifies the next case without path reuse.
                 */
                const auto directory = temporary.path() / std::to_string(passed + failed);
                fs::create_directory(directory);
                test.run(directory);
                ++passed;
                std::cout << "[PASS] " << test.name << '\n';
            }
            catch (const std::exception& error) {
                ++failed;
                std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
            }
            catch (...) {
                ++failed;
                std::cerr << "[FAIL] " << test.name << ": unknown exception\n";
            }
        }
        /** @brief Checks cleanup explicitly so removal errors affect suite status. */
        temporary.cleanup();
    }
    catch (const std::exception& error) {
        ++failed;
        std::cerr << "[FAIL] Test setup or cleanup: " << error.what() << '\n';
    }
    std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
#endif

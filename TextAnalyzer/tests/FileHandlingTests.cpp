/**
 * @file FileHandlingTests.cpp
 * @brief Implements isolated filesystem tests enabled by FILE_HANDLING_RUN_TESTS.
 */
#include "../RunConfig.h"

#if FILE_HANDLING_RUN_TESTS
#include "FileHandlingTests.h"
#include "../FileHandling.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

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

    /** @brief Owns a newly created temporary directory and cleans it on exit. */
    class TemporaryDirectory {
    public:
        /**
         * @brief Creates and owns a new directory for temporary test files.
         * @throws std::filesystem::filesystem_error If a filesystem operation fails.
         * @throws std::runtime_error If the parent check fails or all names are taken.
         */
        TemporaryDirectory() {
            // Get the system temporary directory as an absolute, normalized path.
            // Lexical normalization simplifies the path without resolving symlinks.
            const auto root = fs::absolute(fs::temp_directory_path()).lexically_normal();
            // Use a clock tick count to reduce name collisions, not as a calendar date.
            // This value alone does not guarantee a unique directory name.
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            // Try up to 100 names, changing the numeric suffix after a collision.
            for (int attempt = 0; attempt < 100; ++attempt) {
                // fs::path overloads operator / to compose paths, not divide numbers.
                // Here it appends a relative folder name to root, handling separators.
                // Example: fs::path("C:/Temp") / "test" represents C:/Temp/test.
                // It returns a new path without modifying root or creating a directory.
                const auto candidate = root / ("TextAnalyzer-tests-" +
                    std::to_string(stamp) + "-" + std::to_string(attempt));
                // parent_path() extracts the candidate's parent, without the new name.
                // equivalent() queries the filesystem to check that both paths refer
                // to the same directory, rather than comparing their text spelling.
                // We compare the existing parent with root, not the uncreated candidate.
                // A query error can throw; a false result makes check() throw.
                check(fs::equivalent(candidate.parent_path(), root),
                      "Invalid temporary directory");
                // true: a new directory was created; false: a directory already exists.
                // Other creation errors throw instead of retrying with a different name.
                if (fs::create_directory(candidate)) {
                    // Own only a directory created by this instance, for later cleanup.
                    path_ = candidate;
                    // Finish construction successfully; constructors return no value.
                    return;
                }
            }
            // All candidate names were already occupied by existing directories.
            throw std::runtime_error("Could not create a unique test directory");
        }

        TemporaryDirectory(const TemporaryDirectory&) = delete;
        TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

        /** @brief Removes the owned temporary directory and reports cleanup errors. */
        ~TemporaryDirectory() {
            std::error_code error;
            // Recursively remove the directory, its files, and its subdirectories.
            // error is passed by non-const reference (std::error_code&), allowing
            // remove_all to report filesystem errors through it instead of throwing
            // a filesystem_error. On success, the error code is cleared.
            // If path_ no longer exists (e.g. cleanup() already removed it), that is
            // not an error: remove_all removes nothing and returns zero.
            fs::remove_all(path_, error);
            // Report a cleanup failure without throwing a filesystem exception.
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

        /** @brief Performs checked cleanup before reporting suite success. */
        void cleanup() { fs::remove_all(path_); }

    private:
        fs::path path_;
    };

    /**
     * @brief Creates a file with known contents to prepare a test fixture.
     * @details Uses std::ofstream directly so that read tests do not depend on
     * FileHandling::writeTextFile being implemented correctly.
     * @param path Destination file path, passed by const reference to avoid copying.
     * @param content Text to write, passed by const reference to avoid copying.
     * @throws std::ios_base::failure If opening, writing, or closing the file fails.
     */
    void createFixture(const fs::path& path, const std::string& content) {
        /**
         * @brief Opens the file for text output, creating it if it does not exist.
         * @details An existing file is truncated: its previous contents are erased.
         * Parent directories are not created. A missing parent or insufficient
         * permissions can cause opening to fail and set the stream error state.
         */
        std::ofstream stream(path);

        /**
         * @brief Enables exceptions for failed operations and serious I/O errors.
         * @details failbit indicates an operation failure; badbit indicates a
         * serious I/O failure. Bitwise OR combines both flags into one mask.
         * Setting this mask also checks the current state, so an earlier opening
         * failure causes this call to throw std::ios_base::failure immediately.
         */
        stream.exceptions(std::ios::failbit | std::ios::badbit);

        /** @brief Writes the text to the stream; detected failures now throw. */
        stream << content;

        /**
         * @brief Flushes pending output and closes the file explicitly.
         * @details The destructor would also close the file automatically, but
         * an explicit close allows closing errors to be reported as exceptions.
         */
        stream.close();
    }

    /**
     * @brief Checks that reading preserves text and line breaks.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     */
    void testRead(const fs::path& directory) {
        const auto file = directory / "input.txt";
        createFixture(file, "Hello\nWorld\n");
        check(FileHandling::readTextFile(file.string()) == "Hello\nWorld\n",
              "Reading must preserve text and line breaks");
    }

    /**
     * @brief Checks file creation, contents, and reading back written text.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     */
    void testWriteAndRead(const fs::path& directory) {
        const auto file = directory / "output.txt";
        const std::string expected = "Hello\nWorld\n";
        FileHandling::writeTextFile(file.string(), expected);
        std::ifstream stream(file);
        /**
         * @brief A failed check stops this test and propagates to the test runner.
         * @details check() throws std::runtime_error when its condition is false.
         * There is no local catch, so the remaining statements in testWriteAndRead
         * are skipped. Constructed local objects, including stream, are destroyed
         * as the stack unwinds. The exception reaches the catch in
         * runFileHandlingTests(), which surrounds the test.run(directory) call.
         * The runner skips ++passed, records the failure, and continues with the
         * next test. Execution never resumes after the failed check in this test.
         * This behavior applies to every check() call below; a successful check
         * returns normally and allows the next statement to execute.
         */
        check(stream.is_open(), "Writing must create the file");
        const std::string actual{std::istreambuf_iterator<char>(stream),
                                 std::istreambuf_iterator<char>()};
        check(actual == expected, "Written text must match the expected contents");
        check(FileHandling::readTextFile(file.string()) == expected,
              "Reading back written text must preserve its contents");
    }

    /**
     * @brief Checks that overwriting removes all previous contents.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     */
    void testOverwrite(const fs::path& directory) {
        const auto file = directory / "overwrite.txt";
        createFixture(file, "A much longer original message");
        FileHandling::writeTextFile(file.string(), "Short");
        check(FileHandling::readTextFile(file.string()) == "Short",
              "Overwriting must remove the previous contents completely");
    }

    /**
     * @brief Checks that writing and reading empty text produces an empty file.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     */
    void testEmptyFile(const fs::path& directory) {
        const auto file = directory / "empty.txt";
        FileHandling::writeTextFile(file.string(), "");
        check(fs::is_regular_file(file) && fs::file_size(file) == 0,
              "Writing empty text must create an empty file");
        check(FileHandling::readTextFile(file.string()).empty(),
              "Reading an empty file must return an empty string");
    }

    /**
     * @brief Checks that reading a missing file throws std::runtime_error.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     */
    void testMissingFile(const fs::path& directory) {
        expectThrows<std::runtime_error>([&] {
            FileHandling::readTextFile((directory / "missing.txt").string());
        });
    }

    /**
     * @brief Checks that writing with a missing parent throws std::runtime_error.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     */
    void testWriteMissingParent(const fs::path& directory) {
        expectThrows<std::runtime_error>([&] {
            FileHandling::writeTextFile((directory / "missing" / "file.txt").string(), "text");
        });
    }

    /**
     * @brief Checks exact, case-sensitive extension filtering without recursion.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     * @details Only a.txt and b.txt should match. The CSV file, upper.TXT,
     * the folder named folder.txt, and its nested file must be excluded.
     * Sorting both vectors makes the comparison independent of listing order.
     * A second query verifies that an unmatched extension returns no files.
     */
    void testListFilesByExtension(const fs::path& directory) {
        createFixture(directory / "b.txt", "B");
        createFixture(directory / "a.txt", "A");
        createFixture(directory / "other.csv", "C");
        createFixture(directory / "upper.TXT", "D");
		fs::create_directory(directory / "folder.txt");  // a folder with a .txt extension  
        createFixture(directory / "folder.txt" / "nested.txt", "Nested");

        auto actual = FileHandling::listFiles(directory.string(), ".txt");
        std::vector<std::string> expected{
            (directory / "a.txt").string(), (directory / "b.txt").string()};
        std::sort(actual.begin(), actual.end());
        std::sort(expected.begin(), expected.end());
        check(actual == expected,
              "Listing must filter exactly and exclude directories and nested files");
        check(FileHandling::listFiles(directory.string(), ".pdf").empty(),
              "No matching extensions must produce an empty result");
    }

    /**
     * @brief Checks that an empty directory produces no matching files.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     */
    void testEmptyDirectory(const fs::path& directory) {
        check(FileHandling::listFiles(directory.string(), ".txt").empty(),
              "An empty directory must produce an empty result");
    }

    /**
     * @brief Checks that an empty extension selects only files without an extension.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     */
    void testEmptyExtension(const fs::path& directory) {
        createFixture(directory / "README", "No extension");
        createFixture(directory / "notes.txt", "With extension");
        const auto actual = FileHandling::listFiles(directory.string(), "");
        check(actual == std::vector<std::string>{(directory / "README").string()},
              "An empty extension must select files without an extension");
    }

    /**
     * @brief Checks that listing a missing directory throws std::filesystem::filesystem_error.
     * @param directory Existing, empty directory reserved for this test.
     * @throws std::runtime_error If an expectation fails.
     */
    void testMissingDirectory(const fs::path& directory) {
        expectThrows<fs::filesystem_error>([&] {
            FileHandling::listFiles((directory / "missing").string(), ".txt");
        });
    }
}

/**
 * @details Runs each registered case in its own numbered subdirectory. An
 * exception fails that case, is reported, and does not stop subsequent cases.
 * Setup or explicit cleanup errors also mark the suite as failed. Temporary
 * directory ownership provides fallback cleanup during stack unwinding.
 * @return Zero on success, or one if any case, setup, or checked cleanup fails.
 */
int Tests::runFileHandlingTests() {
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
    const TestCase cases[]{
        {"Read existing text", testRead},
        {"Write and read", testWriteAndRead},
        {"Overwrite contents", testOverwrite},
        {"Empty file", testEmptyFile},
        {"Missing file throws", testMissingFile},
        {"Missing destination parent throws", testWriteMissingParent},
        {"Filter files by extension", testListFilesByExtension},
        {"Empty directory", testEmptyDirectory},
        {"Empty extension", testEmptyExtension},
        {"Missing directory throws", testMissingDirectory}
    };

    int passed = 0;
    int failed = 0;
    std::cout << "FileHandling tests\n";
    try {
        TemporaryDirectory temporary;
        for (const auto& test : cases) {
            try {
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

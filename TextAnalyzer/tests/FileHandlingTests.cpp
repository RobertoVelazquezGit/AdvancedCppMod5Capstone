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

    /** @brief Reports a failed expectation in both Debug and Release builds. */
    void check(bool condition, const std::string& message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    /** @brief Requires the callable to throw the specified exception type. */
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

        const fs::path& path() const { return path_; }

        /** @brief Performs checked cleanup before reporting suite success. */
        void cleanup() { fs::remove_all(path_); }

    private:
        fs::path path_;
    };

    // ToDo

    /** @brief Creates input independently of the production write function. */
    void createFixture(const fs::path& path, const std::string& content) {
        std::ofstream stream(path);
        stream.exceptions(std::ios::failbit | std::ios::badbit);
        stream << content;
        stream.close();
    }

    void testRead(const fs::path& directory) {
        const auto file = directory / "input.txt";
        createFixture(file, "Hello\nWorld\n");
        check(FileHandling::readTextFile(file.string()) == "Hello\nWorld\n",
              "Reading must preserve text and line breaks");
    }

    void testWriteAndRead(const fs::path& directory) {
        const auto file = directory / "output.txt";
        const std::string expected = "Hello\nWorld\n";
        FileHandling::writeTextFile(file.string(), expected);
        std::ifstream stream(file);
        check(stream.is_open(), "Writing must create the file");
        const std::string actual{std::istreambuf_iterator<char>(stream),
                                 std::istreambuf_iterator<char>()};
        check(actual == expected, "Written text must match the expected contents");
        check(FileHandling::readTextFile(file.string()) == expected,
              "Reading back written text must preserve its contents");
    }

    void testOverwrite(const fs::path& directory) {
        const auto file = directory / "overwrite.txt";
        createFixture(file, "A much longer original message");
        FileHandling::writeTextFile(file.string(), "Short");
        check(FileHandling::readTextFile(file.string()) == "Short",
              "Overwriting must remove the previous contents completely");
    }

    void testEmptyFile(const fs::path& directory) {
        const auto file = directory / "empty.txt";
        FileHandling::writeTextFile(file.string(), "");
        check(fs::is_regular_file(file) && fs::file_size(file) == 0,
              "Writing empty text must create an empty file");
        check(FileHandling::readTextFile(file.string()).empty(),
              "Reading an empty file must return an empty string");
    }

    void testMissingFile(const fs::path& directory) {
        expectThrows<std::runtime_error>([&] {
            FileHandling::readTextFile((directory / "missing.txt").string());
        });
    }

    void testWriteMissingParent(const fs::path& directory) {
        expectThrows<std::runtime_error>([&] {
            FileHandling::writeTextFile((directory / "missing" / "file.txt").string(), "text");
        });
    }

    void testListFilesByExtension(const fs::path& directory) {
        createFixture(directory / "b.txt", "B");
        createFixture(directory / "a.txt", "A");
        createFixture(directory / "other.csv", "C");
        createFixture(directory / "upper.TXT", "D");
        fs::create_directory(directory / "folder.txt");
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

    void testEmptyDirectory(const fs::path& directory) {
        check(FileHandling::listFiles(directory.string(), ".txt").empty(),
              "An empty directory must produce an empty result");
    }

    void testEmptyExtension(const fs::path& directory) {
        createFixture(directory / "README", "No extension");
        createFixture(directory / "notes.txt", "With extension");
        const auto actual = FileHandling::listFiles(directory.string(), "");
        check(actual == std::vector<std::string>{(directory / "README").string()},
              "An empty extension must select files without an extension");
    }

    void testMissingDirectory(const fs::path& directory) {
        expectThrows<fs::filesystem_error>([&] {
            FileHandling::listFiles((directory / "missing").string(), ".txt");
        });
    }
}

int Tests::runFileHandlingTests() {
    struct TestCase {
        const char* name;
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

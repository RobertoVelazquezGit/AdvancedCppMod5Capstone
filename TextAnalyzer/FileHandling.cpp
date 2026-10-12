#include "FileHandling.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

/** @file FileHandling.cpp
 * @brief Implementation of FileHandling file operations.
 */
namespace FileHandling {
    /**
     * @brief Reads the file using an input stream and an in-memory buffer.
     * @see FileHandling.h for this function's public contract.
     */
    std::string readTextFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    /**
     * @brief Opens the destination in text mode, truncates it, and writes the contents.
     * @see FileHandling.h for parameters and checked errors.
     */
    void writeTextFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file) {
            throw std::runtime_error("Could not open file for writing: " + filename);
        }

        file << content;
        if (!file) {
            throw std::runtime_error("Error writing to file: " + filename);
        }
    }

    /**
     * @brief Scans one directory level and filters entries by type and extension.
     * @details The range-based for loop uses the free begin/end functions
     * for directory_iterator. The end iterator marks completion of traversal.
     * @see FileHandling.h for this function's public contract.
     */
    std::vector<std::string> listFiles(const std::string& directory, const std::string& extension) {
        std::vector<std::string> files;

        /**
         * @brief Traverses directory entries without building a container of them.
         * @details std::filesystem::directory_iterator(directory) constructs an
         * input iterator positioned at the first entry, or at the end if empty.
         * It visits one directory level; it does not recurse into subdirectories.
         *
         * A range-based for loop does not require a container. It requires a
         * range with begin/end support and suitable iterator operations.
         * std::filesystem provides free begin() and end() overloads for
         * directory_iterator, found through argument-dependent lookup (ADL).
         * begin() returns the starting iterator; end() returns a default-constructed
         * iterator representing the end of traversal. The loop compares iterators,
         * dereferences the current one, and increments it to visit the next entry.
         *
         * Each entry is a const reference to the current directory_entry, which
         * can represent a regular file, a directory, or another filesystem object.
         */
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                files.push_back(entry.path().string());
            }
        }

        return files;
    }
}





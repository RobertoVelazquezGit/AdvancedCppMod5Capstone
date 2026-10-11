#pragma once

#include <string>
#include <vector>

/** @brief File reading, writing, and listing operations. */
namespace FileHandling {
    /**
     * @brief Reads a file's contents in text mode.
     * @param filename Path to the file to read.
     * @return Contents extracted from the file; empty if the file is empty.
     * @throws std::runtime_error If the file cannot be opened.
     * @note Subsequent read errors are not explicitly checked.
     */
    std::string readTextFile(const std::string& filename);
    /**
     * @brief Writes text to a file, replacing any existing contents.
     * @param filename Destination path; the file is created if it does not exist.
     * @param content Text to write.
     * @throws std::runtime_error If opening fails or an error is detected
     * while inserting the text into the stream.
     * @note Errors during file closure are not explicitly checked.
     */
    void writeTextFile(const std::string& filename, const std::string& content);
    /**
     * @brief Lists regular files whose extensions match exactly.
     * @param directory Path to the directory to scan.
     * @param extension Extension including the dot, e.g. ".txt"; an empty
     * string selects paths with an empty extension.
     * @return Matching paths in unspecified order; empty if there are no
     * matches. Paths are not automatically converted to absolute paths.
     * @throws std::filesystem::filesystem_error If a filesystem operation
     * used to iterate over or inspect entries fails.
     * @note Does not recurse into subdirectories. Matching is case-sensitive.
     * Symbolic links to regular files may also be included.
     */
    std::vector<std::string> listFiles(const std::string& directory, const std::string& extension);
}

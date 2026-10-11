#pragma once

namespace Tests {
    /**
     * @brief Runs repeatable FileHandling tests in an isolated temporary directory.
     * @return Zero if all tests and cleanup succeed, or one otherwise.
     */
    int runFileHandlingTests();
}

#pragma once

/** @brief Set to 1 to enable the FileHandling test suite, or 0 to disable it. */
#ifndef FILE_HANDLING_RUN_TESTS
#define FILE_HANDLING_RUN_TESTS 0
#endif

#if FILE_HANDLING_RUN_TESTS != 0 && FILE_HANDLING_RUN_TESTS != 1
#error FILE_HANDLING_RUN_TESTS must be 0 or 1
#endif

/** @brief Set to 1 to enable WordFrequencyAnalyzer tests, or 0 to disable them. */
#ifndef WORDFREQUENCYANALYZER_RUN_TESTS
#define WORDFREQUENCYANALYZER_RUN_TESTS 0
#endif

#if WORDFREQUENCYANALYZER_RUN_TESTS != 0 && WORDFREQUENCYANALYZER_RUN_TESTS != 1
#error WORDFREQUENCYANALYZER_RUN_TESTS must be 0 or 1
#endif

/** @brief Set to 1 to enable ReadabilityAnalyzer tests, or 0 to disable them. */
#ifndef READABILITYANALYZER_RUN_TESTS
#define READABILITYANALYZER_RUN_TESTS 0
#endif

#if READABILITYANALYZER_RUN_TESTS != 0 && READABILITYANALYZER_RUN_TESTS != 1
#error READABILITYANALYZER_RUN_TESTS must be 0 or 1
#endif

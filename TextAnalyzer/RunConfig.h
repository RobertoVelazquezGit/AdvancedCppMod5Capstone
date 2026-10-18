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

/** @brief Set to 1 to enable SentimentAnalyzer tests, or 0 to disable them. */
#ifndef SENTIMENTANALYZER_RUN_TESTS
#define SENTIMENTANALYZER_RUN_TESTS 0
#endif

#if SENTIMENTANALYZER_RUN_TESTS != 0 && SENTIMENTANALYZER_RUN_TESTS != 1
#error SENTIMENTANALYZER_RUN_TESTS must be 0 or 1
#endif

/** @brief Set to 1 to enable factory and engine tests, or 0 to disable them. */
#ifndef FACTORYANALYZER_RUN_TESTS
#define FACTORYANALYZER_RUN_TESTS 0
#endif

#if FACTORYANALYZER_RUN_TESTS != 0 && FACTORYANALYZER_RUN_TESTS != 1
#error FACTORYANALYZER_RUN_TESTS must be 0 or 1
#endif

/** @brief Set to 1 to enable ThreadPool tests, or 0 to disable them. */
#ifndef TREADPOOL_RUN_TESTS
#define TREADPOOL_RUN_TESTS 1
#endif

#if TREADPOOL_RUN_TESTS != 0 && TREADPOOL_RUN_TESTS != 1
#error TREADPOOL_RUN_TESTS must be 0 or 1
#endif
#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

#include <stdio.h>
#include <stdlib.h>

extern unsigned int g_test_count;
extern unsigned int g_test_failures;

#define TEST_ASSERT_TRUE(condition)                                                \
    do                                                                             \
    {                                                                              \
        if (!(condition))                                                          \
        {                                                                          \
            fprintf(stderr, "%s:%d: assertion failed: %s\n",                    \
                    __FILE__, __LINE__, #condition);                               \
            g_test_failures++;                                                     \
            return;                                                                \
        }                                                                          \
    } while (0)

#define TEST_ASSERT_EQ_UINT(expected, actual)                                      \
    do                                                                             \
    {                                                                              \
        const unsigned long expected_value = (unsigned long)(expected);            \
        const unsigned long actual_value = (unsigned long)(actual);                \
        if (expected_value != actual_value)                                        \
        {                                                                          \
            fprintf(stderr, "%s:%d: expected %lu, got %lu\n",                   \
                    __FILE__, __LINE__, expected_value, actual_value);              \
            g_test_failures++;                                                     \
            return;                                                                \
        }                                                                          \
    } while (0)

#define TEST_ASSERT_EQ_PTR(expected, actual)                                       \
    do                                                                             \
    {                                                                              \
        const void *expected_value = (const void *)(expected);                     \
        const void *actual_value = (const void *)(actual);                         \
        if (expected_value != actual_value)                                        \
        {                                                                          \
            fprintf(stderr, "%s:%d: expected pointer %p, got %p\n",             \
                    __FILE__, __LINE__, expected_value, actual_value);              \
            g_test_failures++;                                                     \
            return;                                                                \
        }                                                                          \
    } while (0)

#define RUN_TEST(function)                                                         \
    do                                                                             \
    {                                                                              \
        const unsigned int failures_before = g_test_failures;                      \
        g_test_count++;                                                            \
        function();                                                                \
        if (g_test_failures == failures_before)                                    \
        {                                                                          \
            printf("[PASS] %s\n", #function);                                    \
        }                                                                          \
        else                                                                       \
        {                                                                          \
            printf("[FAIL] %s\n", #function);                                    \
        }                                                                          \
    } while (0)

#endif /* TEST_SUPPORT_H */

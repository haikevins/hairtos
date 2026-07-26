#include <stdint.h>

#include "hr_benchmark.h"
#include "test_support.h"

static void test_benchmark_stats_empty_state(void)
{
    hr_benchmark_stats_t stats;

    hr_benchmark_stats_reset(&stats);
    TEST_ASSERT_EQ_UINT(0U, hr_benchmark_stats_count(&stats));
    TEST_ASSERT_EQ_UINT(0U, hr_benchmark_stats_min(&stats));
    TEST_ASSERT_EQ_UINT(0U, hr_benchmark_stats_max(&stats));
    TEST_ASSERT_EQ_UINT(0U, hr_benchmark_stats_mean(&stats));
    TEST_ASSERT_EQ_UINT(0U, hr_benchmark_stats_percentile(&stats, 95U));
}

static void test_benchmark_stats_summary_and_percentiles(void)
{
    hr_benchmark_stats_t stats;
    static const uint32_t samples[] = {40U, 10U, 50U, 20U, 30U};
    size_t index;

    hr_benchmark_stats_reset(&stats);
    for (index = 0U; index < (sizeof(samples) / sizeof(samples[0])); index++)
    {
        TEST_ASSERT_TRUE(hr_benchmark_stats_record(&stats, samples[index]));
    }

    TEST_ASSERT_EQ_UINT(5U, hr_benchmark_stats_count(&stats));
    TEST_ASSERT_EQ_UINT(10U, hr_benchmark_stats_min(&stats));
    TEST_ASSERT_EQ_UINT(50U, hr_benchmark_stats_max(&stats));
    TEST_ASSERT_EQ_UINT(30U, hr_benchmark_stats_mean(&stats));
    TEST_ASSERT_EQ_UINT(30U, hr_benchmark_stats_percentile(&stats, 50U));
    TEST_ASSERT_EQ_UINT(50U, hr_benchmark_stats_percentile(&stats, 95U));
    TEST_ASSERT_EQ_UINT(50U, hr_benchmark_stats_percentile(&stats, 100U));
}

static void test_benchmark_stats_capacity_is_bounded(void)
{
    hr_benchmark_stats_t stats;
    size_t index;

    hr_benchmark_stats_reset(&stats);
    for (index = 0U; index < (size_t)HR_BENCHMARK_MAX_SAMPLES; index++)
    {
        TEST_ASSERT_TRUE(hr_benchmark_stats_record(&stats, (uint32_t)index));
    }

    TEST_ASSERT_TRUE(!hr_benchmark_stats_record(&stats, 99U));
    TEST_ASSERT_EQ_UINT(HR_BENCHMARK_MAX_SAMPLES,
                        hr_benchmark_stats_count(&stats));
}

static void test_benchmark_cycle_helpers_handle_wrap_and_conversion(void)
{
    TEST_ASSERT_EQ_UINT(32U,
                        hr_benchmark_elapsed_cycles(UINT32_MAX - 15U, 16U));
    TEST_ASSERT_EQ_UINT(80U, hr_benchmark_adjust_cycles(100U, 20U));
    TEST_ASSERT_EQ_UINT(0U, hr_benchmark_adjust_cycles(10U, 20U));
    TEST_ASSERT_EQ_UINT(1000U,
                        hr_benchmark_cycles_to_nanoseconds(72U, 72000000U));
    TEST_ASSERT_EQ_UINT(0U,
                        hr_benchmark_cycles_to_nanoseconds(72U, 0U));
}

void run_benchmark_tests(void)
{
    RUN_TEST(test_benchmark_stats_empty_state);
    RUN_TEST(test_benchmark_stats_summary_and_percentiles);
    RUN_TEST(test_benchmark_stats_capacity_is_bounded);
    RUN_TEST(test_benchmark_cycle_helpers_handle_wrap_and_conversion);
}

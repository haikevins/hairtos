#include "phase16_stress.h"
#include "test_support.h"

static void test_phase16_scheduler_stress_preserves_invariants(void)
{
    phase16_stress_result_t result;

    TEST_ASSERT_TRUE(phase16_scheduler_stress_run(50000U, &result));
    TEST_ASSERT_EQ_UINT(50000U, result.iterations);
    TEST_ASSERT_EQ_UINT(50000U, result.validations);
    TEST_ASSERT_TRUE(result.insertions > 1000U);
    TEST_ASSERT_TRUE(result.removals > 1000U);
    TEST_ASSERT_TRUE(result.rotations > 1000U);
}

void run_phase16_stress_tests(void)
{
    RUN_TEST(test_phase16_scheduler_stress_preserves_invariants);
}

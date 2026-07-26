#include <stdbool.h>

#include "hairtos/hr_semaphore.h"
#include "hr_semaphore_internal.h"
#include "test_support.h"

extern bool g_mock_inside_isr;

static void test_counting_semaphore_nonblocking_counts(void)
{
    static hr_semaphore_t semaphore;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_semaphore_create_counting(&semaphore, 2U, 3U));
    TEST_ASSERT_TRUE(hr_semaphore_is_valid(&semaphore));
    TEST_ASSERT_TRUE(hr_semaphore_validate_internal(&semaphore));
    TEST_ASSERT_EQ_UINT(2U, hr_semaphore_get_count(&semaphore));
    TEST_ASSERT_EQ_UINT(3U, hr_semaphore_get_max_count(&semaphore));

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_semaphore_take(&semaphore, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_semaphore_take(&semaphore, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_ERROR_SEMAPHORE_EMPTY,
                        hr_semaphore_take(&semaphore, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_semaphore_give(&semaphore));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_semaphore_give(&semaphore));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_semaphore_give(&semaphore));
    TEST_ASSERT_EQ_UINT(HR_ERROR_SEMAPHORE_FULL,
                        hr_semaphore_give(&semaphore));
}

static void test_binary_semaphore_and_isr_context_rules(void)
{
    static hr_semaphore_t semaphore;
    bool higher_priority_task_woken = true;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_semaphore_create_binary(&semaphore, false));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_semaphore_give_from_isr(
                            &semaphore,
                            &higher_priority_task_woken));

    g_mock_inside_isr = true;
    TEST_ASSERT_EQ_UINT(HR_ERROR_FROM_ISR,
                        hr_semaphore_give(&semaphore));
    TEST_ASSERT_EQ_UINT(HR_ERROR_FROM_ISR,
                        hr_semaphore_take(&semaphore, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_semaphore_give_from_isr(
                            &semaphore,
                            &higher_priority_task_woken));
    TEST_ASSERT_TRUE(!higher_priority_task_woken);
    g_mock_inside_isr = false;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_semaphore_take(&semaphore, HR_NO_WAIT));
}

static void test_semaphore_rejects_invalid_creation(void)
{
    static hr_semaphore_t semaphore;

    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_semaphore_create_counting(NULL, 0U, 1U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_semaphore_create_counting(&semaphore, 2U, 1U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_semaphore_create_counting(&semaphore, 0U, 0U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_semaphore_create_counting(&semaphore, 0U, 2U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_semaphore_create_counting(&semaphore, 0U, 2U));
}

void run_semaphore_tests(void)
{
    RUN_TEST(test_counting_semaphore_nonblocking_counts);
    RUN_TEST(test_binary_semaphore_and_isr_context_rules);
    RUN_TEST(test_semaphore_rejects_invalid_creation);
}

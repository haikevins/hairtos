#include <stdbool.h>

#include "hairtos/hr_mutex.h"
#include "hr_mutex_internal.h"
#include "test_support.h"

extern bool g_mock_inside_isr;

static void test_mutex_creation_and_metadata(void)
{
    static hr_mutex_t normal_mutex;
    static hr_mutex_t recursive_mutex;

    TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_create(&normal_mutex));
    TEST_ASSERT_TRUE(hr_mutex_is_valid(&normal_mutex));
    TEST_ASSERT_TRUE(hr_mutex_validate_internal(&normal_mutex));
    TEST_ASSERT_TRUE(!hr_mutex_is_recursive(&normal_mutex));
    TEST_ASSERT_EQ_PTR(NULL, hr_mutex_get_owner(&normal_mutex));
    TEST_ASSERT_EQ_UINT(0U, hr_mutex_get_recursion_count(&normal_mutex));
    TEST_ASSERT_EQ_UINT(0U, hr_mutex_get_waiting_tasks(&normal_mutex));

    TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_create_recursive(&recursive_mutex));
    TEST_ASSERT_TRUE(hr_mutex_is_recursive(&recursive_mutex));
    TEST_ASSERT_TRUE(hr_mutex_validate_internal(&recursive_mutex));
}

static void test_mutex_rejects_invalid_context(void)
{
    static hr_mutex_t mutex;

    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT, hr_mutex_create(NULL));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_create(&mutex));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE, hr_mutex_create(&mutex));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_mutex_lock(&mutex, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE, hr_mutex_unlock(&mutex));

    g_mock_inside_isr = true;
    TEST_ASSERT_EQ_UINT(HR_ERROR_FROM_ISR,
                        hr_mutex_lock(&mutex, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_ERROR_FROM_ISR, hr_mutex_unlock(&mutex));
    g_mock_inside_isr = false;
}

void run_mutex_tests(void)
{
    RUN_TEST(test_mutex_creation_and_metadata);
    RUN_TEST(test_mutex_rejects_invalid_context);
}

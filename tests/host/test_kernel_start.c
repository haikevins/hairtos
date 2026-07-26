#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_kernel_internal.h"
#include "hr_task_internal.h"
#include "test_support.h"

extern unsigned int g_mock_context_switch_requests;

static void phase5_dummy_task(void *argument)
{
    (void)argument;
}

static void test_kernel_uses_priority_scheduler_and_rotates_equal_high_tasks(void)
{
    static hr_task_t low_task;
    static hr_task_t task_a;
    static hr_task_t task_b;
    static hr_stack_t low_stack[64];
    static hr_stack_t stack_a[64];
    static hr_stack_t stack_b[64];
    const hr_task_control_block_t *control_block;

    g_mock_context_switch_requests = 0U;
    hr_task_yield();
    TEST_ASSERT_EQ_UINT(0U, g_mock_context_switch_requests);


    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&low_task,
                                              "phase6-low",
                                              phase5_dummy_task,
                                              NULL,
                                              low_stack,
                                              64U,
                                              5U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&task_a,
                                              "phase5-a",
                                              phase5_dummy_task,
                                              NULL,
                                              stack_a,
                                              64U,
                                              2U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&task_b,
                                              "phase5-b",
                                              phase5_dummy_task,
                                              NULL,
                                              stack_b,
                                              64U,
                                              2U));

    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE, hr_task_start(&task_a));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_init());
    TEST_ASSERT_EQ_UINT(1U, hr_kernel_get_task_count());

    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&low_task));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&task_a));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&task_b));
    TEST_ASSERT_EQ_UINT(4U, hr_kernel_get_task_count());

    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_prepare_start());
    TEST_ASSERT_EQ_PTR(&task_a, hr_task_current());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING, hr_task_get_state(&task_a));
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&task_b));
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&low_task));

    control_block = hr_task_control_block_const(&task_a);
    TEST_ASSERT_EQ_PTR(control_block, g_hr_current_task_control_block);

    hr_task_yield();
    TEST_ASSERT_EQ_UINT(1U, g_mock_context_switch_requests);
    TEST_ASSERT_EQ_PTR(&task_a, hr_task_current());

    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&task_b, hr_task_current());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&task_a));
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING, hr_task_get_state(&task_b));
    TEST_ASSERT_EQ_PTR(hr_task_control_block_const(&task_b),
                       g_hr_current_task_control_block);

    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&task_a, hr_task_current());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING, hr_task_get_state(&task_a));
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&task_b));
}

void run_kernel_start_tests(void)
{
    RUN_TEST(test_kernel_uses_priority_scheduler_and_rotates_equal_high_tasks);
}

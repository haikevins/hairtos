#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_kernel_internal.h"
#include "hr_task_internal.h"
#include "test_support.h"

static void phase4_dummy_task(void *argument)
{
    (void)argument;
}

static void test_kernel_initializes_idle_and_selects_highest_ready_task(void)
{
    static hr_task_t application_task;
    static hr_stack_t application_stack[64];
    const hr_task_control_block_t *control_block;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&application_task,
                                              "phase4-app",
                                              phase4_dummy_task,
                                              NULL,
                                              application_stack,
                                              64U,
                                              2U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_task_start(&application_task));

    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_init());
    TEST_ASSERT_EQ_UINT(HR_KERNEL_STATE_INITIALIZED, hr_kernel_get_state());
    TEST_ASSERT_EQ_UINT(1U, hr_kernel_get_task_count());
    TEST_ASSERT_TRUE(!hr_kernel_is_running());
    TEST_ASSERT_EQ_PTR(NULL, hr_task_current());

    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&application_task));
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY,
                        hr_task_get_state(&application_task));
    TEST_ASSERT_EQ_UINT(2U, hr_kernel_get_task_count());

    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_task_start(&application_task));

    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_prepare_start());
    TEST_ASSERT_TRUE(hr_kernel_is_running());
    TEST_ASSERT_EQ_UINT(HR_KERNEL_STATE_RUNNING, hr_kernel_get_state());
    TEST_ASSERT_EQ_PTR(&application_task, hr_task_current());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING,
                        hr_task_get_state(&application_task));

    control_block = hr_task_control_block_const(&application_task);
    TEST_ASSERT_EQ_PTR(control_block, g_hr_current_task_control_block);
}

void run_kernel_start_tests(void)
{
    RUN_TEST(test_kernel_initializes_idle_and_selects_highest_ready_task);
}

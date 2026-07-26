#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hr_kernel_internal.h"
#include "hr_task_internal.h"
#include "test_support.h"

extern unsigned int g_mock_context_switch_requests;
extern bool g_mock_inside_isr;

static void phase8_dummy_task(void *argument)
{
    (void)argument;
}

static void test_kernel_preemption_round_robin_and_delay_race(void)
{
    static hr_task_t low_task;
    static hr_task_t high_a;
    static hr_task_t high_b;
    static hr_task_t reserved_idle_priority_task;
    static hr_stack_t low_stack[64];
    static hr_stack_t high_a_stack[64];
    static hr_stack_t high_b_stack[64];
    static hr_stack_t reserved_stack[64];
    hr_tick_t missed_anchor;

    g_mock_context_switch_requests = 0U;
    g_mock_inside_isr = false;
    hr_task_yield();
    TEST_ASSERT_EQ_UINT(0U, g_mock_context_switch_requests);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_init());
    TEST_ASSERT_EQ_UINT(1U, hr_kernel_get_task_count());

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&low_task,
                                              "phase8-low",
                                              phase8_dummy_task,
                                              NULL,
                                              low_stack,
                                              64U,
                                              5U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&high_a,
                                              "phase8-high-a",
                                              phase8_dummy_task,
                                              NULL,
                                              high_a_stack,
                                              64U,
                                              1U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&high_b,
                                              "phase8-high-b",
                                              phase8_dummy_task,
                                              NULL,
                                              high_b_stack,
                                              64U,
                                              1U));

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&reserved_idle_priority_task,
                                              "reserved-idle-priority",
                                              phase8_dummy_task,
                                              NULL,
                                              reserved_stack,
                                              64U,
                                              (hr_priority_t)HR_CFG_IDLE_PRIORITY));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_task_start(&reserved_idle_priority_task));

    /* Registration order must not override fixed priority. */
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&low_task));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&high_a));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&high_b));
    TEST_ASSERT_EQ_UINT(4U, hr_kernel_get_task_count());

    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_prepare_start());
    TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

    /* With a one-tick quantum, equal-priority CPU-bound tasks alternate. */
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(1U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());

    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(2U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

    /* Both high tasks block; the lower-priority task becomes runnable. */
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(5U));
    TEST_ASSERT_EQ_UINT(3U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());

    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(3U));
    TEST_ASSERT_EQ_UINT(4U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&low_task, hr_task_current());

    hr_kernel_tick_from_isr();
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(4U, hr_time_now());
    TEST_ASSERT_EQ_UINT(4U, g_mock_context_switch_requests);

    /* High B wakes at tick 5 and preempts Low without waiting for yield. */
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(5U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&high_b));
    TEST_ASSERT_EQ_UINT(5U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&low_task));

    /* No equal-priority peer exists yet, so High B keeps running. */
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(6U, hr_time_now());
    TEST_ASSERT_EQ_UINT(5U, g_mock_context_switch_requests);

    /* High A wakes at tick 7; the equal-priority quantum rotates to it. */
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(7U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&high_a));
    TEST_ASSERT_EQ_UINT(6U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

    /* A missed delay-until deadline returns immediately. */
    missed_anchor = 1U;
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay_until(&missed_anchor, 2U));
    TEST_ASSERT_EQ_UINT(3U, missed_anchor);
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING, hr_task_get_state(&high_a));

    /* One-tick timeout can expire before the pending block PendSV executes. */
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(1U));
    TEST_ASSERT_EQ_UINT(7U, g_mock_context_switch_requests);
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_BLOCKED, hr_task_get_state(&high_a));
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(8U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&high_a));
    TEST_ASSERT_EQ_UINT(7U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());

    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(8U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

    g_mock_inside_isr = true;
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE, hr_task_delay(1U));
    g_mock_inside_isr = false;
}

void run_kernel_start_tests(void)
{
    RUN_TEST(test_kernel_preemption_round_robin_and_delay_race);
}

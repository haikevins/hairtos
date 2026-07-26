#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hr_kernel_internal.h"
#include "hr_task_internal.h"
#include "test_support.h"

extern unsigned int g_mock_context_switch_requests;
extern bool g_mock_inside_isr;

static void phase7_dummy_task(void *argument)
{
    (void)argument;
}

static void test_kernel_priority_yield_delay_and_timeout_wakeup(void)
{
    static hr_task_t task_a;
    static hr_task_t task_b;
    static hr_stack_t stack_a[64];
    static hr_stack_t stack_b[64];
    hr_tick_t periodic_anchor;

    g_mock_context_switch_requests = 0U;
    g_mock_inside_isr = false;
    hr_task_yield();
    TEST_ASSERT_EQ_UINT(0U, g_mock_context_switch_requests);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_init());
    TEST_ASSERT_EQ_UINT(1U, hr_kernel_get_task_count());

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&task_a,
                                              "phase7-a",
                                              phase7_dummy_task,
                                              NULL,
                                              stack_a,
                                              64U,
                                              2U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&task_b,
                                              "phase7-b",
                                              phase7_dummy_task,
                                              NULL,
                                              stack_b,
                                              64U,
                                              2U));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&task_a));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&task_b));
    TEST_ASSERT_EQ_UINT(3U, hr_kernel_get_task_count());

    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_prepare_start());
    TEST_ASSERT_EQ_PTR(&task_a, hr_task_current());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING, hr_task_get_state(&task_a));

    hr_task_yield();
    TEST_ASSERT_EQ_UINT(1U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&task_b, hr_task_current());

    /* B blocks until tick 3; A becomes current. */
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(3U));
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_BLOCKED, hr_task_get_state(&task_b));
    TEST_ASSERT_EQ_UINT(2U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&task_a, hr_task_current());

    /* A blocks until tick 5; only idle remains ready. */
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(5U));
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_BLOCKED, hr_task_get_state(&task_a));
    TEST_ASSERT_EQ_UINT(3U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_TRUE(hr_task_current() != &task_a);
    TEST_ASSERT_TRUE(hr_task_current() != &task_b);
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING,
                        hr_task_get_state(hr_task_current()));

    hr_kernel_tick_from_isr();
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(2U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_BLOCKED, hr_task_get_state(&task_b));
    TEST_ASSERT_EQ_UINT(3U, g_mock_context_switch_requests);

    /* Tick 3 wakes B and requests a switch because idle is running. */
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(3U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&task_b));
    TEST_ASSERT_EQ_UINT(4U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&task_b, hr_task_current());

    /* Tick 5 wakes A, but Phase 7 does not time-slice/preempt running B. */
    hr_kernel_tick_from_isr();
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(5U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&task_a));
    TEST_ASSERT_EQ_PTR(&task_b, hr_task_current());
    TEST_ASSERT_EQ_UINT(4U, g_mock_context_switch_requests);

    hr_task_yield();
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&task_a, hr_task_current());

    periodic_anchor = hr_time_now();
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay_until(&periodic_anchor, 4U));
    TEST_ASSERT_EQ_UINT(9U, periodic_anchor);
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_BLOCKED, hr_task_get_state(&task_a));
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&task_b, hr_task_current());

    hr_kernel_tick_from_isr();
    hr_kernel_tick_from_isr();
    hr_kernel_tick_from_isr();
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(9U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&task_a));

    /* Missed periodic deadline returns immediately without blocking. */
    periodic_anchor = 1U;
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay_until(&periodic_anchor, 2U));
    TEST_ASSERT_EQ_UINT(3U, periodic_anchor);
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING, hr_task_get_state(&task_b));

    /* A one-tick delay may expire before its pending PendSV is serviced. */
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(1U));
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_BLOCKED, hr_task_get_state(&task_b));
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&task_b));
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&task_a, hr_task_current());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING, hr_task_get_state(&task_a));

    g_mock_inside_isr = true;
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE, hr_task_delay(1U));
    g_mock_inside_isr = false;
}

void run_kernel_start_tests(void)
{
    RUN_TEST(test_kernel_priority_yield_delay_and_timeout_wakeup);
}

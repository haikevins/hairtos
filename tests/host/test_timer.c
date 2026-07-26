#include <stdbool.h>
#include <stdint.h>

#include "hairtos/hr_kernel.h"
#include "hairtos/hr_timer.h"
#include "hr_timer_internal.h"
#include "test_support.h"

extern bool g_mock_inside_isr;

typedef struct
{
    uint32_t count;
    uint32_t id;
} timer_counter_t;

static uint32_t g_callback_order[8];
static size_t g_callback_order_count;

static void count_callback(void *argument)
{
    timer_counter_t *counter = (timer_counter_t *)argument;
    counter->count++;
    if (g_callback_order_count < 8U)
    {
        g_callback_order[g_callback_order_count] = counter->id;
        g_callback_order_count++;
    }
}

static void advance_timer_clock(hr_tick_t *now, hr_tick_t ticks)
{
    hr_tick_t index;

    g_mock_inside_isr = true;
    for (index = 0U; index < ticks; index++)
    {
        bool switch_required = false;
        (*now)++;
        hr_timer_tick_from_isr(*now, &switch_required);
    }
    g_mock_inside_isr = false;
}

static void drain_callbacks(void)
{
    while (hr_timer_process_one_pending())
    {
    }
}

static void test_one_shot_and_periodic_timers(void)
{
    static hr_timer_t one_shot;
    static hr_timer_t periodic;
    static timer_counter_t one_shot_counter = {0U, 1U};
    static timer_counter_t periodic_counter = {0U, 2U};
    hr_tick_t now = hr_kernel_get_tick();

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_timer_create_static(&one_shot,
                                               "one-shot",
                                               3U,
                                               false,
                                               count_callback,
                                               &one_shot_counter));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_timer_create_static(&periodic,
                                               "periodic",
                                               2U,
                                               true,
                                               count_callback,
                                               &periodic_counter));
    TEST_ASSERT_TRUE(hr_timer_validate_internal(&one_shot));
    TEST_ASSERT_TRUE(hr_timer_validate_internal(&periodic));

    TEST_ASSERT_EQ_UINT(HR_OK, hr_timer_start(&one_shot));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timer_start(&periodic));
    TEST_ASSERT_TRUE(hr_timer_is_active(&one_shot));
    TEST_ASSERT_TRUE(hr_timer_is_active(&periodic));

    advance_timer_clock(&now, 2U);
    TEST_ASSERT_EQ_UINT(0U, one_shot_counter.count);
    TEST_ASSERT_EQ_UINT(1U, hr_timer_get_pending_count(&periodic));
    drain_callbacks();
    TEST_ASSERT_EQ_UINT(1U, periodic_counter.count);

    advance_timer_clock(&now, 1U);
    TEST_ASSERT_TRUE(!hr_timer_is_active(&one_shot));
    TEST_ASSERT_EQ_UINT(1U, hr_timer_get_pending_count(&one_shot));
    drain_callbacks();
    TEST_ASSERT_EQ_UINT(1U, one_shot_counter.count);

    advance_timer_clock(&now, 3U);
    drain_callbacks();
    TEST_ASSERT_EQ_UINT(3U, periodic_counter.count);
    TEST_ASSERT_TRUE(hr_timer_is_active(&periodic));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timer_stop(&periodic));
    TEST_ASSERT_TRUE(!hr_timer_is_active(&periodic));
}

static void test_timer_reset_change_period_stop_and_fifo(void)
{
    static hr_timer_t first;
    static hr_timer_t second;
    static timer_counter_t first_counter = {0U, 11U};
    static timer_counter_t second_counter = {0U, 22U};
    hr_tick_t now = hr_kernel_get_tick() + 100U;

    g_callback_order_count = 0U;
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_timer_create_static(&first,
                                               "first",
                                               4U,
                                               false,
                                               count_callback,
                                               &first_counter));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_timer_create_static(&second,
                                               "second",
                                               4U,
                                               false,
                                               count_callback,
                                               &second_counter));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timer_start(&first));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timer_start(&second));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE, hr_timer_start(&first));

    advance_timer_clock(&now, 4U);
    TEST_ASSERT_EQ_UINT(1U, hr_timer_get_pending_count(&first));
    TEST_ASSERT_EQ_UINT(1U, hr_timer_get_pending_count(&second));
    drain_callbacks();
    TEST_ASSERT_EQ_UINT(2U, g_callback_order_count);
    TEST_ASSERT_EQ_UINT(11U, g_callback_order[0]);
    TEST_ASSERT_EQ_UINT(22U, g_callback_order[1]);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_timer_reset(&first));
    advance_timer_clock(&now, 2U);
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timer_reset(&first));
    advance_timer_clock(&now, 3U);
    TEST_ASSERT_EQ_UINT(0U, hr_timer_get_pending_count(&first));
    advance_timer_clock(&now, 1U);
    drain_callbacks();
    TEST_ASSERT_EQ_UINT(2U, first_counter.count);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_timer_change_period(&second, 2U));
    TEST_ASSERT_EQ_UINT(2U, hr_timer_get_period(&second));
    advance_timer_clock(&now, 1U);
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timer_stop(&second));
    advance_timer_clock(&now, 2U);
    drain_callbacks();
    TEST_ASSERT_EQ_UINT(1U, second_counter.count);

    g_mock_inside_isr = true;
    TEST_ASSERT_EQ_UINT(HR_ERROR_FROM_ISR, hr_timer_reset(&first));
    TEST_ASSERT_EQ_UINT(HR_ERROR_FROM_ISR, hr_timer_stop(&first));
    g_mock_inside_isr = false;
}

void run_timer_tests(void)
{
    RUN_TEST(test_one_shot_and_periodic_timers);
    RUN_TEST(test_timer_reset_change_period_stop_and_fifo);
}

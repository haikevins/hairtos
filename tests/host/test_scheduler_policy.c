#include "hr_scheduler_internal.h"
#include "test_support.h"

typedef struct
{
    unsigned int id;
    hr_ready_node_t ready;
} scheduler_task_t;

static void scheduler_task_init(scheduler_task_t *task,
                                unsigned int id,
                                hr_priority_t priority)
{
    task->id = id;
    hr_ready_node_init(&task->ready, task, priority);
}

static void test_scheduler_ignores_registration_order_across_priorities(void)
{
    hr_scheduler_t scheduler;
    scheduler_task_t low;
    scheduler_task_t high;
    scheduler_task_t medium;

    hr_scheduler_init(&scheduler);
    scheduler_task_init(&low, 1U, 6U);
    scheduler_task_init(&high, 2U, 1U);
    scheduler_task_init(&medium, 3U, 3U);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &low.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &medium.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &high.ready));

    TEST_ASSERT_EQ_PTR(&high.ready, hr_scheduler_select_highest(&scheduler));
    TEST_ASSERT_EQ_UINT(3U, hr_scheduler_ready_count(&scheduler));
    TEST_ASSERT_TRUE(hr_scheduler_validate(&scheduler));
}

static void test_scheduler_yield_rotates_only_highest_priority_fifo(void)
{
    hr_scheduler_t scheduler;
    scheduler_task_t low;
    scheduler_task_t high_a;
    scheduler_task_t high_b;

    hr_scheduler_init(&scheduler);
    scheduler_task_init(&low, 1U, 5U);
    scheduler_task_init(&high_a, 2U, 1U);
    scheduler_task_init(&high_b, 3U, 1U);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &low.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &high_a.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &high_b.ready));

    TEST_ASSERT_EQ_PTR(&high_a.ready, hr_scheduler_select_highest(&scheduler));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_scheduler_yield_current(&scheduler, &high_a.ready));
    TEST_ASSERT_EQ_PTR(&high_b.ready, hr_scheduler_select_highest(&scheduler));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_scheduler_yield_current(&scheduler, &high_b.ready));
    TEST_ASSERT_EQ_PTR(&high_a.ready, hr_scheduler_select_highest(&scheduler));

    /* The lower-priority task remains READY but is never selected. */
    TEST_ASSERT_TRUE(low.ready.node.list != NULL);
    TEST_ASSERT_TRUE(hr_scheduler_validate(&scheduler));
}

static void test_scheduler_single_highest_task_yields_to_itself(void)
{
    hr_scheduler_t scheduler;
    scheduler_task_t low;
    scheduler_task_t high;

    hr_scheduler_init(&scheduler);
    scheduler_task_init(&low, 1U, 5U);
    scheduler_task_init(&high, 2U, 1U);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &low.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &high.ready));
    TEST_ASSERT_EQ_PTR(&high.ready, hr_scheduler_select_highest(&scheduler));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_scheduler_yield_current(&scheduler, &high.ready));
    TEST_ASSERT_EQ_PTR(&high.ready, hr_scheduler_select_highest(&scheduler));
}

static void test_scheduler_rejects_yield_from_non_selected_task(void)
{
    hr_scheduler_t scheduler;
    scheduler_task_t low;
    scheduler_task_t high;

    hr_scheduler_init(&scheduler);
    scheduler_task_init(&low, 1U, 5U);
    scheduler_task_init(&high, 2U, 1U);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &low.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_scheduler_add_ready(&scheduler, &high.ready));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_scheduler_yield_current(&scheduler, &low.ready));
    TEST_ASSERT_EQ_PTR(&high.ready, hr_scheduler_select_highest(&scheduler));
    TEST_ASSERT_TRUE(hr_scheduler_validate(&scheduler));
}

void run_scheduler_policy_tests(void)
{
    RUN_TEST(test_scheduler_ignores_registration_order_across_priorities);
    RUN_TEST(test_scheduler_yield_rotates_only_highest_priority_fifo);
    RUN_TEST(test_scheduler_single_highest_task_yields_to_itself);
    RUN_TEST(test_scheduler_rejects_yield_from_non_selected_task);
}

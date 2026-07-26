#include "hr_scheduler_internal.h"
#include "test_support.h"

typedef struct
{
    int id;
    hr_ready_node_t ready;
} fake_task_t;

static void fake_task_init(fake_task_t *task, int id, hr_priority_t priority)
{
    task->id = id;
    hr_ready_node_init(&task->ready, task, priority);
}

static void test_ready_set_selects_smallest_priority_number(void)
{
    hr_ready_set_t set;
    fake_task_t low;
    fake_task_t high;
    fake_task_t medium;

    hr_ready_set_init(&set);
    fake_task_init(&low, 1, 6U);
    fake_task_init(&high, 2, 1U);
    fake_task_init(&medium, 3, 3U);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_insert(&set, &low.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_insert(&set, &high.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_insert(&set, &medium.ready));

    TEST_ASSERT_EQ_PTR(&high.ready, hr_ready_set_peek_highest(&set));
    TEST_ASSERT_TRUE(hr_ready_set_validate(&set));
}

static void test_ready_set_equal_priority_is_fifo_and_rotates(void)
{
    hr_ready_set_t set;
    fake_task_t first;
    fake_task_t second;
    fake_task_t third;

    hr_ready_set_init(&set);
    fake_task_init(&first, 1, 2U);
    fake_task_init(&second, 2, 2U);
    fake_task_init(&third, 3, 2U);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_insert(&set, &first.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_insert(&set, &second.ready));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_insert(&set, &third.ready));
    TEST_ASSERT_EQ_PTR(&first.ready, hr_ready_set_peek_highest(&set));

    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_rotate_highest(&set));
    TEST_ASSERT_EQ_PTR(&second.ready, hr_ready_set_peek_highest(&set));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_rotate_highest(&set));
    TEST_ASSERT_EQ_PTR(&third.ready, hr_ready_set_peek_highest(&set));
    TEST_ASSERT_TRUE(hr_ready_set_validate(&set));
}

static void test_ready_set_remove_clears_bitmap_bit(void)
{
    hr_ready_set_t set;
    fake_task_t task;

    hr_ready_set_init(&set);
    fake_task_init(&task, 1, 4U);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_insert(&set, &task.ready));
    TEST_ASSERT_TRUE((hr_ready_set_bitmap(&set) & (UINT32_C(1) << 4U)) != 0U);
    TEST_ASSERT_EQ_UINT(HR_OK, hr_ready_set_remove(&set, &task.ready));
    TEST_ASSERT_EQ_UINT(0U, hr_ready_set_bitmap(&set));
    TEST_ASSERT_TRUE(hr_ready_set_is_empty(&set));
    TEST_ASSERT_TRUE(hr_ready_set_validate(&set));
}

static void test_ready_set_rejects_invalid_priority(void)
{
    hr_ready_set_t set;
    fake_task_t task;

    hr_ready_set_init(&set);
    fake_task_init(&task, 1, (hr_priority_t)HR_CFG_PRIORITY_COUNT);

    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_ready_set_insert(&set, &task.ready));
    TEST_ASSERT_TRUE(hr_ready_set_validate(&set));
}

void run_ready_set_tests(void)
{
    RUN_TEST(test_ready_set_selects_smallest_priority_number);
    RUN_TEST(test_ready_set_equal_priority_is_fifo_and_rotates);
    RUN_TEST(test_ready_set_remove_clears_bitmap_bit);
    RUN_TEST(test_ready_set_rejects_invalid_priority);
}

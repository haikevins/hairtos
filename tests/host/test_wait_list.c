#include "hr_wait_internal.h"
#include "test_support.h"

typedef struct
{
    int id;
    hr_wait_node_t wait;
} waiter_t;

static void waiter_init(waiter_t *waiter, int id, hr_priority_t priority)
{
    waiter->id = id;
    hr_wait_node_init(&waiter->wait, waiter, priority);
}

static void test_wait_list_orders_by_priority_then_fifo(void)
{
    hr_wait_list_t list;
    waiter_t low;
    waiter_t high_first;
    waiter_t medium;
    waiter_t high_second;

    hr_wait_list_init(&list);
    waiter_init(&low, 1, 6U);
    waiter_init(&high_first, 2, 1U);
    waiter_init(&medium, 3, 3U);
    waiter_init(&high_second, 4, 1U);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_wait_list_insert(&list, &low.wait));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_wait_list_insert(&list, &high_first.wait));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_wait_list_insert(&list, &medium.wait));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_wait_list_insert(&list, &high_second.wait));

    TEST_ASSERT_EQ_PTR(&high_first.wait, hr_wait_list_pop(&list));
    TEST_ASSERT_EQ_PTR(&high_second.wait, hr_wait_list_pop(&list));
    TEST_ASSERT_EQ_PTR(&medium.wait, hr_wait_list_pop(&list));
    TEST_ASSERT_EQ_PTR(&low.wait, hr_wait_list_pop(&list));
    TEST_ASSERT_TRUE(hr_wait_list_is_empty(&list));
    TEST_ASSERT_TRUE(hr_wait_list_validate(&list));
}

static void test_wait_list_remove_unlinks_specific_waiter(void)
{
    hr_wait_list_t list;
    waiter_t first;
    waiter_t second;

    hr_wait_list_init(&list);
    waiter_init(&first, 1, 2U);
    waiter_init(&second, 2, 3U);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_wait_list_insert(&list, &first.wait));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_wait_list_insert(&list, &second.wait));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_wait_list_remove(&list, &first.wait));
    TEST_ASSERT_EQ_PTR(&second.wait, hr_wait_list_peek(&list));
    TEST_ASSERT_EQ_UINT(1U, hr_wait_list_size(&list));
    TEST_ASSERT_TRUE(hr_wait_list_validate(&list));
}

void run_wait_list_tests(void)
{
    RUN_TEST(test_wait_list_orders_by_priority_then_fifo);
    RUN_TEST(test_wait_list_remove_unlinks_specific_waiter);
}

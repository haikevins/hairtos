#include <stdint.h>

#include "hr_timeout_internal.h"
#include "test_support.h"

typedef struct
{
    int id;
    hr_timeout_node_t timeout;
} timed_item_t;

static void timed_item_init(timed_item_t *item, int id)
{
    item->id = id;
    hr_timeout_node_init(&item->timeout, item);
}

static timed_item_t *expired_item(hr_list_node_t *node)
{
    return (timed_item_t *)hr_list_node_owner(node);
}

static void test_timeout_list_expires_in_deadline_order(void)
{
    hr_timeout_list_t timeouts;
    hr_list_t expired;
    timed_item_t late;
    timed_item_t early;
    timed_item_t equal;

    hr_timeout_list_init(&timeouts, 100U);
    hr_list_init(&expired);
    timed_item_init(&late, 1);
    timed_item_init(&early, 2);
    timed_item_init(&equal, 3);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_insert(&timeouts, &late.timeout, 20U));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_insert(&timeouts, &early.timeout, 5U));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_insert(&timeouts, &equal.timeout, 5U));

    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_advance(&timeouts, 105U, &expired));
    TEST_ASSERT_EQ_PTR(&early, expired_item(hr_list_pop_front(&expired)));
    TEST_ASSERT_EQ_PTR(&equal, expired_item(hr_list_pop_front(&expired)));
    TEST_ASSERT_EQ_UINT(1U, hr_timeout_list_size(&timeouts));

    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_advance(&timeouts, 120U, &expired));
    TEST_ASSERT_EQ_PTR(&late, expired_item(hr_list_pop_front(&expired)));
    TEST_ASSERT_TRUE(hr_timeout_list_is_empty(&timeouts));
    TEST_ASSERT_TRUE(hr_timeout_list_validate(&timeouts));
}

static void test_timeout_list_handles_tick_wrap(void)
{
    hr_timeout_list_t timeouts;
    hr_list_t expired;
    timed_item_t before_wrap;
    timed_item_t after_wrap;
    const hr_tick_t start = UINT32_MAX - UINT32_C(3);

    hr_timeout_list_init(&timeouts, start);
    hr_list_init(&expired);
    timed_item_init(&before_wrap, 1);
    timed_item_init(&after_wrap, 2);

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_timeout_list_insert(&timeouts, &before_wrap.timeout, 2U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_timeout_list_insert(&timeouts, &after_wrap.timeout, 6U));

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_timeout_list_advance(&timeouts, UINT32_MAX - UINT32_C(1),
                                                &expired));
    TEST_ASSERT_EQ_PTR(&before_wrap, expired_item(hr_list_pop_front(&expired)));

    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_advance(&timeouts, 0U, &expired));
    TEST_ASSERT_TRUE(hr_list_is_empty(&expired));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_advance(&timeouts, 2U, &expired));
    TEST_ASSERT_EQ_PTR(&after_wrap, expired_item(hr_list_pop_front(&expired)));
    TEST_ASSERT_TRUE(hr_timeout_list_is_empty(&timeouts));
    TEST_ASSERT_TRUE(hr_timeout_list_validate(&timeouts));
}

static void test_timeout_list_remove_cancels_timeout(void)
{
    hr_timeout_list_t timeouts;
    hr_list_t expired;
    timed_item_t item;

    hr_timeout_list_init(&timeouts, 10U);
    hr_list_init(&expired);
    timed_item_init(&item, 1);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_insert(&timeouts, &item.timeout, 4U));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_remove(&timeouts, &item.timeout));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_advance(&timeouts, 20U, &expired));
    TEST_ASSERT_TRUE(hr_list_is_empty(&expired));
    TEST_ASSERT_TRUE(hr_timeout_list_is_empty(&timeouts));
}

static void test_timeout_list_zero_delay_expires_at_current_tick(void)
{
    hr_timeout_list_t timeouts;
    hr_list_t expired;
    timed_item_t item;

    hr_timeout_list_init(&timeouts, 77U);
    hr_list_init(&expired);
    timed_item_init(&item, 1);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_insert(&timeouts, &item.timeout, 0U));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_timeout_list_advance(&timeouts, 77U, &expired));
    TEST_ASSERT_EQ_PTR(&item, expired_item(hr_list_pop_front(&expired)));
    TEST_ASSERT_TRUE(hr_timeout_list_is_empty(&timeouts));
}

void run_timeout_tests(void)
{
    RUN_TEST(test_timeout_list_expires_in_deadline_order);
    RUN_TEST(test_timeout_list_handles_tick_wrap);
    RUN_TEST(test_timeout_list_remove_cancels_timeout);
    RUN_TEST(test_timeout_list_zero_delay_expires_at_current_tick);
}

#include "hr_list_internal.h"
#include "test_support.h"

typedef struct
{
    int value;
    hr_list_node_t node;
} test_item_t;

static void item_init(test_item_t *item, int value)
{
    item->value = value;
    hr_list_node_init(&item->node, item);
}

static void test_list_init_is_empty_and_valid(void)
{
    hr_list_t list;

    hr_list_init(&list);

    TEST_ASSERT_TRUE(hr_list_is_empty(&list));
    TEST_ASSERT_EQ_UINT(0U, hr_list_size(&list));
    TEST_ASSERT_EQ_PTR(NULL, hr_list_front(&list));
    TEST_ASSERT_EQ_PTR(NULL, hr_list_back(&list));
    TEST_ASSERT_TRUE(hr_list_validate(&list));
}

static void test_list_push_back_preserves_fifo(void)
{
    hr_list_t list;
    test_item_t first;
    test_item_t second;
    test_item_t third;

    hr_list_init(&list);
    item_init(&first, 1);
    item_init(&second, 2);
    item_init(&third, 3);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_push_back(&list, &first.node));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_push_back(&list, &second.node));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_push_back(&list, &third.node));

    TEST_ASSERT_EQ_PTR(&first.node, hr_list_pop_front(&list));
    TEST_ASSERT_EQ_PTR(&second.node, hr_list_pop_front(&list));
    TEST_ASSERT_EQ_PTR(&third.node, hr_list_pop_front(&list));
    TEST_ASSERT_TRUE(hr_list_is_empty(&list));
    TEST_ASSERT_TRUE(hr_list_validate(&list));
}

static void test_list_remove_middle_repairs_links(void)
{
    hr_list_t list;
    test_item_t first;
    test_item_t middle;
    test_item_t last;

    hr_list_init(&list);
    item_init(&first, 1);
    item_init(&middle, 2);
    item_init(&last, 3);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_push_back(&list, &first.node));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_push_back(&list, &middle.node));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_push_back(&list, &last.node));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_remove(&middle.node));

    TEST_ASSERT_EQ_UINT(2U, hr_list_size(&list));
    TEST_ASSERT_EQ_PTR(&last.node, hr_list_next(&list, &first.node));
    TEST_ASSERT_EQ_PTR(&first.node, hr_list_previous(&list, &last.node));
    TEST_ASSERT_TRUE(!hr_list_node_is_linked(&middle.node));
    TEST_ASSERT_TRUE(hr_list_validate(&list));
}

static void test_list_rejects_double_insert_and_double_remove(void)
{
    hr_list_t list;
    test_item_t item;

    hr_list_init(&list);
    item_init(&item, 7);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_push_back(&list, &item.node));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_list_push_back(&list, &item.node));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_remove(&item.node));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE, hr_list_remove(&item.node));
    TEST_ASSERT_TRUE(hr_list_validate(&list));
}

static void test_list_insert_before_places_node_correctly(void)
{
    hr_list_t list;
    test_item_t first;
    test_item_t second;
    test_item_t inserted;

    hr_list_init(&list);
    item_init(&first, 1);
    item_init(&second, 2);
    item_init(&inserted, 3);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_push_back(&list, &first.node));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_list_push_back(&list, &second.node));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_list_insert_before(&list, &second.node, &inserted.node));

    TEST_ASSERT_EQ_PTR(&inserted.node, hr_list_next(&list, &first.node));
    TEST_ASSERT_EQ_PTR(&second.node, hr_list_next(&list, &inserted.node));
    TEST_ASSERT_TRUE(hr_list_validate(&list));
}

void run_list_tests(void)
{
    RUN_TEST(test_list_init_is_empty_and_valid);
    RUN_TEST(test_list_push_back_preserves_fifo);
    RUN_TEST(test_list_remove_middle_repairs_links);
    RUN_TEST(test_list_rejects_double_insert_and_double_remove);
    RUN_TEST(test_list_insert_before_places_node_correctly);
}

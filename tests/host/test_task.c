#include <stddef.h>

#include "hairtos/hr_task.h"
#include "hr_task_internal.h"
#include "test_support.h"

static void dummy_task(void *argument)
{
    (void)argument;
}

static void test_task_create_initializes_tcb_and_stack(void)
{
    hr_task_t task = {0};
    hr_stack_t stack[64];
    int argument = 42;
    const hr_task_control_block_t *control_block;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&task,
                                              "worker",
                                              dummy_task,
                                              &argument,
                                              stack,
                                              64U,
                                              2U));

    TEST_ASSERT_TRUE(hr_task_is_valid(&task));
    TEST_ASSERT_EQ_PTR("worker", hr_task_get_name(&task));
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_CREATED, hr_task_get_state(&task));
    TEST_ASSERT_EQ_UINT(2U, hr_task_get_base_priority(&task));
    TEST_ASSERT_EQ_UINT(2U, hr_task_get_effective_priority(&task));
    TEST_ASSERT_EQ_UINT(64U, hr_task_get_stack_words(&task));
    TEST_ASSERT_TRUE(hr_task_stack_guard_is_valid(&task));
    TEST_ASSERT_TRUE(hr_task_get_stack_high_watermark(&task) > 0U);

    control_block = hr_task_control_block_const(&task);
    TEST_ASSERT_TRUE(control_block->stack_pointer != NULL);
    TEST_ASSERT_EQ_PTR(stack, control_block->stack_low);
    TEST_ASSERT_EQ_PTR(&stack[63], control_block->stack_high);
    TEST_ASSERT_EQ_PTR(&task, hr_list_node_owner(&control_block->ready_node.node));
    TEST_ASSERT_EQ_PTR(&task, hr_list_node_owner(&control_block->wait_node.node));
    TEST_ASSERT_EQ_PTR(&task, hr_list_node_owner(&control_block->timeout_node.node));
    TEST_ASSERT_EQ_PTR(&task, hr_list_node_owner(&control_block->all_task_node));
    TEST_ASSERT_TRUE(!hr_list_node_is_linked(&control_block->ready_node.node));
    TEST_ASSERT_TRUE(!hr_list_node_is_linked(&control_block->wait_node.node));
    TEST_ASSERT_TRUE(!hr_list_node_is_linked(&control_block->timeout_node.node));
}

static void test_task_create_rejects_invalid_arguments(void)
{
    hr_task_t task = {0};
    hr_stack_t stack[64];

    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_task_create_static(NULL, "x", dummy_task, NULL,
                                              stack, 64U, 1U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_task_create_static(&task, NULL, dummy_task, NULL,
                                              stack, 64U, 1U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_task_create_static(&task, "x", NULL, NULL,
                                              stack, 64U, 1U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_task_create_static(&task, "x", dummy_task, NULL,
                                              NULL, 64U, 1U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_task_create_static(&task, "x", dummy_task, NULL,
                                              stack, HR_CFG_MIN_TASK_STACK_WORDS - 1U, 1U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_task_create_static(&task, "x", dummy_task, NULL,
                                              stack, 64U, HR_CFG_PRIORITY_COUNT));
}

static void test_task_stack_guard_detects_corruption(void)
{
    hr_task_t task = {0};
    hr_stack_t stack[64];

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&task, "guard", dummy_task, NULL,
                                              stack, 64U, 3U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_task_create_static(&task, "guard-again", dummy_task, NULL,
                                              stack, 64U, 3U));
    TEST_ASSERT_TRUE(hr_task_stack_guard_is_valid(&task));

    stack[0] ^= 1U;
    TEST_ASSERT_TRUE(!hr_task_stack_guard_is_valid(&task));
}

static void test_task_high_watermark_tracks_low_stack_use(void)
{
    hr_task_t task = {0};
    hr_stack_t stack[64];
    size_t before;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&task, "water", dummy_task, NULL,
                                              stack, 64U, 3U));

    before = hr_task_get_stack_high_watermark(&task);
    TEST_ASSERT_TRUE(before > 3U);
    stack[1] = 0U;
    TEST_ASSERT_EQ_UINT(0U, hr_task_get_stack_high_watermark(&task));
}

void run_task_tests(void)
{
    RUN_TEST(test_task_create_initializes_tcb_and_stack);
    RUN_TEST(test_task_create_rejects_invalid_arguments);
    RUN_TEST(test_task_stack_guard_detects_corruption);
    RUN_TEST(test_task_high_watermark_tracks_low_stack_use);
}

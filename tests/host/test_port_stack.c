#include <stdint.h>

#include "hr_portmacro.h"
#include "test_support.h"

static void test_port_stack_builds_expected_initial_frame(void)
{
    hr_stack_t stack[40];
    hr_stack_t *stack_pointer;

    stack_pointer = hr_port_initialize_stack_raw(stack,
                                                 40U,
                                                 0x08001235UL,
                                                 0x20000300UL,
                                                 0x08000457UL);

    TEST_ASSERT_TRUE(stack_pointer != NULL);
    TEST_ASSERT_EQ_UINT(0U, ((uintptr_t)stack_pointer % HR_PORT_STACK_ALIGNMENT_BYTES));
    TEST_ASSERT_EQ_UINT(0x44444444UL, stack_pointer[HR_PORT_FRAME_R4_INDEX]);
    TEST_ASSERT_EQ_UINT(0xBBBBBBBBUL, stack_pointer[HR_PORT_FRAME_R11_INDEX]);
    TEST_ASSERT_EQ_UINT(0x20000300UL, stack_pointer[HR_PORT_FRAME_R0_INDEX]);
    TEST_ASSERT_EQ_UINT(0x08000457UL, stack_pointer[HR_PORT_FRAME_LR_INDEX]);
    TEST_ASSERT_EQ_UINT(0x08001234UL, stack_pointer[HR_PORT_FRAME_PC_INDEX]);
    TEST_ASSERT_EQ_UINT(HR_PORT_INITIAL_XPSR, stack_pointer[HR_PORT_FRAME_XPSR_INDEX]);
}

static void test_port_stack_aligns_top_down_to_eight_bytes(void)
{
    hr_stack_t storage[41];
    hr_stack_t *stack_pointer;

    stack_pointer = hr_port_initialize_stack_raw(&storage[1],
                                                 39U,
                                                 0x08001001UL,
                                                 0x20000001UL,
                                                 0x08002001UL);

    TEST_ASSERT_TRUE(stack_pointer != NULL);
    TEST_ASSERT_EQ_UINT(0U, ((uintptr_t)stack_pointer % HR_PORT_STACK_ALIGNMENT_BYTES));
}

static void test_port_stack_rejects_invalid_or_small_storage(void)
{
    hr_stack_t stack[HR_PORT_INITIAL_FRAME_WORDS];

    TEST_ASSERT_EQ_PTR(NULL,
                       hr_port_initialize_stack_raw(NULL,
                                                    HR_PORT_INITIAL_FRAME_WORDS,
                                                    1U,
                                                    2U,
                                                    3U));
    TEST_ASSERT_EQ_PTR(NULL,
                       hr_port_initialize_stack_raw(stack,
                                                    HR_PORT_INITIAL_FRAME_WORDS - 1U,
                                                    1U,
                                                    2U,
                                                    3U));
}

void run_port_stack_tests(void)
{
    RUN_TEST(test_port_stack_builds_expected_initial_frame);
    RUN_TEST(test_port_stack_aligns_top_down_to_eight_bytes);
    RUN_TEST(test_port_stack_rejects_invalid_or_small_storage);
}

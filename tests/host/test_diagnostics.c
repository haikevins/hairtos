#include <stdint.h>
#include <string.h>

#include "hairtos/hr_diagnostics.h"
#include "hairtos/hr_task.h"
#include "hr_diagnostics_internal.h"
#include "test_support.h"

static void diagnostics_dummy_task(void *argument)
{
    (void)argument;
}

static void test_diagnostics_task_snapshot_and_stack_guard(void)
{
    hr_task_t task = {0};
    hr_stack_t stack[64];
    hr_task_diagnostics_t diagnostics;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&task,
                                              "diagnostic-task",
                                              diagnostics_dummy_task,
                                              NULL,
                                              stack,
                                              64U,
                                              2U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_diagnostics_get_task(&task, &diagnostics));
    TEST_ASSERT_EQ_PTR(&task, diagnostics.task);
    TEST_ASSERT_EQ_PTR("diagnostic-task", diagnostics.name);
    TEST_ASSERT_EQ_UINT(64U, diagnostics.stack_words);
    TEST_ASSERT_TRUE(diagnostics.stack_free_words > 0U);
    TEST_ASSERT_TRUE(diagnostics.stack_guard_valid);

    stack[0] ^= 1U;
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_diagnostics_get_task(&task, &diagnostics));
    TEST_ASSERT_TRUE(!diagnostics.stack_guard_valid);
}

static void test_diagnostics_runtime_counters_are_recorded(void)
{
    hr_runtime_statistics_t statistics;

    hr_diagnostics_reset_runtime_statistics();
    hr_diagnostics_internal_note_tick();
    hr_diagnostics_internal_note_tick();
    hr_diagnostics_internal_note_switch(UINT32_C(1), true);
    hr_diagnostics_internal_note_switch(UINT32_C(4), false);
    hr_diagnostics_internal_note_timeout_wakeup();
    hr_diagnostics_internal_note_invariant_result(true);
    hr_diagnostics_internal_note_invariant_result(false);
    hr_diagnostics_internal_note_stack_result(true);
    hr_diagnostics_internal_note_stack_result(false);

    hr_diagnostics_get_runtime_statistics(&statistics);
    TEST_ASSERT_EQ_UINT(2U, statistics.systick_count);
    TEST_ASSERT_EQ_UINT(2U, statistics.pendsv_count);
    TEST_ASSERT_EQ_UINT(1U, statistics.task_switch_count);
    TEST_ASSERT_EQ_UINT(1U, statistics.yield_count);
    TEST_ASSERT_EQ_UINT(1U, statistics.preemption_count);
    TEST_ASSERT_EQ_UINT(1U, statistics.timeout_wakeup_count);
    TEST_ASSERT_EQ_UINT(2U, statistics.invariant_check_count);
    TEST_ASSERT_EQ_UINT(1U, statistics.invariant_failure_count);
    TEST_ASSERT_EQ_UINT(2U, statistics.stack_check_count);
    TEST_ASSERT_EQ_UINT(1U, statistics.stack_failure_count);
}

static void test_diagnostics_panic_record_can_be_retained_and_cleared(void)
{
    hr_panic_record_t record;

    hr_diagnostics_clear_last_panic();
    TEST_ASSERT_TRUE(!hr_diagnostics_get_last_panic(&record));

    hr_diagnostics_record_panic(HR_PANIC_USER, "test_diagnostics.c", 123U);
    TEST_ASSERT_TRUE(hr_diagnostics_get_last_panic(&record));
    TEST_ASSERT_EQ_UINT(HR_PANIC_USER, record.reason);
    TEST_ASSERT_EQ_UINT(123U, record.source_line);
    TEST_ASSERT_TRUE(record.signature != 0U);
    TEST_ASSERT_TRUE(record.sequence > 0U);
    TEST_ASSERT_TRUE(strcmp("user",
                            hr_diagnostics_panic_reason_string(record.reason)) == 0);

    hr_diagnostics_clear_last_panic();
    TEST_ASSERT_TRUE(!hr_diagnostics_get_last_panic(&record));
}

static void test_diagnostics_fault_record_captures_frame(void)
{
    hr_fault_context_t fault = {0};
    hr_panic_record_t record;

    fault.pc = UINT32_C(0x08001234);
    fault.lr = UINT32_C(0xFFFFFFFD);
    fault.cfsr = UINT32_C(0x00010000);
    fault.exception_number = 3U;

    hr_diagnostics_record_fault(HR_PANIC_HARDFAULT, &fault);
    TEST_ASSERT_TRUE(hr_diagnostics_get_last_panic(&record));
    TEST_ASSERT_EQ_UINT(HR_PANIC_HARDFAULT, record.reason);
    TEST_ASSERT_EQ_UINT(fault.pc, record.fault.pc);
    TEST_ASSERT_EQ_UINT(fault.lr, record.fault.lr);
    TEST_ASSERT_EQ_UINT(fault.cfsr, record.fault.cfsr);
    TEST_ASSERT_EQ_UINT(3U, record.fault.exception_number);
    hr_diagnostics_clear_last_panic();
}

void run_diagnostics_tests(void)
{
    RUN_TEST(test_diagnostics_task_snapshot_and_stack_guard);
    RUN_TEST(test_diagnostics_runtime_counters_are_recorded);
    RUN_TEST(test_diagnostics_panic_record_can_be_retained_and_cleared);
    RUN_TEST(test_diagnostics_fault_record_captures_frame);
}

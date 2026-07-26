#include "test_support.h"

unsigned int g_test_count = 0U;
unsigned int g_test_failures = 0U;

void run_list_tests(void);
void run_ready_set_tests(void);
void run_wait_list_tests(void);
void run_timeout_tests(void);
void run_port_stack_tests(void);
void run_task_tests(void);
void run_kernel_start_tests(void);
void run_scheduler_policy_tests(void);
void run_queue_tests(void);
void run_semaphore_tests(void);
void run_mutex_tests(void);
void run_timer_tests(void);

int main(void)
{
    run_list_tests();
    run_ready_set_tests();
    run_wait_list_tests();
    run_timeout_tests();
    run_port_stack_tests();
    run_task_tests();
    run_queue_tests();
    run_semaphore_tests();
    run_mutex_tests();
    run_kernel_start_tests();
    run_timer_tests();
    run_scheduler_policy_tests();

    printf("\nHairRTOS completed host tests: %u run, %u failed\n",
           g_test_count,
           g_test_failures);

    return (g_test_failures == 0U) ? EXIT_SUCCESS : EXIT_FAILURE;
}

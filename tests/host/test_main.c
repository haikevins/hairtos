#include "test_support.h"

unsigned int g_test_count = 0U;
unsigned int g_test_failures = 0U;

void run_list_tests(void);
void run_ready_set_tests(void);
void run_wait_list_tests(void);
void run_timeout_tests(void);
void run_port_stack_tests(void);
void run_task_tests(void);

int main(void)
{
    run_list_tests();
    run_ready_set_tests();
    run_wait_list_tests();
    run_timeout_tests();
    run_port_stack_tests();
    run_task_tests();

    printf("\nHairRTOS completed host tests: %u run, %u failed\n",
           g_test_count,
           g_test_failures);

    return (g_test_failures == 0U) ? EXIT_SUCCESS : EXIT_FAILURE;
}

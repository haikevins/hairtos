#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hairtos/hr_time.h"
#include "hairtos/hr_queue.h"
#include "hairtos/hr_semaphore.h"
#include "hairtos/hr_mutex.h"
#include "hr_kernel_internal.h"
#include "hr_queue_internal.h"
#include "hr_semaphore_internal.h"
#include "hr_mutex_internal.h"
#include "hr_task_internal.h"
#include "test_support.h"

extern unsigned int g_mock_context_switch_requests;
extern bool g_mock_inside_isr;

static void phase8_dummy_task(void *argument)
{
    (void)argument;
}

static void test_kernel_preemption_round_robin_and_delay_race(void)
{
    static hr_task_t low_task;
    static hr_task_t high_a;
    static hr_task_t high_b;
    static hr_task_t reserved_idle_priority_task;
    static hr_stack_t low_stack[64];
    static hr_stack_t high_a_stack[64];
    static hr_stack_t high_b_stack[64];
    static hr_stack_t reserved_stack[64];
    hr_tick_t missed_anchor;

    g_mock_context_switch_requests = 0U;
    g_mock_inside_isr = false;
    hr_task_yield();
    TEST_ASSERT_EQ_UINT(0U, g_mock_context_switch_requests);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_init());
    TEST_ASSERT_EQ_UINT(1U, hr_kernel_get_task_count());

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&low_task,
                                              "phase8-low",
                                              phase8_dummy_task,
                                              NULL,
                                              low_stack,
                                              64U,
                                              5U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&high_a,
                                              "phase8-high-a",
                                              phase8_dummy_task,
                                              NULL,
                                              high_a_stack,
                                              64U,
                                              1U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&high_b,
                                              "phase8-high-b",
                                              phase8_dummy_task,
                                              NULL,
                                              high_b_stack,
                                              64U,
                                              1U));

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&reserved_idle_priority_task,
                                              "reserved-idle-priority",
                                              phase8_dummy_task,
                                              NULL,
                                              reserved_stack,
                                              64U,
                                              (hr_priority_t)HR_CFG_IDLE_PRIORITY));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_task_start(&reserved_idle_priority_task));

    /* Registration order must not override fixed priority. */
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&low_task));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&high_a));
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_start(&high_b));
    TEST_ASSERT_EQ_UINT(4U, hr_kernel_get_task_count());

    TEST_ASSERT_EQ_UINT(HR_OK, hr_kernel_prepare_start());
    TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

    /* With a one-tick quantum, equal-priority CPU-bound tasks alternate. */
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(1U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());

    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(2U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

    /* Both high tasks block; the lower-priority task becomes runnable. */
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(5U));
    TEST_ASSERT_EQ_UINT(3U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());

    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(3U));
    TEST_ASSERT_EQ_UINT(4U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&low_task, hr_task_current());

    hr_kernel_tick_from_isr();
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(4U, hr_time_now());
    TEST_ASSERT_EQ_UINT(4U, g_mock_context_switch_requests);

    /* High B wakes at tick 5 and preempts Low without waiting for yield. */
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(5U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&high_b));
    TEST_ASSERT_EQ_UINT(5U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&low_task));

    /* No equal-priority peer exists yet, so High B keeps running. */
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(6U, hr_time_now());
    TEST_ASSERT_EQ_UINT(5U, g_mock_context_switch_requests);

    /* High A wakes at tick 7; the equal-priority quantum rotates to it. */
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(7U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&high_a));
    TEST_ASSERT_EQ_UINT(6U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

    /* A missed delay-until deadline returns immediately. */
    missed_anchor = 1U;
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay_until(&missed_anchor, 2U));
    TEST_ASSERT_EQ_UINT(3U, missed_anchor);
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_RUNNING, hr_task_get_state(&high_a));

    /* One-tick timeout can expire before the pending block PendSV executes. */
    TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(1U));
    TEST_ASSERT_EQ_UINT(7U, g_mock_context_switch_requests);
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_BLOCKED, hr_task_get_state(&high_a));
    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(8U, hr_time_now());
    TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY, hr_task_get_state(&high_a));
    TEST_ASSERT_EQ_UINT(7U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());

    hr_kernel_tick_from_isr();
    TEST_ASSERT_EQ_UINT(8U, g_mock_context_switch_requests);
    hr_kernel_select_next_from_pendsv();
    TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

    g_mock_inside_isr = true;
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE, hr_task_delay(1U));
    g_mock_inside_isr = false;

    /* Phase 9: direct handoff to a blocked receiver. */
    {
        static hr_queue_t queue;
        static uint32_t queue_storage[1];
        hr_queue_control_block_t *queue_control_block;
        hr_task_control_block_t *high_a_control_block;
        hr_task_control_block_t *high_b_control_block;
        uint32_t received = 0U;
        uint32_t first = 41U;
        uint32_t second = 42U;
        unsigned int requests_before;

        TEST_ASSERT_EQ_UINT(HR_OK,
                            hr_queue_create_static(&queue,
                                                   queue_storage,
                                                   sizeof(queue_storage[0]),
                                                   1U));
        queue_control_block = hr_queue_control_block(&queue);
        high_a_control_block = hr_task_control_block(&high_a);
        high_b_control_block = hr_task_control_block(&high_b);

        TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_kernel_block_current_on_wait_list(
                &queue_control_block->receive_waiters,
                queue_control_block,
                HR_TASK_WAIT_QUEUE_RECEIVE,
                &received,
                10U));
        TEST_ASSERT_EQ_UINT(1U, hr_queue_get_waiting_receivers(&queue));
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());

        requests_before = g_mock_context_switch_requests;
        TEST_ASSERT_EQ_UINT(HR_OK,
                            hr_queue_send(&queue, &first, HR_NO_WAIT));
        TEST_ASSERT_EQ_UINT(first, received);
        TEST_ASSERT_EQ_UINT(0U, hr_queue_get_count(&queue));
        TEST_ASSERT_EQ_UINT(0U, hr_queue_get_waiting_receivers(&queue));
        TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY,
                            hr_task_get_state(&high_a));
        TEST_ASSERT_EQ_UINT(requests_before,
                            g_mock_context_switch_requests);

        /* A full queue blocks a sender. A receive atomically fills the freed
         * slot from that sender before waking it. */
        TEST_ASSERT_EQ_UINT(HR_OK,
                            hr_queue_send(&queue, &first, HR_NO_WAIT));
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_kernel_block_current_on_wait_list(
                &queue_control_block->send_waiters,
                queue_control_block,
                HR_TASK_WAIT_QUEUE_SEND,
                &second,
                HR_WAIT_FOREVER));
        TEST_ASSERT_EQ_UINT(1U, hr_queue_get_waiting_senders(&queue));
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

        received = 0U;
        TEST_ASSERT_EQ_UINT(HR_OK,
                            hr_queue_receive(&queue, &received, HR_NO_WAIT));
        TEST_ASSERT_EQ_UINT(first, received);
        TEST_ASSERT_EQ_UINT(1U, hr_queue_get_count(&queue));
        TEST_ASSERT_EQ_UINT(0U, hr_queue_get_waiting_senders(&queue));
        TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY,
                            hr_task_get_state(&high_b));
        TEST_ASSERT_EQ_UINT(HR_OK, high_b_control_block->wait_result);

        received = 0U;
        TEST_ASSERT_EQ_UINT(HR_OK,
                            hr_queue_receive(&queue, &received, HR_NO_WAIT));
        TEST_ASSERT_EQ_UINT(second, received);

        /* Finite timeout removes the task from both timeout and queue wait
         * structures, then returns it to READY with HR_ERROR_TIMEOUT. */
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_kernel_block_current_on_wait_list(
                &queue_control_block->receive_waiters,
                queue_control_block,
                HR_TASK_WAIT_QUEUE_RECEIVE,
                &received,
                2U));
        TEST_ASSERT_EQ_UINT(1U, hr_queue_get_waiting_receivers(&queue));
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());
        hr_kernel_tick_from_isr();
        hr_kernel_tick_from_isr();
        TEST_ASSERT_EQ_UINT(0U, hr_queue_get_waiting_receivers(&queue));
        TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY,
                            hr_task_get_state(&high_a));
        TEST_ASSERT_EQ_UINT(HR_ERROR_TIMEOUT,
                            high_a_control_block->wait_result);

        /* Equal-priority receivers stay FIFO. ISR send wakes them in order and
         * reports that a higher-priority task became READY. */
        {
            uint32_t high_b_received = 0U;
            uint32_t high_a_received = 0U;
            uint32_t isr_first = 71U;
            uint32_t isr_second = 72U;
            bool higher_priority_task_woken = false;

            TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());
            TEST_ASSERT_EQ_UINT(
                HR_OK,
                hr_kernel_block_current_on_wait_list(
                    &queue_control_block->receive_waiters,
                    queue_control_block,
                    HR_TASK_WAIT_QUEUE_RECEIVE,
                    &high_b_received,
                    HR_WAIT_FOREVER));
            hr_kernel_select_next_from_pendsv();
            TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());

            TEST_ASSERT_EQ_UINT(
                HR_OK,
                hr_kernel_block_current_on_wait_list(
                    &queue_control_block->receive_waiters,
                    queue_control_block,
                    HR_TASK_WAIT_QUEUE_RECEIVE,
                    &high_a_received,
                    HR_WAIT_FOREVER));
            TEST_ASSERT_EQ_UINT(2U,
                                hr_queue_get_waiting_receivers(&queue));
            hr_kernel_select_next_from_pendsv();
            TEST_ASSERT_EQ_PTR(&low_task, hr_task_current());

            g_mock_inside_isr = true;
            TEST_ASSERT_EQ_UINT(
                HR_OK,
                hr_queue_send_from_isr(&queue,
                                       &isr_first,
                                       &higher_priority_task_woken));
            TEST_ASSERT_TRUE(higher_priority_task_woken);
            TEST_ASSERT_EQ_UINT(isr_first, high_b_received);

            higher_priority_task_woken = false;
            TEST_ASSERT_EQ_UINT(
                HR_OK,
                hr_queue_send_from_isr(&queue,
                                       &isr_second,
                                       &higher_priority_task_woken));
            TEST_ASSERT_TRUE(higher_priority_task_woken);
            TEST_ASSERT_EQ_UINT(isr_second, high_a_received);
            g_mock_inside_isr = false;

            TEST_ASSERT_EQ_UINT(0U,
                                hr_queue_get_waiting_receivers(&queue));
            TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY,
                                hr_task_get_state(&high_b));
            TEST_ASSERT_EQ_UINT(HR_TASK_STATE_READY,
                                hr_task_get_state(&high_a));
        }

        TEST_ASSERT_TRUE(hr_queue_validate_internal(&queue));
    }

    /* Phase 10.1: a binary semaphore given from ISR wakes equal-priority
     * waiters in FIFO order and reports higher-priority readiness. */
    {
        static hr_semaphore_t semaphore;
        hr_semaphore_control_block_t *semaphore_control_block;
        hr_task_control_block_t *high_a_control_block;
        hr_task_control_block_t *high_b_control_block;
        bool higher_priority_task_woken = false;

        TEST_ASSERT_EQ_UINT(HR_OK,
                            hr_semaphore_create_binary(&semaphore, false));
        semaphore_control_block = hr_semaphore_control_block(&semaphore);
        high_a_control_block = hr_task_control_block(&high_a);
        high_b_control_block = hr_task_control_block(&high_b);

        /* Current is Low while both high tasks are READY after the queue ISR. */
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_kernel_block_current_on_wait_list_ex(
                &semaphore_control_block->waiters,
                semaphore_control_block,
                HR_TASK_WAIT_SEMAPHORE_TAKE,
                NULL,
                HR_WAIT_FOREVER,
                NULL));
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_kernel_block_current_on_wait_list_ex(
                &semaphore_control_block->waiters,
                semaphore_control_block,
                HR_TASK_WAIT_SEMAPHORE_TAKE,
                NULL,
                HR_WAIT_FOREVER,
                NULL));
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&low_task, hr_task_current());
        TEST_ASSERT_EQ_UINT(2U,
                            hr_semaphore_get_waiting_tasks(&semaphore));

        g_mock_inside_isr = true;
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_semaphore_give_from_isr(&semaphore,
                                       &higher_priority_task_woken));
        TEST_ASSERT_TRUE(higher_priority_task_woken);
        TEST_ASSERT_EQ_UINT(HR_OK, high_b_control_block->wait_result);
        TEST_ASSERT_EQ_UINT(HR_ERROR_INTERNAL,
                            high_a_control_block->wait_result);

        higher_priority_task_woken = false;
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_semaphore_give_from_isr(&semaphore,
                                       &higher_priority_task_woken));
        TEST_ASSERT_TRUE(higher_priority_task_woken);
        g_mock_inside_isr = false;

        TEST_ASSERT_EQ_UINT(HR_OK, high_a_control_block->wait_result);
        TEST_ASSERT_EQ_UINT(0U,
                            hr_semaphore_get_waiting_tasks(&semaphore));
        TEST_ASSERT_EQ_UINT(0U, hr_semaphore_get_count(&semaphore));
        TEST_ASSERT_TRUE(hr_semaphore_validate_internal(&semaphore));
    }

    /* Phase 10.2: inheritance remains active while any held mutex still has a
     * higher-priority waiter. Ownership is handed directly to each waiter. */
    {
        static hr_mutex_t mutex_a;
        static hr_mutex_t mutex_b;
        hr_mutex_control_block_t *mutex_a_control_block;
        hr_mutex_control_block_t *mutex_b_control_block;
        hr_task_control_block_t *low_control_block;
        hr_task_control_block_t *high_a_control_block;
        hr_task_control_block_t *high_b_control_block;

        TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_create(&mutex_a));
        TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_create(&mutex_b));
        mutex_a_control_block = hr_mutex_control_block(&mutex_a);
        mutex_b_control_block = hr_mutex_control_block(&mutex_b);
        low_control_block = hr_task_control_block(&low_task);
        high_a_control_block = hr_task_control_block(&high_a);
        high_b_control_block = hr_task_control_block(&high_b);

        TEST_ASSERT_EQ_PTR(&low_task, hr_task_current());
        TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_lock(&mutex_a, HR_NO_WAIT));
        TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_lock(&mutex_b, HR_NO_WAIT));
        TEST_ASSERT_EQ_UINT(2U, low_control_block->owned_mutex_count);

        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_kernel_block_current_on_wait_list_ex(
                &mutex_a_control_block->waiters,
                mutex_a_control_block,
                HR_TASK_WAIT_MUTEX_LOCK,
                NULL,
                HR_WAIT_FOREVER,
                hr_mutex_wait_cleanup));
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_mutex_recompute_owner_priority(mutex_a_control_block));
        TEST_ASSERT_EQ_UINT(1U,
                            hr_task_get_effective_priority(&low_task));

        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_kernel_block_current_on_wait_list_ex(
                &mutex_b_control_block->waiters,
                mutex_b_control_block,
                HR_TASK_WAIT_MUTEX_LOCK,
                NULL,
                HR_WAIT_FOREVER,
                hr_mutex_wait_cleanup));
        TEST_ASSERT_EQ_UINT(
            HR_OK,
            hr_mutex_recompute_owner_priority(mutex_b_control_block));
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&low_task, hr_task_current());
        TEST_ASSERT_EQ_UINT(1U,
                            hr_task_get_effective_priority(&low_task));

        TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_unlock(&mutex_a));
        TEST_ASSERT_EQ_PTR(&high_b, hr_mutex_get_owner(&mutex_a));
        TEST_ASSERT_EQ_UINT(HR_OK, high_b_control_block->wait_result);
        TEST_ASSERT_EQ_UINT(1U, low_control_block->owned_mutex_count);
        TEST_ASSERT_EQ_UINT(1U,
                            hr_task_get_effective_priority(&low_task));

        TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_unlock(&mutex_b));
        TEST_ASSERT_EQ_PTR(&high_a, hr_mutex_get_owner(&mutex_b));
        TEST_ASSERT_EQ_UINT(HR_OK, high_a_control_block->wait_result);
        TEST_ASSERT_EQ_UINT(0U, low_control_block->owned_mutex_count);
        TEST_ASSERT_EQ_UINT(5U,
                            hr_task_get_effective_priority(&low_task));

        TEST_ASSERT_TRUE(hr_mutex_validate_internal(&mutex_a));
        TEST_ASSERT_TRUE(hr_mutex_validate_internal(&mutex_b));
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());

        /* Release both handed-off mutexes and block both high tasks briefly so
         * Low can own a fresh mutex for the timeout-restoration scenario. */
        TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_unlock(&mutex_a));
        TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(1U));
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());
        TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_unlock(&mutex_b));
        TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(1U));
        hr_kernel_select_next_from_pendsv();
        TEST_ASSERT_EQ_PTR(&low_task, hr_task_current());

        {
            static hr_mutex_t timeout_mutex;
            hr_mutex_control_block_t *timeout_control_block;

            TEST_ASSERT_EQ_UINT(HR_OK, hr_mutex_create(&timeout_mutex));
            timeout_control_block = hr_mutex_control_block(&timeout_mutex);
            TEST_ASSERT_EQ_UINT(HR_OK,
                                hr_mutex_lock(&timeout_mutex, HR_NO_WAIT));

            /* Wake High A/B, then let High B wait for this mutex for two ticks. */
            hr_kernel_tick_from_isr();
            hr_kernel_select_next_from_pendsv();
            TEST_ASSERT_EQ_PTR(&high_b, hr_task_current());
            TEST_ASSERT_EQ_UINT(
                HR_OK,
                hr_kernel_block_current_on_wait_list_ex(
                    &timeout_control_block->waiters,
                    timeout_control_block,
                    HR_TASK_WAIT_MUTEX_LOCK,
                    NULL,
                    2U,
                    hr_mutex_wait_cleanup));
            TEST_ASSERT_EQ_UINT(
                HR_OK,
                hr_mutex_recompute_owner_priority(timeout_control_block));
            TEST_ASSERT_EQ_UINT(1U,
                                hr_task_get_effective_priority(&low_task));

            hr_kernel_select_next_from_pendsv();
            TEST_ASSERT_EQ_PTR(&high_a, hr_task_current());
            TEST_ASSERT_EQ_UINT(HR_OK, hr_task_delay(10U));
            hr_kernel_select_next_from_pendsv();
            TEST_ASSERT_EQ_PTR(&low_task, hr_task_current());

            hr_kernel_tick_from_isr();
            TEST_ASSERT_EQ_UINT(1U,
                                hr_task_get_effective_priority(&low_task));
            hr_kernel_tick_from_isr();
            TEST_ASSERT_EQ_UINT(HR_ERROR_TIMEOUT,
                                high_b_control_block->wait_result);
            TEST_ASSERT_EQ_UINT(0U,
                                hr_mutex_get_waiting_tasks(&timeout_mutex));
            TEST_ASSERT_EQ_UINT(5U,
                                hr_task_get_effective_priority(&low_task));
            TEST_ASSERT_TRUE(hr_mutex_validate_internal(&timeout_mutex));
        }
    }
}

void run_kernel_start_tests(void)
{
    RUN_TEST(test_kernel_preemption_round_robin_and_delay_race);
}

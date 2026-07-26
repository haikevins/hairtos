#include <stdbool.h>
#include <stdint.h>

#include "hairtos/hr_queue.h"
#include "hr_queue_internal.h"
#include "test_support.h"

extern bool g_mock_inside_isr;

static void test_queue_fifo_wrap_full_and_empty(void)
{
    static hr_queue_t queue;
    static uint32_t storage[3];
    uint32_t value;
    uint32_t received = 0U;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_create_static(&queue,
                                               storage,
                                               sizeof(storage[0]),
                                               3U));
    TEST_ASSERT_TRUE(hr_queue_is_valid(&queue));
    TEST_ASSERT_TRUE(hr_queue_validate_internal(&queue));
    TEST_ASSERT_EQ_UINT(3U, hr_queue_get_capacity(&queue));
    TEST_ASSERT_EQ_UINT(0U, hr_queue_get_count(&queue));

    value = 11U;
    TEST_ASSERT_EQ_UINT(HR_OK, hr_queue_send(&queue, &value, HR_NO_WAIT));
    value = 22U;
    TEST_ASSERT_EQ_UINT(HR_OK, hr_queue_send(&queue, &value, HR_NO_WAIT));
    value = 33U;
    TEST_ASSERT_EQ_UINT(HR_OK, hr_queue_send(&queue, &value, HR_NO_WAIT));
    value = 44U;
    TEST_ASSERT_EQ_UINT(HR_ERROR_QUEUE_FULL,
                        hr_queue_send(&queue, &value, HR_NO_WAIT));

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_receive(&queue, &received, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(11U, received);

    TEST_ASSERT_EQ_UINT(HR_OK, hr_queue_send(&queue, &value, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(3U, hr_queue_get_count(&queue));

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_receive(&queue, &received, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(22U, received);
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_receive(&queue, &received, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(33U, received);
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_receive(&queue, &received, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(44U, received);
    TEST_ASSERT_EQ_UINT(HR_ERROR_QUEUE_EMPTY,
                        hr_queue_receive(&queue, &received, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(0U, hr_queue_get_count(&queue));
    TEST_ASSERT_TRUE(hr_queue_validate_internal(&queue));
}

static void test_queue_rejects_invalid_creation_and_recreation(void)
{
    static hr_queue_t queue;
    static unsigned char storage[8];

    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_queue_create_static(NULL, storage, 1U, 8U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_queue_create_static(&queue, NULL, 1U, 8U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_queue_create_static(&queue, storage, 0U, 8U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_ARGUMENT,
                        hr_queue_create_static(&queue, storage, 1U, 0U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_create_static(&queue, storage, 1U, 8U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_queue_create_static(&queue, storage, 1U, 8U));
}

static void test_queue_isr_nonblocking_api(void)
{
    static hr_queue_t queue;
    static uint16_t storage[2];
    uint16_t sent = 0x1234U;
    uint16_t received = 0U;
    bool higher_priority_task_woken = true;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_create_static(&queue,
                                               storage,
                                               sizeof(storage[0]),
                                               2U));

    g_mock_inside_isr = false;
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_queue_send_from_isr(&queue,
                                               &sent,
                                               &higher_priority_task_woken));

    g_mock_inside_isr = true;
    TEST_ASSERT_EQ_UINT(HR_ERROR_FROM_ISR,
                        hr_queue_send(&queue, &sent, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_send_from_isr(&queue,
                                               &sent,
                                               &higher_priority_task_woken));
    TEST_ASSERT_TRUE(!higher_priority_task_woken);
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_receive_from_isr(&queue,
                                                  &received,
                                                  &higher_priority_task_woken));
    TEST_ASSERT_EQ_UINT(sent, received);
    TEST_ASSERT_TRUE(!higher_priority_task_woken);
    TEST_ASSERT_EQ_UINT(HR_ERROR_QUEUE_EMPTY,
                        hr_queue_receive_from_isr(&queue,
                                                  &received,
                                                  &higher_priority_task_woken));
    g_mock_inside_isr = false;
}

static void test_queue_blocking_requires_running_kernel(void)
{
    static hr_queue_t full_queue;
    static uint32_t full_storage[1];
    static hr_queue_t empty_queue;
    static uint32_t empty_storage[1];
    uint32_t value = 7U;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_create_static(&full_queue,
                                               full_storage,
                                               sizeof(value),
                                               1U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_create_static(&empty_queue,
                                               empty_storage,
                                               sizeof(value),
                                               1U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_send(&full_queue, &value, HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_queue_send(&full_queue, &value, 10U));
    TEST_ASSERT_EQ_UINT(HR_ERROR_INVALID_STATE,
                        hr_queue_receive(&empty_queue, &value, 10U));
}

void run_queue_tests(void)
{
    RUN_TEST(test_queue_fifo_wrap_full_and_empty);
    RUN_TEST(test_queue_rejects_invalid_creation_and_recreation);
    RUN_TEST(test_queue_isr_nonblocking_api);
    RUN_TEST(test_queue_blocking_requires_running_kernel);
}

#include "test_support.h"

#include <stdalign.h>
#include <stdint.h>
#include <string.h>

#include "hr_heap_lab.h"
#include "hr_pool_lab.h"

static void test_heap_init_alignment_and_minimum_allocation(void)
{
    unsigned char memory[1024U + 7U];
    hr_heap_lab_t heap;
    hr_heap_lab_stats_t stats;
    void *pointer;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_heap_lab_init(&heap, &memory[1], sizeof(memory) - 1U));
    TEST_ASSERT_TRUE(hr_heap_lab_validate(&heap));

    pointer = hr_heap_lab_alloc(&heap, 1U);
    TEST_ASSERT_TRUE(pointer != NULL);
    TEST_ASSERT_EQ_UINT(0U, (uintptr_t)pointer % alignof(max_align_t));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_get_stats(&heap, &stats));
    TEST_ASSERT_EQ_UINT(1U, stats.allocated_blocks);
    TEST_ASSERT_EQ_UINT(1U, stats.requested_bytes);
    TEST_ASSERT_TRUE(stats.internal_fragmentation_bytes > 0U);
}

static void test_heap_first_fit_reuses_freed_block(void)
{
    unsigned char memory[1024U];
    hr_heap_lab_t heap;
    void *first;
    void *second;
    void *replacement;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_heap_lab_init(&heap, memory, sizeof(memory)));
    first = hr_heap_lab_alloc(&heap, 48U);
    second = hr_heap_lab_alloc(&heap, 48U);
    TEST_ASSERT_TRUE((first != NULL) && (second != NULL));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_free(&heap, first));

    replacement = hr_heap_lab_alloc(&heap, 24U);
    TEST_ASSERT_EQ_PTR(first, replacement);
    TEST_ASSERT_TRUE(hr_heap_lab_validate(&heap));
}

static void test_heap_forward_and_backward_coalescing(void)
{
    unsigned char memory[1536U];
    hr_heap_lab_t heap;
    hr_heap_lab_stats_t stats;
    void *first;
    void *middle;
    void *last;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_heap_lab_init(&heap, memory, sizeof(memory)));
    first = hr_heap_lab_alloc(&heap, 96U);
    middle = hr_heap_lab_alloc(&heap, 128U);
    last = hr_heap_lab_alloc(&heap, 160U);
    TEST_ASSERT_TRUE((first != NULL) && (middle != NULL) && (last != NULL));

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_free(&heap, middle));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_free(&heap, first));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_free(&heap, last));
    TEST_ASSERT_TRUE(hr_heap_lab_validate(&heap));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_get_stats(&heap, &stats));
    TEST_ASSERT_EQ_UINT(0U, stats.allocated_blocks);
    TEST_ASSERT_EQ_UINT(1U, stats.free_blocks);
    TEST_ASSERT_EQ_UINT(stats.free_payload_bytes, stats.largest_free_block_bytes);
    TEST_ASSERT_EQ_UINT(0U, stats.external_fragmentation_bytes);
}

static void test_heap_coalesced_space_satisfies_large_allocation(void)
{
    unsigned char memory[1024U];
    hr_heap_lab_t heap;
    void *first;
    void *second;
    void *third;
    void *large;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_heap_lab_init(&heap, memory, sizeof(memory)));
    first = hr_heap_lab_alloc(&heap, 160U);
    second = hr_heap_lab_alloc(&heap, 160U);
    third = hr_heap_lab_alloc(&heap, 160U);
    TEST_ASSERT_TRUE((first != NULL) && (second != NULL) && (third != NULL));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_free(&heap, second));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_free(&heap, first));

    large = hr_heap_lab_alloc(&heap, 280U);
    TEST_ASSERT_EQ_PTR(first, large);
    TEST_ASSERT_TRUE(hr_heap_lab_validate(&heap));
}

static void test_heap_reports_internal_and_external_fragmentation(void)
{
    unsigned char memory[1536U];
    hr_heap_lab_t heap;
    hr_heap_lab_stats_t stats;
    void *first;
    void *middle;
    void *last;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_heap_lab_init(&heap, memory, sizeof(memory)));
    first = hr_heap_lab_alloc(&heap, 1U);
    middle = hr_heap_lab_alloc(&heap, 97U);
    last = hr_heap_lab_alloc(&heap, 33U);
    TEST_ASSERT_TRUE((first != NULL) && (middle != NULL) && (last != NULL));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_free(&heap, middle));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_get_stats(&heap, &stats));

    TEST_ASSERT_TRUE(stats.internal_fragmentation_bytes > 0U);
    TEST_ASSERT_TRUE(stats.free_blocks >= 2U);
    TEST_ASSERT_TRUE(stats.external_fragmentation_bytes > 0U);
    TEST_ASSERT_TRUE(stats.largest_free_block_bytes < stats.free_payload_bytes);
}

static void test_heap_rejects_invalid_and_double_free(void)
{
    unsigned char memory[1024U];
    unsigned char outside = 0U;
    hr_heap_lab_t heap;
    void *pointer;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_heap_lab_init(&heap, memory, sizeof(memory)));
    pointer = hr_heap_lab_alloc(&heap, 64U);
    TEST_ASSERT_TRUE(pointer != NULL);

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_ERROR_INVALID_POINTER,
                        hr_heap_lab_free(&heap, &outside));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_ERROR_INVALID_POINTER,
                        hr_heap_lab_free(&heap,
                            (void *)(((unsigned char *)pointer) + 1U)));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_free(&heap, pointer));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_ERROR_DOUBLE_FREE,
                        hr_heap_lab_free(&heap, pointer));
    TEST_ASSERT_TRUE(hr_heap_lab_validate(&heap));
}

static void test_heap_exhaustion_and_failed_allocation_counter(void)
{
    unsigned char memory[512U];
    hr_heap_lab_t heap;
    hr_heap_lab_stats_t stats;
    void *blocks[32U];
    size_t count = 0U;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_heap_lab_init(&heap, memory, sizeof(memory)));
    while (count < 32U)
    {
        blocks[count] = hr_heap_lab_alloc(&heap, 32U);
        if (blocks[count] == NULL)
        {
            break;
        }
        count++;
    }

    TEST_ASSERT_TRUE(count > 0U);
    TEST_ASSERT_TRUE(count < 32U);
    TEST_ASSERT_EQ_PTR(NULL, hr_heap_lab_alloc(&heap, 32U));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_heap_lab_get_stats(&heap, &stats));
    TEST_ASSERT_TRUE(stats.failed_allocation_count >= 2U);
}

static uint32_t next_random(uint32_t *state)
{
    *state = (*state * UINT32_C(1664525)) + UINT32_C(1013904223);
    return *state;
}

static void test_heap_randomized_allocate_free_sequences(void)
{
    unsigned char memory[4096U];
    hr_heap_lab_t heap;
    void *slots[32U];
    uint32_t random_state = UINT32_C(0x12345678);
    size_t step;

    (void)memset(slots, 0, sizeof(slots));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_heap_lab_init(&heap, memory, sizeof(memory)));

    for (step = 0U; step < 2000U; step++)
    {
        const size_t index = (size_t)(next_random(&random_state) % 32U);

        if (slots[index] == NULL)
        {
            const size_t request =
                (size_t)((next_random(&random_state) % 160U) + 1U);
            slots[index] = hr_heap_lab_alloc(&heap, request);
        }
        else
        {
            TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                                hr_heap_lab_free(&heap, slots[index]));
            slots[index] = NULL;
        }
        TEST_ASSERT_TRUE(hr_heap_lab_validate(&heap));
    }

    for (step = 0U; step < 32U; step++)
    {
        if (slots[step] != NULL)
        {
            TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                                hr_heap_lab_free(&heap, slots[step]));
        }
    }
    TEST_ASSERT_TRUE(hr_heap_lab_validate(&heap));
}

static void test_pool_allocates_fixed_aligned_blocks_and_reuses_lifo(void)
{
    unsigned char memory[512U + 3U];
    hr_pool_lab_t pool;
    void *first;
    void *second;
    void *replacement;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_pool_lab_init(&pool, &memory[1], sizeof(memory) - 1U,
                                         24U, 8U));
    first = hr_pool_lab_alloc(&pool);
    second = hr_pool_lab_alloc(&pool);
    TEST_ASSERT_TRUE((first != NULL) && (second != NULL));
    TEST_ASSERT_TRUE(first != second);
    TEST_ASSERT_EQ_UINT(0U, (uintptr_t)first % alignof(max_align_t));

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_pool_lab_free(&pool, first));
    replacement = hr_pool_lab_alloc(&pool);
    TEST_ASSERT_EQ_PTR(first, replacement);
    TEST_ASSERT_TRUE(hr_pool_lab_validate(&pool));
}

static void test_pool_exhaustion_stats_and_recovery(void)
{
    unsigned char memory[512U];
    hr_pool_lab_t pool;
    hr_pool_lab_stats_t stats;
    void *blocks[8U];
    size_t index;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_pool_lab_init(&pool, memory, sizeof(memory), 24U, 8U));
    for (index = 0U; index < 8U; index++)
    {
        blocks[index] = hr_pool_lab_alloc(&pool);
        TEST_ASSERT_TRUE(blocks[index] != NULL);
    }
    TEST_ASSERT_EQ_PTR(NULL, hr_pool_lab_alloc(&pool));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_pool_lab_get_stats(&pool, &stats));
    TEST_ASSERT_EQ_UINT(8U, stats.allocated_blocks);
    TEST_ASSERT_EQ_UINT(0U, stats.free_blocks);
    TEST_ASSERT_EQ_UINT(1U, stats.failed_allocation_count);

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_pool_lab_free(&pool, blocks[3]));
    TEST_ASSERT_EQ_PTR(blocks[3], hr_pool_lab_alloc(&pool));
    TEST_ASSERT_TRUE(hr_pool_lab_validate(&pool));
}

static void test_pool_rejects_invalid_and_double_free(void)
{
    unsigned char memory[512U];
    unsigned char outside = 0U;
    hr_pool_lab_t pool;
    void *pointer;

    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK,
                        hr_pool_lab_init(&pool, memory, sizeof(memory), 32U, 8U));
    pointer = hr_pool_lab_alloc(&pool);
    TEST_ASSERT_TRUE(pointer != NULL);
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_ERROR_INVALID_POINTER,
                        hr_pool_lab_free(&pool, &outside));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_ERROR_INVALID_POINTER,
                        hr_pool_lab_free(&pool,
                            (void *)(((unsigned char *)pointer) + 1U)));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_OK, hr_pool_lab_free(&pool, pointer));
    TEST_ASSERT_EQ_UINT(HR_HEAP_LAB_ERROR_DOUBLE_FREE,
                        hr_pool_lab_free(&pool, pointer));
    TEST_ASSERT_TRUE(hr_pool_lab_validate(&pool));
}

void run_memory_allocator_lab_tests(void)
{
    RUN_TEST(test_heap_init_alignment_and_minimum_allocation);
    RUN_TEST(test_heap_first_fit_reuses_freed_block);
    RUN_TEST(test_heap_forward_and_backward_coalescing);
    RUN_TEST(test_heap_coalesced_space_satisfies_large_allocation);
    RUN_TEST(test_heap_reports_internal_and_external_fragmentation);
    RUN_TEST(test_heap_rejects_invalid_and_double_free);
    RUN_TEST(test_heap_exhaustion_and_failed_allocation_counter);
    RUN_TEST(test_heap_randomized_allocate_free_sequences);
    RUN_TEST(test_pool_allocates_fixed_aligned_blocks_and_reuses_lifo);
    RUN_TEST(test_pool_exhaustion_stats_and_recovery);
    RUN_TEST(test_pool_rejects_invalid_and_double_free);
}

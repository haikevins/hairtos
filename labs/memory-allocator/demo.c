#include <stdio.h>

#include "hr_heap_lab.h"
#include "hr_pool_lab.h"

static void print_heap_stats(const char *label, const hr_heap_lab_t *heap)
{
    hr_heap_lab_stats_t stats;

    if (hr_heap_lab_get_stats(heap, &stats) != HR_HEAP_LAB_OK)
    {
        printf("%s: invalid heap\n", label);
        return;
    }

    printf("%s: allocated=%zu free=%zu largest=%zu internal=%zu external=%zu\n",
           label,
           stats.allocated_payload_bytes,
           stats.free_payload_bytes,
           stats.largest_free_block_bytes,
           stats.internal_fragmentation_bytes,
           stats.external_fragmentation_bytes);
}

int main(void)
{
    unsigned char heap_memory[2048U];
    unsigned char pool_memory[512U];
    hr_heap_lab_t heap;
    hr_pool_lab_t pool;
    void *first;
    void *second;
    void *third;

    if (hr_heap_lab_init(&heap, heap_memory, sizeof(heap_memory)) != HR_HEAP_LAB_OK)
    {
        return 1;
    }
    first = hr_heap_lab_alloc(&heap, 80U);
    second = hr_heap_lab_alloc(&heap, 160U);
    third = hr_heap_lab_alloc(&heap, 48U);
    print_heap_stats("after allocation", &heap);

    (void)hr_heap_lab_free(&heap, second);
    print_heap_stats("after middle free", &heap);
    (void)hr_heap_lab_free(&heap, first);
    (void)hr_heap_lab_free(&heap, third);
    print_heap_stats("after coalescing", &heap);

    if (hr_pool_lab_init(&pool, pool_memory, sizeof(pool_memory), 24U, 8U) !=
        HR_HEAP_LAB_OK)
    {
        return 1;
    }
    first = hr_pool_lab_alloc(&pool);
    second = hr_pool_lab_alloc(&pool);
    printf("pool: first=%p second=%p valid=%s\n",
           first,
           second,
           hr_pool_lab_validate(&pool) ? "yes" : "no");

    return hr_heap_lab_validate(&heap) ? 0 : 1;
}

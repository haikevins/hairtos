#include "board.h"
#include "hr_heap_lab.h"
#include "hr_pool_lab.h"

#define HEAP_ARENA_BYTES UINT32_C(2048)
#define POOL_ARENA_BYTES UINT32_C(512)

static unsigned char g_heap_arena[HEAP_ARENA_BYTES];
static unsigned char g_pool_arena[POOL_ARENA_BYTES];

static void print_heap_stats(const char *label, const hr_heap_lab_t *heap)
{
    hr_heap_lab_stats_t stats;

    if (hr_heap_lab_get_stats(heap, &stats) != HR_HEAP_LAB_OK)
    {
        board_uart_write_line("heap stats error");
        board_panic();
    }

    board_uart_write_string(label);
    board_uart_write_string(" requested=");
    board_uart_write_u32((uint32_t)stats.requested_bytes);
    board_uart_write_string(" allocated=");
    board_uart_write_u32((uint32_t)stats.allocated_payload_bytes);
    board_uart_write_string(" free=");
    board_uart_write_u32((uint32_t)stats.free_payload_bytes);
    board_uart_write_string(" largest=");
    board_uart_write_u32((uint32_t)stats.largest_free_block_bytes);
    board_uart_write_string(" internal_frag=");
    board_uart_write_u32((uint32_t)stats.internal_fragmentation_bytes);
    board_uart_write_string(" external_frag=");
    board_uart_write_u32((uint32_t)stats.external_fragmentation_bytes);
    board_uart_write_line("");
}

int main(void)
{
    hr_heap_lab_t heap;
    hr_pool_lab_t pool;
    hr_pool_lab_stats_t pool_stats;
    void *first;
    void *middle;
    void *last;
    void *large;
    void *pool_block;

    board_init();
    board_uart_write_line("hairtos memory allocator lab");
    board_uart_write_line("Allocator remains isolated from the kernel runtime.");

    if (hr_heap_lab_init(&heap, g_heap_arena, sizeof(g_heap_arena)) !=
        HR_HEAP_LAB_OK)
    {
        board_uart_write_line("heap init: FAIL");
        board_panic();
    }

    first = hr_heap_lab_alloc(&heap, 96U);
    middle = hr_heap_lab_alloc(&heap, 240U);
    last = hr_heap_lab_alloc(&heap, 80U);
    if ((first == NULL) || (middle == NULL) || (last == NULL))
    {
        board_uart_write_line("heap allocation: FAIL");
        board_panic();
    }
    print_heap_stats("after alloc", &heap);

    if ((hr_heap_lab_free(&heap, middle) != HR_HEAP_LAB_OK) ||
        (hr_heap_lab_free(&heap, first) != HR_HEAP_LAB_OK))
    {
        board_uart_write_line("heap free: FAIL");
        board_panic();
    }
    print_heap_stats("after fragmented free", &heap);

    large = hr_heap_lab_alloc(&heap, 280U);
    if (large != first)
    {
        board_uart_write_line("first-fit/coalesce: FAIL");
        board_panic();
    }
    print_heap_stats("after coalesced alloc", &heap);

    if ((hr_heap_lab_free(&heap, large) != HR_HEAP_LAB_OK) ||
        (hr_heap_lab_free(&heap, last) != HR_HEAP_LAB_OK) ||
        !hr_heap_lab_validate(&heap))
    {
        board_uart_write_line("heap validation: FAIL");
        board_panic();
    }
    print_heap_stats("after full coalesce", &heap);

    if (hr_pool_lab_init(&pool, g_pool_arena, sizeof(g_pool_arena), 24U, 8U) !=
        HR_HEAP_LAB_OK)
    {
        board_uart_write_line("pool init: FAIL");
        board_panic();
    }
    pool_block = hr_pool_lab_alloc(&pool);
    if ((pool_block == NULL) ||
        (hr_pool_lab_free(&pool, pool_block) != HR_HEAP_LAB_OK) ||
        (hr_pool_lab_get_stats(&pool, &pool_stats) != HR_HEAP_LAB_OK) ||
        !hr_pool_lab_validate(&pool))
    {
        board_uart_write_line("fixed-block pool: FAIL");
        board_panic();
    }

    board_uart_write_string("pool capacity=");
    board_uart_write_u32((uint32_t)pool_stats.capacity);
    board_uart_write_string(" stride=");
    board_uart_write_u32((uint32_t)pool_stats.block_stride);
    board_uart_write_string(" free=");
    board_uart_write_u32((uint32_t)pool_stats.free_blocks);
    board_uart_write_line("");
    board_uart_write_line("Memory allocator lab: PASS");

    for (;;)
    {
        board_led_toggle();
        board_delay_ms(500U);
    }
}

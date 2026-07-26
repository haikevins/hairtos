#ifndef HR_HEAP_LAB_H
#define HR_HEAP_LAB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    HR_HEAP_LAB_OK = 0,
    HR_HEAP_LAB_ERROR_INVALID_ARGUMENT,
    HR_HEAP_LAB_ERROR_NOT_INITIALIZED,
    HR_HEAP_LAB_ERROR_OUT_OF_MEMORY,
    HR_HEAP_LAB_ERROR_INVALID_POINTER,
    HR_HEAP_LAB_ERROR_DOUBLE_FREE,
    HR_HEAP_LAB_ERROR_CORRUPT
} hr_heap_lab_status_t;

typedef struct hr_heap_lab_block hr_heap_lab_block_t;

typedef struct
{
    unsigned char *arena_begin;
    unsigned char *arena_end;
    hr_heap_lab_block_t *first_block;
    size_t alignment;
    size_t header_bytes;
    size_t managed_bytes;
    size_t allocation_count;
    size_t free_count;
    size_t failed_allocation_count;
    uint32_t magic;
} hr_heap_lab_t;

typedef struct
{
    size_t managed_bytes;
    size_t requested_bytes;
    size_t allocated_payload_bytes;
    size_t free_payload_bytes;
    size_t largest_free_block_bytes;
    size_t internal_fragmentation_bytes;
    size_t external_fragmentation_bytes;
    size_t allocated_blocks;
    size_t free_blocks;
    size_t allocation_count;
    size_t free_count;
    size_t failed_allocation_count;
} hr_heap_lab_stats_t;

hr_heap_lab_status_t hr_heap_lab_init(
    hr_heap_lab_t *heap,
    void *memory,
    size_t memory_bytes
);

void *hr_heap_lab_alloc(hr_heap_lab_t *heap, size_t requested_bytes);

hr_heap_lab_status_t hr_heap_lab_free(
    hr_heap_lab_t *heap,
    void *pointer
);

hr_heap_lab_status_t hr_heap_lab_get_stats(
    const hr_heap_lab_t *heap,
    hr_heap_lab_stats_t *stats
);

bool hr_heap_lab_validate(const hr_heap_lab_t *heap);
size_t hr_heap_lab_alignment(void);

#endif /* HR_HEAP_LAB_H */

#ifndef HR_POOL_LAB_H
#define HR_POOL_LAB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hr_heap_lab.h"

typedef struct hr_pool_lab_free_node hr_pool_lab_free_node_t;

typedef struct
{
    unsigned char *memory_begin;
    unsigned char *memory_end;
    hr_pool_lab_free_node_t *free_head;
    size_t block_stride;
    size_t requested_block_bytes;
    size_t capacity;
    size_t free_blocks;
    size_t allocation_count;
    size_t free_count;
    size_t failed_allocation_count;
    uint32_t magic;
} hr_pool_lab_t;

typedef struct
{
    size_t block_bytes;
    size_t block_stride;
    size_t capacity;
    size_t allocated_blocks;
    size_t free_blocks;
    size_t internal_fragmentation_per_block;
    size_t allocation_count;
    size_t free_count;
    size_t failed_allocation_count;
} hr_pool_lab_stats_t;

hr_heap_lab_status_t hr_pool_lab_init(
    hr_pool_lab_t *pool,
    void *memory,
    size_t memory_bytes,
    size_t block_bytes,
    size_t block_count
);

void *hr_pool_lab_alloc(hr_pool_lab_t *pool);

hr_heap_lab_status_t hr_pool_lab_free(
    hr_pool_lab_t *pool,
    void *pointer
);

hr_heap_lab_status_t hr_pool_lab_get_stats(
    const hr_pool_lab_t *pool,
    hr_pool_lab_stats_t *stats
);

bool hr_pool_lab_validate(const hr_pool_lab_t *pool);

#endif /* HR_POOL_LAB_H */

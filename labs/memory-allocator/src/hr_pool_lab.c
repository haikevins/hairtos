#include "hr_pool_lab.h"

#include <stdalign.h>

#define HR_POOL_LAB_MAGIC UINT32_C(0x504C4142)

struct hr_pool_lab_free_node
{
    struct hr_pool_lab_free_node *next;
};

static bool align_up_size(size_t value, size_t alignment, size_t *result)
{
    const size_t mask = alignment - 1U;

    if ((result == NULL) || (value > (SIZE_MAX - mask)))
    {
        return false;
    }
    *result = (value + mask) & ~mask;
    return true;
}

static uintptr_t align_up_address(uintptr_t value, size_t alignment)
{
    const uintptr_t mask = (uintptr_t)(alignment - 1U);
    return (value + mask) & ~mask;
}

static bool pool_is_initialized(const hr_pool_lab_t *pool)
{
    return (pool != NULL) &&
           (pool->magic == HR_POOL_LAB_MAGIC) &&
           (pool->memory_begin != NULL) &&
           (pool->capacity > 0U);
}

static bool pointer_is_block(const hr_pool_lab_t *pool, const void *pointer)
{
    const uintptr_t address = (uintptr_t)pointer;
    const uintptr_t begin = (uintptr_t)pool->memory_begin;
    const uintptr_t end = (uintptr_t)pool->memory_end;

    return (address >= begin) && (address < end) &&
           (((address - begin) % pool->block_stride) == 0U);
}

static bool pointer_is_free(const hr_pool_lab_t *pool, const void *pointer)
{
    const hr_pool_lab_free_node_t *node;
    size_t visited = 0U;

    for (node = pool->free_head; node != NULL; node = node->next)
    {
        if ((const void *)node == pointer)
        {
            return true;
        }
        visited++;
        if (visited > pool->capacity)
        {
            return false;
        }
    }
    return false;
}

hr_heap_lab_status_t hr_pool_lab_init(
    hr_pool_lab_t *pool,
    void *memory,
    size_t memory_bytes,
    size_t block_bytes,
    size_t block_count
)
{
    const size_t alignment = alignof(max_align_t);
    const size_t original_block_bytes = block_bytes;
    uintptr_t raw_begin;
    uintptr_t aligned_begin;
    uintptr_t raw_end;
    size_t stride;
    size_t available;
    size_t capacity;
    size_t index;

    if ((pool == NULL) || (memory == NULL) || (block_bytes == 0U) ||
        (block_count == 0U) || (memory_bytes == 0U))
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }

    pool->memory_begin = NULL;
    pool->memory_end = NULL;
    pool->free_head = NULL;
    pool->block_stride = 0U;
    pool->requested_block_bytes = 0U;
    pool->capacity = 0U;
    pool->free_blocks = 0U;
    pool->allocation_count = 0U;
    pool->free_count = 0U;
    pool->failed_allocation_count = 0U;
    pool->magic = 0U;
    if (block_bytes < sizeof(hr_pool_lab_free_node_t))
    {
        block_bytes = sizeof(hr_pool_lab_free_node_t);
    }
    if (!align_up_size(block_bytes, alignment, &stride))
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }

    raw_begin = (uintptr_t)memory;
    if (raw_begin > (UINTPTR_MAX - memory_bytes))
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }
    raw_end = raw_begin + memory_bytes;
    aligned_begin = align_up_address(raw_begin, alignment);
    if (aligned_begin >= raw_end)
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }

    available = (size_t)(raw_end - aligned_begin);
    capacity = available / stride;
    if (capacity > block_count)
    {
        capacity = block_count;
    }
    if (capacity == 0U)
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }

    pool->memory_begin = (unsigned char *)aligned_begin;
    pool->memory_end = pool->memory_begin + (capacity * stride);
    pool->block_stride = stride;
    pool->requested_block_bytes = original_block_bytes;
    pool->capacity = capacity;
    pool->free_blocks = capacity;
    pool->magic = HR_POOL_LAB_MAGIC;

    pool->free_head = NULL;
    for (index = capacity; index > 0U; index--)
    {
        hr_pool_lab_free_node_t *node =
            (hr_pool_lab_free_node_t *)(void *)(pool->memory_begin +
                                                ((index - 1U) * stride));
        node->next = pool->free_head;
        pool->free_head = node;
    }

    return HR_HEAP_LAB_OK;
}

void *hr_pool_lab_alloc(hr_pool_lab_t *pool)
{
    hr_pool_lab_free_node_t *node;

    if (!pool_is_initialized(pool) || (pool->free_head == NULL))
    {
        if (pool_is_initialized(pool))
        {
            pool->failed_allocation_count++;
        }
        return NULL;
    }

    node = pool->free_head;
    pool->free_head = node->next;
    node->next = NULL;
    pool->free_blocks--;
    pool->allocation_count++;
    return node;
}

hr_heap_lab_status_t hr_pool_lab_free(
    hr_pool_lab_t *pool,
    void *pointer
)
{
    hr_pool_lab_free_node_t *node;

    if (!pool_is_initialized(pool))
    {
        return HR_HEAP_LAB_ERROR_NOT_INITIALIZED;
    }
    if ((pointer == NULL) || !pointer_is_block(pool, pointer))
    {
        return HR_HEAP_LAB_ERROR_INVALID_POINTER;
    }
    if (pointer_is_free(pool, pointer))
    {
        return HR_HEAP_LAB_ERROR_DOUBLE_FREE;
    }

    node = (hr_pool_lab_free_node_t *)pointer;
    node->next = pool->free_head;
    pool->free_head = node;
    pool->free_blocks++;
    pool->free_count++;
    return HR_HEAP_LAB_OK;
}

bool hr_pool_lab_validate(const hr_pool_lab_t *pool)
{
    const hr_pool_lab_free_node_t *node;
    size_t count = 0U;

    if (!pool_is_initialized(pool) ||
        (pool->memory_end <= pool->memory_begin) ||
        (pool->free_blocks > pool->capacity))
    {
        return false;
    }

    for (node = pool->free_head; node != NULL; node = node->next)
    {
        if (!pointer_is_block(pool, node))
        {
            return false;
        }
        count++;
        if (count > pool->capacity)
        {
            return false;
        }
    }

    return count == pool->free_blocks;
}

hr_heap_lab_status_t hr_pool_lab_get_stats(
    const hr_pool_lab_t *pool,
    hr_pool_lab_stats_t *stats
)
{
    if (!pool_is_initialized(pool))
    {
        return HR_HEAP_LAB_ERROR_NOT_INITIALIZED;
    }
    if (stats == NULL)
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }
    if (!hr_pool_lab_validate(pool))
    {
        return HR_HEAP_LAB_ERROR_CORRUPT;
    }

    stats->block_bytes = pool->requested_block_bytes;
    stats->block_stride = pool->block_stride;
    stats->capacity = pool->capacity;
    stats->allocated_blocks = pool->capacity - pool->free_blocks;
    stats->free_blocks = pool->free_blocks;
    stats->internal_fragmentation_per_block =
        pool->block_stride - pool->requested_block_bytes;
    stats->allocation_count = pool->allocation_count;
    stats->free_count = pool->free_count;
    stats->failed_allocation_count = pool->failed_allocation_count;
    return HR_HEAP_LAB_OK;
}

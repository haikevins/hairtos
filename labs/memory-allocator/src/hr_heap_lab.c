#include "hr_heap_lab.h"

#include <stdalign.h>

#define HR_HEAP_LAB_MAGIC       UINT32_C(0x484C4142)
#define HR_HEAP_BLOCK_MAGIC     UINT32_C(0x48424C4B)

struct hr_heap_lab_block
{
    size_t payload_bytes;
    size_t requested_bytes;
    struct hr_heap_lab_block *previous;
    struct hr_heap_lab_block *next;
    uint32_t magic;
    bool is_free;
};

static bool size_add_overflows(size_t left, size_t right)
{
    return left > (SIZE_MAX - right);
}

static bool align_up_size(size_t value, size_t alignment, size_t *result)
{
    const size_t mask = alignment - 1U;

    if ((result == NULL) || (alignment == 0U) || ((alignment & mask) != 0U))
    {
        return false;
    }
    if (size_add_overflows(value, mask))
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

static unsigned char *block_payload(
    const hr_heap_lab_t *heap,
    const hr_heap_lab_block_t *block
)
{
    return ((unsigned char *)(uintptr_t)block) + heap->header_bytes;
}

static bool heap_is_initialized(const hr_heap_lab_t *heap)
{
    return (heap != NULL) &&
           (heap->magic == HR_HEAP_LAB_MAGIC) &&
           (heap->arena_begin != NULL) &&
           (heap->arena_end != NULL) &&
           (heap->first_block != NULL);
}

static void merge_with_next(
    const hr_heap_lab_t *heap,
    hr_heap_lab_block_t *block
)
{
    hr_heap_lab_block_t *next = block->next;

    if ((next == NULL) || !next->is_free)
    {
        return;
    }

    block->payload_bytes += heap->header_bytes + next->payload_bytes;
    block->next = next->next;
    if (block->next != NULL)
    {
        block->next->previous = block;
    }

    next->magic = 0U;
    next->previous = NULL;
    next->next = NULL;
    next->payload_bytes = 0U;
    next->requested_bytes = 0U;
    next->is_free = false;
}

size_t hr_heap_lab_alignment(void)
{
    return alignof(max_align_t);
}

hr_heap_lab_status_t hr_heap_lab_init(
    hr_heap_lab_t *heap,
    void *memory,
    size_t memory_bytes
)
{
    uintptr_t raw_begin;
    uintptr_t aligned_begin;
    uintptr_t raw_end;
    size_t alignment;
    size_t header_bytes;
    size_t managed_bytes;
    hr_heap_lab_block_t *first;

    if ((heap == NULL) || (memory == NULL) || (memory_bytes == 0U))
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }

    heap->arena_begin = NULL;
    heap->arena_end = NULL;
    heap->first_block = NULL;
    heap->alignment = 0U;
    heap->header_bytes = 0U;
    heap->managed_bytes = 0U;
    heap->allocation_count = 0U;
    heap->free_count = 0U;
    heap->failed_allocation_count = 0U;
    heap->magic = 0U;
    alignment = hr_heap_lab_alignment();
    if (!align_up_size(sizeof(hr_heap_lab_block_t), alignment, &header_bytes))
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
    if ((aligned_begin >= raw_end) || ((raw_end - aligned_begin) > SIZE_MAX))
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }

    managed_bytes = (size_t)(raw_end - aligned_begin);
    managed_bytes &= ~(alignment - 1U);
    if (managed_bytes < (header_bytes + alignment))
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }

    heap->arena_begin = (unsigned char *)aligned_begin;
    heap->arena_end = heap->arena_begin + managed_bytes;
    heap->alignment = alignment;
    heap->header_bytes = header_bytes;
    heap->managed_bytes = managed_bytes;
    heap->magic = HR_HEAP_LAB_MAGIC;

    first = (hr_heap_lab_block_t *)(void *)heap->arena_begin;
    first->payload_bytes = managed_bytes - header_bytes;
    first->requested_bytes = 0U;
    first->previous = NULL;
    first->next = NULL;
    first->magic = HR_HEAP_BLOCK_MAGIC;
    first->is_free = true;
    heap->first_block = first;

    return HR_HEAP_LAB_OK;
}

void *hr_heap_lab_alloc(hr_heap_lab_t *heap, size_t requested_bytes)
{
    hr_heap_lab_block_t *block;
    size_t aligned_bytes;

    if (!heap_is_initialized(heap) || (requested_bytes == 0U) ||
        !align_up_size(requested_bytes, heap->alignment, &aligned_bytes))
    {
        if (heap_is_initialized(heap))
        {
            heap->failed_allocation_count++;
        }
        return NULL;
    }

    for (block = heap->first_block; block != NULL; block = block->next)
    {
        size_t remainder;

        if (!block->is_free || (block->payload_bytes < aligned_bytes))
        {
            continue;
        }

        remainder = block->payload_bytes - aligned_bytes;
        if (remainder >= (heap->header_bytes + heap->alignment))
        {
            unsigned char *new_address = block_payload(heap, block) + aligned_bytes;
            hr_heap_lab_block_t *split = (hr_heap_lab_block_t *)(void *)new_address;

            split->payload_bytes = remainder - heap->header_bytes;
            split->requested_bytes = 0U;
            split->previous = block;
            split->next = block->next;
            split->magic = HR_HEAP_BLOCK_MAGIC;
            split->is_free = true;
            if (split->next != NULL)
            {
                split->next->previous = split;
            }
            block->next = split;
            block->payload_bytes = aligned_bytes;
        }

        block->requested_bytes = requested_bytes;
        block->is_free = false;
        heap->allocation_count++;
        return block_payload(heap, block);
    }

    heap->failed_allocation_count++;
    return NULL;
}

hr_heap_lab_status_t hr_heap_lab_free(
    hr_heap_lab_t *heap,
    void *pointer
)
{
    hr_heap_lab_block_t *block;

    if (!heap_is_initialized(heap))
    {
        return HR_HEAP_LAB_ERROR_NOT_INITIALIZED;
    }
    if (pointer == NULL)
    {
        return HR_HEAP_LAB_ERROR_INVALID_POINTER;
    }

    for (block = heap->first_block; block != NULL; block = block->next)
    {
        if ((void *)block_payload(heap, block) != pointer)
        {
            continue;
        }

        if (block->is_free)
        {
            return HR_HEAP_LAB_ERROR_DOUBLE_FREE;
        }

        block->is_free = true;
        block->requested_bytes = 0U;
        heap->free_count++;

        merge_with_next(heap, block);
        if ((block->previous != NULL) && block->previous->is_free)
        {
            block = block->previous;
            merge_with_next(heap, block);
        }

        return HR_HEAP_LAB_OK;
    }

    return HR_HEAP_LAB_ERROR_INVALID_POINTER;
}

bool hr_heap_lab_validate(const hr_heap_lab_t *heap)
{
    const hr_heap_lab_block_t *block;
    const hr_heap_lab_block_t *previous = NULL;
    const unsigned char *expected;
    size_t visited = 0U;

    if (!heap_is_initialized(heap) ||
        (heap->alignment != hr_heap_lab_alignment()) ||
        (heap->arena_end <= heap->arena_begin) ||
        (((uintptr_t)heap->arena_begin % heap->alignment) != 0U))
    {
        return false;
    }

    expected = heap->arena_begin;
    for (block = heap->first_block; block != NULL; block = block->next)
    {
        const unsigned char *block_address = (const unsigned char *)(const void *)block;
        const unsigned char *end;

        if ((block_address != expected) ||
            (block->magic != HR_HEAP_BLOCK_MAGIC) ||
            (block->previous != previous) ||
            (block->payload_bytes < heap->alignment) ||
            ((block->payload_bytes % heap->alignment) != 0U) ||
            (!block->is_free && ((block->requested_bytes == 0U) ||
                                 (block->requested_bytes > block->payload_bytes))) ||
            (block->is_free && (block->requested_bytes != 0U)))
        {
            return false;
        }

        end = block_payload(heap, block) + block->payload_bytes;
        if ((end <= block_address) || (end > heap->arena_end))
        {
            return false;
        }
        if ((block->next != NULL) && block->is_free && block->next->is_free)
        {
            return false;
        }

        expected = end;
        previous = block;
        visited++;
        if (visited > (heap->managed_bytes / heap->alignment))
        {
            return false;
        }
    }

    return expected == heap->arena_end;
}

hr_heap_lab_status_t hr_heap_lab_get_stats(
    const hr_heap_lab_t *heap,
    hr_heap_lab_stats_t *stats
)
{
    const hr_heap_lab_block_t *block;

    if (!heap_is_initialized(heap))
    {
        return HR_HEAP_LAB_ERROR_NOT_INITIALIZED;
    }
    if (stats == NULL)
    {
        return HR_HEAP_LAB_ERROR_INVALID_ARGUMENT;
    }
    if (!hr_heap_lab_validate(heap))
    {
        return HR_HEAP_LAB_ERROR_CORRUPT;
    }

    stats->managed_bytes = 0U;
    stats->requested_bytes = 0U;
    stats->allocated_payload_bytes = 0U;
    stats->free_payload_bytes = 0U;
    stats->largest_free_block_bytes = 0U;
    stats->internal_fragmentation_bytes = 0U;
    stats->external_fragmentation_bytes = 0U;
    stats->allocated_blocks = 0U;
    stats->free_blocks = 0U;
    stats->allocation_count = 0U;
    stats->free_count = 0U;
    stats->failed_allocation_count = 0U;
    stats->managed_bytes = heap->managed_bytes;
    stats->allocation_count = heap->allocation_count;
    stats->free_count = heap->free_count;
    stats->failed_allocation_count = heap->failed_allocation_count;

    for (block = heap->first_block; block != NULL; block = block->next)
    {
        if (block->is_free)
        {
            stats->free_blocks++;
            stats->free_payload_bytes += block->payload_bytes;
            if (block->payload_bytes > stats->largest_free_block_bytes)
            {
                stats->largest_free_block_bytes = block->payload_bytes;
            }
        }
        else
        {
            stats->allocated_blocks++;
            stats->requested_bytes += block->requested_bytes;
            stats->allocated_payload_bytes += block->payload_bytes;
            stats->internal_fragmentation_bytes +=
                block->payload_bytes - block->requested_bytes;
        }
    }

    stats->external_fragmentation_bytes =
        stats->free_payload_bytes - stats->largest_free_block_bytes;
    return HR_HEAP_LAB_OK;
}

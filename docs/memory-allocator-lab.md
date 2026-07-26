# Memory Allocator Lab

## Position in the project

The completed Phase 14 implementation lives under `labs/memory-allocator/`. It
is not required by the static-first HairRTOS kernel and is not linked into tasks,
queues, synchronization objects, timers, or HairEvent unless the dedicated lab
is built.

## Public lab headers

```text
labs/memory-allocator/include/hr_heap_lab.h
labs/memory-allocator/include/hr_pool_lab.h
```

## First-fit heap API

```c
hr_heap_lab_status_t hr_heap_lab_init(
    hr_heap_lab_t *heap,
    void *memory,
    size_t memory_bytes
);

void *hr_heap_lab_alloc(hr_heap_lab_t *heap, size_t requested_bytes);
hr_heap_lab_status_t hr_heap_lab_free(hr_heap_lab_t *heap, void *pointer);
hr_heap_lab_status_t hr_heap_lab_get_stats(
    const hr_heap_lab_t *heap,
    hr_heap_lab_stats_t *stats
);
bool hr_heap_lab_validate(const hr_heap_lab_t *heap);
```

## Fixed-block pool API

```c
hr_heap_lab_status_t hr_pool_lab_init(
    hr_pool_lab_t *pool,
    void *memory,
    size_t memory_bytes,
    size_t block_bytes,
    size_t block_count
);

void *hr_pool_lab_alloc(hr_pool_lab_t *pool);
hr_heap_lab_status_t hr_pool_lab_free(hr_pool_lab_t *pool, void *pointer);
```

## Commands

```bash
make phase14-lab
make host-tests
make EXAMPLE=14-memory-allocator-lab
make phase14-check
```

See `phase14-memory-allocator-lab.md` for design, fragmentation metrics, error
handling, and architectural boundaries.

# Memory Allocator Lab

## Position in the project

This is an educational lab under `labs/memory-allocator/`. It is not required by
the static-first HairRTOS v1.0 kernel and must not become a hidden dependency of
task, queue, timer, or synchronization code.

## Learning objectives

Implement and measure:

- aligned allocation;
- first-fit search;
- block headers;
- splitting a larger free block;
- coalescing adjacent free blocks;
- internal and external fragmentation;
- invalid-free detection;
- double-free detection;
- out-of-memory behavior;
- usage and fragmentation statistics.

## Proposed block model

```text
[header | payload][header | payload][header | payload]
```

A header may contain:

- payload size;
- free/allocated state;
- previous and next physical-block metadata;
- optional debug magic value.

## API concept

```c
void hr_heap_lab_init(void *memory, size_t size);
void *hr_heap_lab_alloc(size_t size);
hr_status_t hr_heap_lab_free(void *pointer);
void hr_heap_lab_get_stats(hr_heap_lab_stats_t *stats);
```

These names are provisional and intentionally separate from public kernel APIs.

## Required host tests

- minimum allocation;
- alignment for multiple requested sizes;
- exact-fit allocation;
- split remaining block;
- forward and backward coalescing;
- allocation after coalescing;
- exhaustion;
- invalid pointer;
- pointer into the middle of a block;
- double free;
- randomized allocate/free sequences;
- fragmentation statistics.

## Target demonstration

An optional target example can use a statically reserved heap region and print
statistics through UART. Allocation must not occur from PendSV, SysTick, or a
critical kernel path.

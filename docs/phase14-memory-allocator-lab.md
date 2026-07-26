# Phase 14 — Memory Allocator Lab

## Purpose

Phase 14 adds an isolated memory-management laboratory while preserving the
static-first HairRTOS architecture. It compares a deterministic fixed-block
pool with a variable-size first-fit heap.

## Fixed-block pool model

The caller supplies a memory region, requested block size, and block count. The
implementation aligns the arena and rounds each block stride to `max_align_t`.
Free blocks carry an intrusive single-link pointer in their own storage.

Properties:

- constant-time pop/push of the free-list head;
- fixed maximum capacity;
- no external fragmentation;
- predictable internal fragmentation from stride rounding;
- exact block-boundary validation;
- exhaustion and double-free diagnostics.

## First-fit heap model

The first physical block covers the complete aligned arena. Allocation walks the
physical list and chooses the first free block large enough for the aligned
request. A block is split only when the remainder can contain both a new header
and at least one aligned payload unit.

Freeing marks the block free and coalesces with the next and previous physical
neighbors. The validator requires physical adjacency and rejects consecutive
free blocks, which would indicate a missed coalescing operation.

## Fragmentation metrics

The heap reports:

- requested bytes;
- allocated payload capacity;
- free payload bytes;
- largest free block;
- internal fragmentation: allocated capacity minus requested bytes;
- external fragmentation: total free payload minus largest free block;
- allocated/free block count;
- allocation, free, and failed-allocation counters.

These metrics are observations of the current arena state; they do not include
header overhead in payload totals.

## Error model

The lab returns dedicated `hr_heap_lab_status_t` values for invalid arguments,
uninitialized contexts, out-of-memory conditions, invalid pointers, double
free, and detected structural corruption. Allocation returns `NULL` on failure
and increments the failed-allocation counter when the heap or pool is valid.

## Concurrency and RTOS boundary

Neither allocator contains locks. If tasks share an allocator, the application
must serialize access with an appropriate mutex. Allocator calls must never be
made from ISR context or scheduler exception paths.

HairRTOS task, queue, semaphore, mutex, timer, HairEvent, and Active Object
storage remain caller-provided and static. Phase 14 does not change that policy.

## Validation

Host coverage includes alignment, first-fit reuse, split behavior, forward and
backward coalescing, large allocation after coalescing, internal/external
fragmentation, invalid free, middle-pointer free, double free, exhaustion,
deterministic randomized sequences, pool reuse, and pool diagnostics.

The STM32 example repeats a representative sequence and reports statistics over
UART. Physical timing and long-duration target stress remain outside Phase 14.

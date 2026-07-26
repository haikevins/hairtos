# Phase 14 — Memory Allocator Lab

This lab studies allocator mechanics without making HairRTOS depend on dynamic
memory. The production kernel remains static-first.

## Implemented allocators

### Fixed-block pool

`hr_pool_lab_t` divides a caller-provided arena into equally sized aligned
blocks. Allocation and free-list insertion are constant-time. Free validation
is intentionally linear in this educational implementation so invalid pointers
and double frees can be diagnosed without a second metadata array.

### First-fit heap

`hr_heap_lab_t` manages a caller-provided arena as a physical block chain:

```text
[header | payload][header | payload][header | payload]
```

It implements:

- `max_align_t` payload alignment;
- first-fit search;
- block splitting when the remainder can hold another header and payload;
- forward and backward adjacent-block coalescing;
- invalid-pointer and middle-of-block rejection;
- double-free detection;
- out-of-memory accounting;
- structural validation;
- internal and external fragmentation statistics.

## Host commands

```bash
make phase14-lab
make host-tests
make phase14-check
```

The standalone native demo prints allocator statistics. `host-tests` runs the
allocator through ASan/UBSan together with all completed HairRTOS tests.

## STM32 target demonstration

```bash
make EXAMPLE=14-memory-allocator-lab
make EXAMPLE=14-memory-allocator-lab flash
```

The target example uses statically reserved application arenas and prints
statistics through USART1. It does not allocate TCBs, queues, timers, Active
Objects, or kernel internals dynamically.

## Deliberate boundaries

- no global heap;
- no `malloc`, `free`, or libc allocator dependency;
- no allocation from SysTick, PendSV, SVC, or an ISR;
- no allocator calls in kernel hot paths;
- no synchronization inside the allocator;
- callers must serialize concurrent access themselves.

The lab is intended for learning and measurement, not as a production general-
purpose allocator.

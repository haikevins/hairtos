# Memory Model

## Kernel policy

HairRTOS v1.0 is static-first. Applications supply TCBs, stacks, queue storage,
synchronization objects, timers, Active Objects, and initial event storage.

Static allocation provides deterministic capacity, avoids external
fragmentation, and reduces failure sources while scheduler and context-switch
code are new.

Fixed-size pools may later support dynamic HairEvent events. Pool capacity and
ownership must remain explicit.

## Memory allocator lab

Phase 14 implements an isolated fixed-block pool and first-fit heap lab covering:

- fixed-block capacity and alignment;
- variable-size alignment;
- block splitting;
- adjacent-block coalescing;
- invalid and double free;
- exhaustion;
- internal and external fragmentation;
- allocator statistics.

The lab lives under `labs/memory-allocator/` and is not a required kernel
dependency. See `memory-allocator-lab.md`.

## Stack diagnostics

Task stacks are filled with `0xA5`, checked with a guard word, and measured for a
high-water mark.

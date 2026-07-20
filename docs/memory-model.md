# Memory Model

v1.0 is static-first. Applications supply TCBs, stacks, queue storage,
synchronization objects, timers, Active Objects, and initial event storage.

Static allocation gives deterministic capacity, avoids external fragmentation,
and reduces failure sources while scheduler and context-switch code are new.

Fixed-size pools may later support dynamic events. A general-purpose heap is
outside the early roadmap and requires tests for alignment, splitting,
coalescing, invalid/double free, exhaustion, fragmentation, and concurrency.

Task stacks are filled with `0xA5`, checked with a guard word, and measured for a
high-water mark.

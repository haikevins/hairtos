# Phase 14 — Memory Allocator Lab

This STM32 target demonstration uses two allocators implemented under
`labs/memory-allocator/`:

- a constant-size fixed-block pool;
- a first-fit heap with aligned allocation, block splitting, and adjacent-block
  coalescing.

The allocator is deliberately not used by the hairtos kernel. The demo reserves
static application arenas, performs deterministic allocation/free sequences,
prints fragmentation statistics over USART1, and then blinks PC13 after PASS.

```bash
make EXAMPLE=14-memory-allocator-lab build
make EXAMPLE=14-memory-allocator-lab run
```

Run the host lab and sanitizer tests separately:

```bash
make ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run
make EXAMPLE=14-memory-allocator-lab check
```

# Phase 2 — Intrusive Lists and Kernel Data Structures

## Status

Complete.

Phase 2 is host-only and does not add task switching or scheduler execution to
STM32 firmware. The Phase 1 bare-metal target remains buildable and unchanged in
behavior.

## Implemented components

### Intrusive doubly linked list

A circular sentinel list with O(1) insertion/removal when a node is known.
Each node stores its owner and current list membership, allowing debug checks for
double insertion and double removal.

Implemented operations include initialization, front/back insertion, insertion
before a position, removal, pop, traversal, size, and structural validation.

### Priority ready set

One FIFO queue exists per configured priority plus a ready bitmap. Priority 0 is
the highest. Equal-priority nodes preserve insertion order and can be rotated for
future round-robin scheduling.

### Priority wait list

Waiters are ordered by effective priority. Equal-priority waiters remain FIFO.
This becomes the shared ordering rule for queue, semaphore, and mutex waiters.

### Timeout list

Two ordered lists are used:

- current tick epoch;
- overflow epoch after 32-bit tick wrap.

When the tick counter wraps, the two lists are swapped. Nodes with equal wake
ticks preserve FIFO order.

## Deliberate boundaries

Phase 2 does not implement:

- a Task Control Block;
- context switch;
- SVC or PendSV;
- a running scheduler;
- blocking task APIs;
- queue, semaphore, or mutex objects.

The structures operate on generic intrusive nodes so they can be host-tested
before Phase 3 defines the TCB.

## Commands

```bash
make host-tests
make phase2-check
```

`host-tests` enables AddressSanitizer and UndefinedBehaviorSanitizer when the
selected host compiler supports them.

## Definition of Done evidence

- no STM32 header is included by Phase 2 sources;
- all host tests compile with warnings as errors;
- ASan and UBSan run cleanly;
- list invariants are checked after mutations;
- ready FIFO and priority ordering are tested;
- wait-list priority/FIFO ordering is tested;
- timeout ordering and tick wrap are tested;
- Phase 1 target build still passes.

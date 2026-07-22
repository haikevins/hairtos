# Testing Guide

## Host tests

Host tests cover:

- intrusive lists;
- ready queues;
- timeout ordering;
- queue ring buffers;
- waiter ordering;
- suspend/resume state policy;
- event pools;
- state transitions;
- allocator-lab split/coalesce and misuse detection.

Build with:

```text
-Wall -Wextra -Werror
-fsanitize=address
-fsanitize=undefined
```

## Target tests

Target tests cover:

- SVC startup;
- PendSV switching;
- preemption;
- round-robin;
- ISR-safe wake-up;
- tick wrap;
- stack guards;
- priority inheritance;
- task suspend/resume;
- software-timer expiration.

## Stress tests

Stress tests run many:

- task switches;
- queue contention sequences;
- event allocation/release cycles;
- timeout/object races;
- suspend/resume operations;
- randomized allocator-lab sequences.

## Benchmarks

Phase 15 benchmarks are separate from pass/fail tests. They measure latency,
jitter, code size, static RAM, and stack usage with DWT and GPIO instrumentation.
See `benchmark-plan.md`.

A fast benchmark does not prove correctness, and an average latency does not
prove a worst-case real-time bound.

## Definition of Done

A feature is complete only with:

- API;
- implementation;
- validation;
- context rules;
- timeout and ISR behavior;
- host tests where possible;
- target tests where required;
- focused example;
- documentation;
- return statuses;
- misuse assertions.

Test names follow:

```text
test_<unit>_<condition>_<expected_result>
```

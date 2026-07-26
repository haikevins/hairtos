# Testing Guide

## Completed host tests through Phase 8

Host tests currently cover:

- intrusive lists;
- ready queues and bitmap maintenance;
- wait-list priority/FIFO ordering;
- timeout ordering and tick wrap;
- Cortex-M initial stack-frame construction;
- TCB creation, state transitions, stack guard, and high-water mark;
- fixed-priority selection;
- higher-priority preemption decisions;
- equal-priority peer detection and time slicing;
- delay/block/wake transitions and the one-tick PendSV race.

They build with strict warnings and AddressSanitizer/UndefinedBehaviorSanitizer.

```bash
make host-tests
```

## Completed target build checks through Phase 8

Automated cross-build and symbol/disassembly checks cover:

- SVC first-task startup;
- PendSV R4-R11 and PSP save/restore;
- fixed-priority cooperative switching;
- kernel SysTick and blocking delay;
- strong SysTick/PendSV handlers;
- preemption and round-robin scheduler paths;
- interrupt masking around the PendSV selector.

```bash
make phase8-check
```

Physical timing and runtime behavior must still be validated on the STM32F103
board.

## Planned tests

Later phases add queue contention, ISR-safe wake-up, priority inheritance, task
suspend/resume, software-timer expiration, event pools, allocator misuse, stress
loads, and DWT/GPIO benchmarks.

## Definition of Done

A feature is complete only with API, implementation, validation, context rules,
focused tests, an example, documentation, return statuses, and misuse handling.

Test names follow:

```text
test_<unit>_<condition>_<expected_result>
```


## Phase 16 stabilization validation

```bash
make host-tests
make phase16-stress
make EXAMPLE=16-diagnostics-stress-stabilization
make phase16-check
```

The automated check cross-builds every target example, runs both host compilers
when available, validates strong Cortex-M fault symbols and the `.noinit`
section, compiles the architecture-independent core for Cortex-M0, and builds
the final target with Make and CMake/Ninja. Physical endurance and fault-reset
checks remain manual release-candidate activities.

# Phase 3 — TCB and Initial Task Stack

## Status

Complete.

Phase 3 defines statically allocated task objects and constructs the initial
Cortex-M3 software plus hardware exception frame. It does not start or switch
tasks; SVC startup is Phase 4 and PendSV switching is Phase 5.

## Public task storage

`hr_task_t` is fixed-size opaque aligned storage. Applications can allocate it
statically while the internal TCB layout remains private. A compile-time
assertion verifies that `HR_CFG_TASK_STORAGE_BYTES` is large enough.

## Internal TCB

The TCB stores:

- saved stack pointer as the first field;
- stack low/high bounds and word count;
- name, entry function, and argument;
- task state;
- base/effective priority;
- wake tick and time slice;
- intrusive ready, wait, timeout, and all-task nodes;
- waiting object and diagnostic counters;
- validation magic.

All intrusive nodes are initialized but remain unlinked in Phase 3.

## Initial Cortex-M3 frame

The returned saved SP points to R4. The frame contains:

```text
R4-R11                        software-saved by future PendSV
R0-R3, R12, LR, PC, xPSR     hardware exception frame
```

R0 contains the task argument. PC contains the task entry address with bit 0
cleared, while xPSR sets the Thumb bit. LR points to `hr_task_exit_error()` with
the Thumb address bit set.

## Stack protection

- the stack is filled with `0xA5A5A5A5`;
- the lowest word contains `HR_CFG_STACK_GUARD_VALUE`;
- the initial saved SP is aligned to 8 bytes;
- high-watermark scanning counts untouched words above the guard.

## Commands

```bash
make host-tests
make phase3-check
make EXAMPLE=03-static-task-stack TOOLCHAIN=clang
```

## Definition of Done evidence

- TCB storage size is checked at compile time;
- invalid create parameters are tested;
- initial frame words and alignment are tested;
- task metadata and intrusive-node ownership are tested;
- stack guard corruption is detected;
- high-watermark behavior is tested;
- Phase 1 and Phase 2 regression checks still pass;
- the Phase 3 target example compiles and links for Cortex-M3.

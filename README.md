# HairRTOS

HairRTOS is an educational fixed-priority preemptive RTOS for ARM Cortex-M.
The first target is the STM32F103C8T6 Blue Pill. HairEvent is an optional
Event-Driven framework above the kernel.

## Current status: Phase 4 complete

Implemented phases:

- **Phase 0:** specification and principles;
- **Phase 1:** register-level Blue Pill bare-metal foundation;
- **Phase 2:** intrusive lists and host-tested kernel data structures;
- **Phase 3:** opaque static TCB and Cortex-M3 initial task stack;
- **Phase 4:** kernel initialization, idle task, first-task selection, and SVC
  startup from MSP into Thread mode on PSP.

Phase 4 starts one selected task. Task-to-task context switching through PendSV
belongs to Phase 5.

## Roadmap

| Phase | Scope | Status |
|---:|---|---|
| 0 | Specification | ✅ Complete |
| 1 | Bare-metal foundation | ✅ Complete |
| 2 | Intrusive list and kernel data structures | ✅ Complete |
| 3 | TCB and initial task stack | ✅ Complete |
| 4 | Start first task using SVC | ✅ Complete |
| 5 | PendSV cooperative context switch | ⬜ Not started |
| 6 | Priority scheduler | ⬜ Not started |
| 7 | SysTick and delay | ⬜ Not started |
| 8 | Preemption and round-robin | ⬜ Not started |
| 9 | Queue and blocking | ⬜ Not started |
| 10 | Semaphore and mutex | ⬜ Not started |
| 11 | Suspend/resume | ⬜ Not started |
| 12 | Software timer | ⬜ Not started |
| 13 | HairEvent framework | ⬜ Not started |
| 14 | Memory allocator lab | ⬜ Not started |
| 15 | Kernel benchmark | ⬜ Not started |
| 16 | Diagnostics and stabilization | ⬜ Not started |

Detailed deliverables and Definition of Done are in `docs/roadmap.md`.

## Phase 4 components

- kernel lifecycle states RESET, INITIALIZED, RUNNING, and PANIC;
- static idle task registered at the lowest priority;
- `hr_task_start()` transition from CREATED to READY;
- first-task choice using the Phase 2 priority ready set;
- current task and selected TCB handoff to the architecture port;
- strong Cortex-M3 `SVC_Handler`;
- R4–R11 restore and hardware-frame exception return;
- Thread mode PSP selection through `CONTROL.SPSEL`;
- target runtime checks for PSP and R0 argument restoration;
- host tests with ASan/UBSan and target disassembly validation.

## Examples: host versus target

See `examples/README.md` for the complete execution-environment matrix.

Run the host example and tests:

```bash
make phase2-example
make host-tests
```

Build or flash implemented target examples:

```bash
make EXAMPLE=01-baremetal-foundation flash
make EXAMPLE=03-static-task-stack flash
make EXAMPLE=04-start-first-task flash
```

The `EXAMPLE` value must be passed in the same command used for `flash`.
Attempting to flash the Phase 2 host example now stops immediately with a clear
Makefile error.

## Validation

```bash
make phase0-check
make phase1-check
make roadmap-check
make example-layout-check
make phase2-check
make phase3-check
make phase4-check
```

Read:

- `docs/phase1-baremetal-foundation.md`
- `docs/phase2-kernel-data-structures.md`
- `docs/phase3-tcb-initial-stack.md`
- `docs/phase4-start-first-task-svc.md`
- `examples/README.md`

The next implementation phase is **Phase 5 — PendSV cooperative context
switch**.

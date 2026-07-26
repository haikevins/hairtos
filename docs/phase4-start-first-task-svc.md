# Phase 4 — Start First Task Using SVC

## Scope

Phase 4 starts exactly one selected task. It does not perform a task-to-task
context switch; PendSV belongs to Phase 5.

## Kernel lifecycle

```text
RESET
  -> hr_kernel_init()
INITIALIZED
  -> create/start application tasks
  -> hr_kernel_start()
RUNNING
```

`hr_kernel_init()` initializes the ready set and all-task list, creates a static
idle task, and registers idle at the lowest priority.

`hr_task_start()` changes a valid task from CREATED to READY and registers its
intrusive ready/all-task nodes. `HR_CFG_MAX_TASKS` includes the idle task.

## Selection

`hr_kernel_prepare_start()` selects the head of the highest-priority ready
queue. The selected TCB is exposed to assembly through:

```c
hr_task_control_block_t *g_hr_current_task_control_block;
```

The TCB saved stack pointer is statically asserted to be at offset zero.

## SVC startup sequence

```text
main() executes in Thread mode on MSP
        |
        | hr_kernel_start()
        v
hr_port_start_first_task()
        |
        | reset MSP to linker _estack
        | svc #0
        v
SVC_Handler executes in Handler mode on MSP
        |
        | load selected TCB saved SP
        | restore R4-R11
        | set PSP to hardware frame
        | set CONTROL.SPSEL
        | exception-return 0xFFFFFFFD
        v
first task executes in Thread mode on PSP
```

Resetting MSP discards the abandoned `main()` call chain and preserves the full reserved handler stack. The Cortex-M exception return restores R0–R3, R12, LR, PC, and xPSR from the
initial frame created in Phase 3.

## Exception priorities

The Cortex-M3 port configures:

- SVC at the highest configured priority for startup;
- SysTick at a medium priority while it remains the Phase 1 millisecond source;
- PendSV at the lowest priority in preparation for Phase 5.

## Idle task

The idle task is a real static HairRTOS task whose entry loops on `WFI`. It is
always ready but cannot run in Phase 4 after the higher-priority first task has
started, because context switching is not implemented yet.

## Definition of Done

- host tests prove idle registration, READY/RUNNING transitions, and first-task
  selection;
- Cortex-M target builds with a strong `SVC_Handler`;
- disassembly contains `svc` startup and PSP restoration;
- target example checks PSP selection and R0 task argument at runtime;
- physical flash/run validation remains to be performed on the user's board.

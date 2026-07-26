# Kernel Lifecycle

## States

```text
RESET -> INITIALIZED -> RUNNING -> PANIC
```

## RESET

Only startup, board initialization, and static storage exist.

## INITIALIZED

`hr_kernel_init()` initializes the ready set and all-task list, creates the idle
task, and registers idle as READY. Application tasks can then be created and
registered with `hr_task_start()`.

## RUNNING

`hr_kernel_start()` selects the highest-priority READY task and invokes the SVC
startup path. A successful call never returns. The selected task becomes RUNNING
and executes on PSP.

The first-task startup is implemented in Phase 4. PendSV switching between
multiple tasks is not available until Phase 5.

## PANIC

A fatal initialization failure or an unexpected return from the architecture
startup path places the kernel in PANIC.

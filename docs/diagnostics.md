# HairRTOS Diagnostics

Phase 16 adds optional diagnostics behind `HR_CFG_ENABLE_DIAGNOSTICS`. Normal
examples do not link the diagnostics implementation.

## Retained panic record

`hr_panic_record_t` is stored in `.noinit.hairtos`, which the reset handler does
not clear. The record contains the panic reason, boot and sequence counters,
kernel tick, current task name/address, source hash/line, exception frame, and
Cortex-M System Control Block fault registers.

Strong Cortex-M3 handlers capture NMI, HardFault, MemManage, BusFault, and
UsageFault. Fault handling disables interrupts, writes the record, calls the
weak panic hook, and enters a fail-stop breakpoint loop. UART is intentionally
not used from the fault handler.

## Health checks

`hr_diagnostics_run_health_check()` validates:

- all-task intrusive list and registered-task count;
- ready queues, ready bitmap, and ready count;
- timeout list structure;
- task state versus ready/wait/timeout membership;
- current TCB consistency;
- every task stack guard;
- minimum free stack words and low-margin task count.

Health checks are intended for a low-frequency monitor task, not an ISR or the
scheduler hot path.

## Runtime counters

Counters cover SysTick, PendSV, actual task changes, yield, blocking,
preemption, time slicing, timeout wakeups, health checks, stack checks, and
panic records. TCB `runtime_counter` remains the per-task tick counter.

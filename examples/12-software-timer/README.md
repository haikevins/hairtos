# Phase 12 — Software Timer

**Environment:** STM32F103C8T6 target

This example demonstrates:

- one-shot and auto-reload software timers;
- a dedicated timer-service task;
- callback execution in task context instead of SysTick ISR context;
- timer reset, stop, and period change;
- callback wake-up through a kernel semaphore;
- deadline ordering driven by the 1 kHz kernel tick.

Build:

```bash
make EXAMPLE=12-software-timer
```

Flash:

```bash
make EXAMPLE=12-software-timer flash
```

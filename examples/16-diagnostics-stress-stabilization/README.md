# Diagnostics and Stress Stabilization

This integrated target runs queue, semaphore, mutex, software timer,
preemption, timeout, retained-fault diagnostics, runtime statistics, health
checks, and stack-integrity checks together. A priority-1 health monitor prints
a report every second and emits a PASS checkpoint after ten seconds without an
invariant failure.

## Target build and run

```bash
make EXAMPLE=16-diagnostics-stress-stabilization build
make EXAMPLE=16-diagnostics-stress-stabilization run
```

UART: USART1 PA9, 115200 baud, 8-N-1.

## Native scheduler stress

```bash
make ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization build
make ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization run
```

## Fault-injection exercise

Build and flash an intentional UsageFault at the fifth health report:

```bash
make EXAMPLE=16-diagnostics-stress-stabilization \
  EXTRA_DEFINES=-DHR_DIAGNOSTICS_INJECT_USAGE_FAULT=1 run
```

Reset the board after the fault. The next boot prints the retained PC, LR,
CFSR, task name, tick, sequence, and panic reason from the `.noinit` record,
then clears the record and starts the normal stress workload. The macro is
disabled in the normal image.

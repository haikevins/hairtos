# hairtos diagnostics and stabilization

This target runs queue, semaphore, mutex, software-timer, preemption, timeout,
and stack-integrity checks together. A priority-1 health monitor prints a report
every second and emits a PASS checkpoint after ten seconds without an invariant
failure.

```bash
make EXAMPLE=hairtos build
make EXAMPLE=hairtos run
```

UART: USART1 PA9, 115200 baud, 8-N-1.

## Fault-injection exercise

Build with an intentional UsageFault at the fifth health report:

```bash
make EXAMPLE=hairtos \
  EXTRA_DEFINES=-DHR_INJECT_USAGE_FAULT=1 run
```

Reset the board after the fault. The next boot prints the retained PC, LR, CFSR,
task name, tick, sequence, and panic reason from the `.noinit` record, then
clears the record and starts the normal stress workload.

The fault-injection macro is disabled in the normal hairtos image.

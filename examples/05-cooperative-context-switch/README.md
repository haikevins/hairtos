# Phase 5 — Cooperative Context Switch

This target example starts two tasks at the same priority. Task A starts through
SVC. Each task explicitly calls `hr_task_yield()`, which pends PendSV. PendSV:

1. saves R4-R11 below the hardware exception frame on the current PSP;
2. stores the resulting PSP in the current TCB;
3. calls the kernel cooperative selector;
4. loads the next TCB PSP;
5. restores R4-R11 and returns from the exception.

Both tasks retain independent local counters and stack cookies.

## Build and flash

```bash
make EXAMPLE=05-cooperative-context-switch build
make EXAMPLE=05-cooperative-context-switch run
```

## Expected UART pattern

```text
hairtos Phase 5
Two equal-priority tasks switch cooperatively through PendSV.
task=A local_counter=1 -> yield
task=B local_counter=1010 -> yield
task=A local_counter=2 -> yield
task=B local_counter=1020 -> yield
```

`board_delay_ms()` remains a temporary busy wait in this example. True blocking
kernel delay is intentionally deferred to Phase 7.

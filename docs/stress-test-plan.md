# Stress Test Plan

## Native deterministic test

`make phase16-stress` performs 500,000 pseudo-random scheduler insert, remove,
and rotation operations. `hr_scheduler_validate()` runs after every operation.
The seed is fixed, so failures are reproducible.

## Target test

`16-diagnostics-stress-stabilization` continuously exercises:

- preemptive and round-robin scheduling;
- queue full/empty, blocking, timeout, and direct handoff paths;
- counting semaphore wakeups from the software-timer service task;
- mutex ownership around shared statistics;
- periodic timers and timeout lists;
- stack guards, high-water marks, runtime counters, and kernel invariants.

The health monitor reports once per second and emits a ten-second PASS
checkpoint. A release candidate should also be left running for at least eight
hours while checking that operation counters continue increasing and that no
panic record appears after reset.

## Fault injection

Compile the target with `HR_PHASE16_INJECT_USAGE_FAULT=1`. At five seconds it
executes `UDF`. After reset, the UART report must contain a retained UsageFault
record with non-zero PC and CFSR information.

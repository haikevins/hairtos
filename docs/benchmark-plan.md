# Kernel benchmark specification and implementation

Phase 15 implements the original benchmark plan as a dedicated target image.
Correctness tests remain separate from performance measurements.

## Implemented metrics

- SVC first-task startup;
- DWT read-pair measurement overhead;
- critical enter/exit;
- fixed-priority scheduler selection at short and long scan positions;
- queue send/receive;
- semaphore take/give;
- uncontended mutex lock/unlock;
- software timer start/stop;
- PendSV yield round trip with two context switches;
- queue wake/preempt round trip;
- HairEvent post-to-dispatch round trip;
- periodic timer interval and absolute jitter;
- Flash, static RAM, and task stack high-water marks.

## Measurement tools

- DWT_CYCCNT for cycle-level timestamps;
- PB0 active-high pulses for external timing validation;
- deferred USART1 reporting after collection.

## Reporting contract

Every firmware report identifies CPU clock, compiler, optimization,
configuration, sample count, measurement overhead, minimum, maximum, mean, p50,
and p95. Results are not portable guarantees and must be collected again after
changing compiler flags or kernel configuration.

See `docs/kernel-benchmark.md` for the exact measurement boundaries and
interpretation rules.

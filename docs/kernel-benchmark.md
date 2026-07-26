# HairRTOS kernel benchmark

## Purpose

Phase 15 measures implemented kernel and HairEvent paths without adding timing
instrumentation to the normal kernel hot path. The benchmark is a dedicated
STM32F103 firmware and is not a synthetic host-speed comparison.

## Test conditions

The firmware reports these conditions before the result table:

- MCU and core clock;
- compiler version;
- optimization level;
- kernel tick rate;
- preemption and time-slicing configuration;
- minimum measured DWT read-pair overhead.

The supplied Phase 15 configuration uses:

```text
CPU:              STM32F103C8T6 Cortex-M3
Core clock:       board-reported clock, normally 72 MHz
Optimization:     -Og
Kernel tick:      1000 Hz
Preemption:       enabled
Time slicing:     disabled during benchmark collection
Startup probe:    priority 0
Timer task:       priority 1
Receiver task:    priority 2
Event AO:         priority 3
Benchmark/peer:   priority 4
```

Time slicing is disabled so a same-priority context switch occurs only at the
explicit benchmark yield. Higher-priority preemption remains enabled.

## DWT cycle counter

`hr_benchmark_clock_init()` enables CoreDebug DEMCR.TRCENA and DWT_CTRL.CYCCNTENA.
The 32-bit counter increments at the core clock and wraps naturally. Elapsed
cycles are calculated with unsigned subtraction, so short intervals remain
valid across one wrap.

At 72 MHz the counter wraps in roughly 59.6 seconds. The complete sample run is
intentionally much shorter than one wrap.

## Measurement overhead

The firmware records 32 back-to-back DWT read pairs. The minimum observed pair
is used as the fixed measurement overhead for short synchronous metrics:

```text
adjusted = max(raw elapsed - minimum read-pair overhead, 0)
```

The raw DWT read-pair distribution is also printed. Timer interval and jitter
samples are not overhead-adjusted because they are differences between callback
observation timestamps rather than one short bracketed function call.

## Metrics

### SVC startup

Timestamp immediately before `hr_kernel_start()` to the first instruction in
the benchmark task. This includes first-task selection, SVC entry, initial
context restore, PSP selection, and exception return.

### Critical section

One `hr_critical_enter()` plus `hr_critical_exit()` pair.

### Scheduler decision

Direct internal policy measurements for:

- ready task at priority 0, the first bitmap scan position;
- ready task at priority 6, a longer fixed-priority scan.

The scheduler objects are local to the benchmark and do not affect the live
kernel scheduler.

### Non-blocking primitives

- queue send plus receive of one 32-bit item;
- binary semaphore take plus give;
- uncontended mutex lock plus unlock;
- software timer start plus stop.

### PendSV yield round trip

A priority-0 startup probe captures the first-task instruction timestamp and suspends itself. Later, the benchmark task yields to an equal-priority peer. The peer acknowledges and
yields back. One sample therefore includes two PendSV context switches and both
scheduler decisions. The report deliberately labels it as a round trip; it does
not falsely present half the value as a directly measured one-way switch.

### Queue wake/preempt round trip

A priority-1 receiver blocks on an empty queue. The priority-3 benchmark task
sends one word, the receiver preempts it, consumes the word, acknowledges, and
blocks again. The sender then resumes and closes the sample.

### HairEvent post/dispatch round trip

The benchmark task posts one static event to a priority-2 Active Object. The
state handler acknowledges the event, then the AO blocks again before the
benchmark task resumes.

### Software timer interval and jitter

A periodic timer runs every 10 ticks. The priority-0 timer service records 24
callback-to-callback intervals. Absolute jitter is:

```text
abs(observed interval cycles - expected interval cycles)
```

This measures the complete timer-service callback observation path, not raw
SysTick interrupt entry latency.

## Report format

The result table is CSV-like:

```text
metric,count,min,p50,mean,p95,max,mean_ns
```

Percentiles use nearest-rank selection over a fixed-capacity static sample
array. No dynamic memory is used.

The report also includes:

- linked Flash image bytes;
- static RAM through the end of `.bss`;
- stack free-word high-water marks for benchmark, peer, receiver, and event AO.

## GPIO validation

PB0 is active-high around these samples:

- yield round trip;
- queue wake/preempt round trip;
- event post/dispatch round trip.

Connect PB0 and GND to a logic analyzer. The pulse includes the two DWT reads and
marker calls, so it is an independent sanity check rather than an identical
number.

## Interpretation rules

- Compare builds only when CPU clock, compiler, optimization, configuration,
  and interrupt load match.
- Use minimum for the least-interfered path, p95/max for tail behavior, and mean
  only as descriptive context.
- A benchmark cannot prove a hard real-time deadline by itself.
- UART output is deferred until sample collection is complete.
- Physical values must be collected on the user's board; build validation alone
  does not create trustworthy latency numbers.

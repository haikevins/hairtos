# Phase 15 — Kernel benchmark

This STM32F103 target example measures HairRTOS with the Cortex-M3 DWT cycle
counter. It stores all samples in static RAM and prints the CSV-like report only
after collection completes.

Measured paths:

- SVC startup to the first benchmark-task instruction;
- DWT read-pair overhead;
- critical-section enter/exit;
- scheduler selection at priority 0 and priority 6;
- non-blocking queue send/receive;
- semaphore take/give;
- uncontended mutex lock/unlock;
- software-timer start/stop;
- cooperative yield round trip through two PendSV switches;
- queue wake/preempt round trip;
- HairEvent post/dispatch round trip;
- periodic software-timer interval and absolute jitter.

PB0 is driven high around context-switch, queue wake, and event-dispatch samples
for optional logic-analyzer validation.

Build and flash:

```bash
make EXAMPLE=15-kernel-benchmark
make EXAMPLE=15-kernel-benchmark flash
```

Monitor USART1 at 115200 8-N-1. Results are hardware-, compiler-, optimization-,
and interrupt-load-specific; do not copy them between boards as guarantees.

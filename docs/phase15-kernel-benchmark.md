# Phase 15 completion — Kernel benchmark

Phase 15 adds a dedicated Cortex-M3 benchmark firmware without inserting
instrumentation into ordinary HairRTOS kernel paths.

## Delivered

- DWT_CYCCNT clock enable/read support;
- PB0 logic-analyzer marker;
- static sample storage with min, max, mean, p50, and p95;
- measurement-overhead calibration and subtraction;
- SVC startup, critical-section, and scheduler measurements;
- queue, semaphore, mutex, and timer-command measurements;
- two-switch cooperative PendSV round trip;
- higher-priority queue wake/preempt round trip;
- HairEvent post/dispatch round trip;
- periodic software-timer interval and absolute-jitter measurements;
- linked Flash/static-RAM and stack high-water reporting;
- host tests for statistics, capacity, wrap handling, and conversion;
- Make, CMake/Ninja, symbol, disassembly, and regression validation.

## Not claimed

The archive does not contain fabricated physical latency results. The firmware
must be flashed to a real STM32F103C8T6 to obtain values. GPIO timing also
requires an external logic analyzer or oscilloscope.

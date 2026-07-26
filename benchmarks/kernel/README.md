# hairtos kernel benchmark support

This directory contains code that is linked only by the Phase 15 benchmark
firmware.

## Components

- `include/hr_benchmark.h`: fixed-capacity sample statistics and target clock API;
- `src/hr_benchmark_stats.c`: min/max/mean/nearest-rank percentile logic, cycle
  wrap handling, overhead subtraction, and cycles-to-nanoseconds conversion;
- `src/hr_benchmark_dwt.c`: Cortex-M3 DWT_CYCCNT initialization and reads;
- `src/hr_benchmark_gpio.c`: PB0 active-high timing marker for logic analyzers.

The benchmark module is not included in ordinary hairtos applications and does
not modify scheduler, queue, semaphore, mutex, timer, or haievent hot paths.

Build the target:

```bash
make EXAMPLE=15-kernel-benchmark build
make EXAMPLE=15-kernel-benchmark run
```

Run generic statistics tests on the host:

```bash
make host-tests
```

# Phase 15 benchmark validation

The executable benchmark is `examples/15-kernel-benchmark`. Generic statistics,
percentile, wrap-around, and conversion logic is host-tested in
`tests/host/test_benchmark.c`.

Target-only latency paths are intentionally measured on the STM32F103 using
DWT_CYCCNT. They cannot be represented by native host timing because the host
does not execute SVC, PendSV, PSP context switching, or Cortex-M exception
entry/return.

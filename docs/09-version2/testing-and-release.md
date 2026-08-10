# Testing và release plan Version 2

## CI host matrix

Tối thiểu:

```text
GCC + ASan + UBSan
Clang + ASan + UBSan
```

Cần bảo đảm sanitizer runtime invocation ổn định, tránh loader-order issue môi trường.

## Cross-build matrix

```text
all examples × all supported targets × supported toolchains
```

Có thể tối ưu CI bằng smoke/full tiers nhưng release candidate chạy full.

## Property/randomized tests

Candidate:

- intrusive list operation sequences;
- timeout wrap/deadline ordering;
- queue direct handoff;
- mutex ownership/inheritance graph;
- event reference ownership;
- HSM transition trees;
- deferred/recall ownership.

Mọi randomized failure phải in seed.

## Model-based HSM

Reference model đơn giản so expected active state/entry/exit sequence với implementation.

## Hardware smoke

Mỗi target:

```text
01 baremetal
04 first task
05 context
08 preemption
10-01 ISR
12 timer
13-06 haievent
15 benchmark
16 diagnostics
```

## Hardware soak

Scheduled multi-hour run:

- progress counters;
- no health failure;
- stack margins;
- no unexpected reset;
- retained record empty unless injected.

## Fault campaign

Inject supported faults, reset, verify record fields/version/build ID.

## Release gates 2.0

- no known invariant failure;
- migration docs complete;
- at least two hardware targets validated;
- HSM conformance tests;
- trace ring tested;
- tickless validation if advertised;
- benchmark baseline captured.

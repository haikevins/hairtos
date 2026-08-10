# Roadmap Version 2

Roadmap này dùng milestone thay vì phase số học của v1.

## M0 — Baseline freeze

Mục tiêu:

- chốt v1 behavior;
- docs/audit complete;
- host test invocation ổn định;
- hardware baseline Blue Pill;
- known issues list.

Exit criteria: có baseline để biết v2 làm thay đổi gì.

## M1 — Port/interrupt contract v2

- failure policy;
- port capability cleanup;
- BASEPRI path Cortex-M3;
- max syscall IRQ priority;
- conformance tests;
- target template cleanup.

Exit: current target chạy lại full regression.

## M2 — Second target proof

- chọn target;
- SoC/board/driver;
- reuse hoặc port architecture;
- all essential examples;
- fault/benchmark validation;
- manifest scalability refactor chỉ khi cần.

Exit: kernel + haievent chạy trên hai target thật mà không fork generic source.

## M3 — HSM core

- parent state;
- event propagation;
- LCA transitions;
- init substates;
- flat compatibility;
- model tests.

Exit: state transition conformance suite green.

## M4 — Event robustness

- deferred/recall;
- RTC blocking detection;
- AO failure diagnostics;
- event ownership instrumentation.

Exit: no leak under randomized event tests.

## M5 — Tickless / low power

- next-deadline API;
- port sleep contract;
- elapsed tick advancement;
- timer/timeout integration;
- hardware power/wake validation.

Exit: same scheduling semantics with periodic and tickless modes.

## M6 — Trace / observability

- static trace ring;
- kernel event records;
- haievent records;
- panic build identity;
- export tooling.

Exit: reconstruct stress/fault timeline from trace.

## M7 — Release hardening

- compiler matrix;
- target matrix;
- property/randomized tests;
- hardware smoke automation;
- soak;
- migration docs;
- benchmark baselines.

## 2.0.0-rc1

Chỉ tạo RC khi M0–M7 mandatory items hoàn tất.

## Sau 2.0

Candidates 2.1+:

- HSM history;
- shared AO executor;
- event flags/task notifications;
- MPU;
- richer power states;
- more targets.

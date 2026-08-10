# Known limitations của v1

## Kernel

- single-core only;
- không task delete/join;
- không FPU context;
- không MPU isolation;
- PRIMASK critical section;
- periodic tick, chưa tickless;
- không event flags/task notification;
- không priority ceiling/deadlock detector.

## haievent

- flat FSM;
- không HSM/history;
- không deferred/recall event;
- RTC chưa runtime-enforced;
- mỗi AO một task/stack;
- không event priority;
- publish partial delivery semantics.

## Portability

- mới một real target;
- Cortex-M0 chỉ compile probe;
- một số educational EXTI examples trực tiếp dùng STM32F1 registers;
- target manifest v1 còn flat và có thể lặp khi target count tăng.

## Diagnostics

- không trace timeline;
- retained record phụ thuộc reset/RAM target;
- no build-ID/commit ID in panic record.

## Test/release

- chưa hardware CI;
- chưa automated soak;
- cross-build không chứng minh electrical/timing behavior.

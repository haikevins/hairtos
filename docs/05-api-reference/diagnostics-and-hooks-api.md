# Diagnostics và Hooks API

## Initialize

```c
hr_diagnostics_initialize();
```

Chuẩn bị retained record/runtime state theo build configuration.

## Runtime statistics

```c
hr_diagnostics_reset_runtime_statistics();
hr_diagnostics_get_runtime_statistics(&stats);
```

Counters gồm tick, PendSV, switches, yield, block, preemption, slice, timeout wakeup, invariant, stack và panic.

## Task diagnostics

```c
hr_diagnostics_get_task(task, &diag);
```

Trả name/state/priorities/stack usage/runtime ticks/guard.

## Health

```c
hr_diagnostics_run_health_check(&report);
```

Kết hợp kernel internal validation với stack checks.

## Panic record

```c
hr_diagnostics_get_last_panic()
hr_diagnostics_clear_last_panic()
hr_diagnostics_record_panic()
hr_diagnostics_record_fault()
hr_diagnostics_panic_reason_string()
```

Record version hiện là 1.

## Hooks

```c
hr_hook_panic()
hr_hook_stack_overflow()
hr_hook_assert_failed()
```

Có thể override weak hook theo target/application policy.

## Assert

`HR_ASSERT` route tới `hr_assert_failed()` khi feature bật.

## Cảnh báo

Retained record chỉ meaningful nếu linker/startup/reset behavior của target giữ section. Không giả định power-cycle giữ SRAM.

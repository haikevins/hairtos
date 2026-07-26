# Diagnostics và hooks API

## 1. Header

```c
#include "hairtos/hr_diagnostics.h"
#include "hairtos/hr_hooks.h"
```

## 2. Runtime statistics

`hr_diagnostics_get_runtime_statistics()` trả counters SysTick, PendSV, switch reason, timeout, invariant, stack và panic.

## 3. Task diagnostics

`hr_diagnostics_get_task(task, &info)` trả state, priority, stack free/used, runtime ticks và guard status.

## 4. Health check

`hr_diagnostics_run_health_check(&report)` validate scheduler/list/task/stack và tổng hợp task count, ready bitmap, timeout count, minimum stack margin.

## 5. Panic record

- get last panic;
- clear last panic;
- record panic/fault;
- reason-to-string.

Record chứa exception registers và được giữ trong `.noinit` khi the integrated `hairtos` target build.

## 6. Hooks

Application có thể override weak hooks:

```c
void hr_hook_panic(const hr_panic_record_t *record);
void hr_hook_stack_overflow(const hr_task_t *task, const char *name);
void hr_hook_assert_failed(const char *expr, const char *file, uint32_t line);
```

## 7. Assert

`HR_ASSERT(expr)` gọi `_Noreturn hr_assert_failed()` khi bật cấu hình.

## 8. Lưu ý

Diagnostics nâng cao phụ thuộc `HR_CFG_ENABLE_DIAGNOSTICS`; runtime counters phụ thuộc `HR_CFG_ENABLE_RUNTIME_STATS`.

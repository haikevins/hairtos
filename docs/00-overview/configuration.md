# Cấu hình hệ thống

## 1. Mục tiêu

hairtos dùng compile-time configuration trong `config/hairtos_config.h`; haievent dùng `config/haievent_config.h`. Một số example override macro bằng compiler definitions.

## 2. Kernel configuration

| Macro | Mặc định | Ý nghĩa |
|---|---:|---|
| `HR_CFG_TICK_RATE_HZ` | `1000` | Tần số kernel tick |
| `HR_CFG_PRIORITY_COUNT` | `8` | Số mức priority |
| `HR_CFG_IDLE_PRIORITY` | `count - 1` | Priority dành riêng cho idle |
| `HR_CFG_MAX_TASKS` | `8` | Số task tối đa, gồm idle và timer-service khi có |
| `HR_CFG_PREEMPTION` | `1` | Cho phép task priority cao preempt |
| `HR_CFG_TIME_SLICING` | `1` | Round-robin giữa task cùng priority |
| `HR_CFG_TIME_SLICE_TICKS` | `1` | Quantum theo tick |
| `HR_CFG_ENABLE_QUEUE` | `1` | Bật queue API/source khi được link |
| `HR_CFG_ENABLE_SEMAPHORE` | `1` | Bật semaphore |
| `HR_CFG_ENABLE_MUTEX` | `1` | Bật mutex |
| `HR_CFG_ENABLE_SOFTWARE_TIMER` | `1` | Bật timer service |
| `HR_CFG_ENABLE_ASSERT` | `1` | Bật `HR_ASSERT` |
| `HR_CFG_ENABLE_STACK_CHECK` | `1` | Bật stack guard/fill diagnostics |
| `HR_CFG_ENABLE_RUNTIME_STATS` | `0` | Ghi runtime counters |
| `HR_CFG_ENABLE_DIAGNOSTICS` | `0` | Bật diagnostics nâng cao |
| `HR_CFG_IDLE_STACK_WORDS` | `128` | Stack idle task |
| `HR_CFG_TIMER_TASK_PRIORITY` | `idle - 1` | Priority timer-service |
| `HR_CFG_TIMER_TASK_STACK_WORDS` | `160` | Stack timer-service |

## 3. Opaque object sizes

```text
HR_CFG_TASK_STORAGE_BYTES       384
HR_CFG_QUEUE_STORAGE_BYTES      192
HR_CFG_SEMAPHORE_STORAGE_BYTES   96
HR_CFG_MUTEX_STORAGE_BYTES      160
HR_CFG_TIMER_STORAGE_BYTES      160
```

Nếu internal control block tăng kích thước vượt public storage, `_Static_assert` sẽ làm build thất bại. Không giảm các giá trị này mà không kiểm tra mọi toolchain.

## 4. Timeout constants

```c
#define HR_NO_WAIT      0UL
#define HR_WAIT_FOREVER 0xFFFFFFFFUL
```

`HR_WAIT_FOREVER` hợp lệ cho IPC blocking nhưng không hợp lệ cho `hr_task_delay()` và timer period.

## 5. haievent configuration

| Macro | Mặc định | Ý nghĩa |
|---|---:|---|
| `HE_CFG_MAX_ACTIVE_OBJECTS` | `8` | Số AO tối đa trong table/snapshot |
| `HE_CFG_MAX_SIGNALS` | `64` | Signal table tối đa |
| `HE_CFG_MAX_INIT_TRANSITIONS` | `8` | Guard cho init transition loop |
| `HE_CFG_ENABLE_HIERARCHICAL_SM` | `0` | HSM chưa triển khai |
| `HE_CFG_*_STORAGE_BYTES` | tùy object | Opaque storage size |

## 6. Override theo example

- Phase 7 tắt preemption và time slicing để minh họa delay cooperative.
- Phase 15 bật software timer nhưng tắt time slicing để giảm nhiễu benchmark.
- Example `hairtos` bật diagnostics và runtime stats.

## 7. Quy tắc thay đổi

Mỗi thay đổi cấu hình phải được build ít nhất bằng host tests và target example bị ảnh hưởng. Thay đổi priority count phải kiểm tra bitmap width và idle priority. Thay đổi tick rate phải cập nhật mọi timeout trong example/tài liệu.

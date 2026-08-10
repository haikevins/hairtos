# Cấu hình compile-time

## Kernel defaults

| Macro | Mặc định | Ý nghĩa |
|---|---:|---|
| `HR_CFG_TICK_RATE_HZ` | 1000 | Tick rate |
| `HR_CFG_PRIORITY_COUNT` | 8 | Số priority |
| `HR_CFG_IDLE_PRIORITY` | 7 | Idle priority |
| `HR_CFG_MAX_TASKS` | 8 | Task registry limit |
| `HR_CFG_PREEMPTION` | 1 | Preemptive scheduler |
| `HR_CFG_TIME_SLICING` | 1 | Equal-priority slicing |
| `HR_CFG_TIME_SLICE_TICKS` | 1 | Quantum |
| `HR_CFG_STATIC_ALLOCATION` | 1 | Static-first |
| `HR_CFG_DYNAMIC_ALLOCATION` | 0 | Không kernel heap |
| `HR_CFG_ENABLE_QUEUE` | 1 | Queue |
| `HR_CFG_ENABLE_SEMAPHORE` | 1 | Semaphore |
| `HR_CFG_ENABLE_MUTEX` | 1 | Mutex |
| `HR_CFG_ENABLE_SOFTWARE_TIMER` | 1 | Software timer |
| `HR_CFG_ENABLE_ASSERT` | 1 | Assert |
| `HR_CFG_ENABLE_STACK_CHECK` | 1 | Stack diagnostics |
| `HR_CFG_ENABLE_RUNTIME_STATS` | 0 | Runtime counters |
| `HR_CFG_ENABLE_DIAGNOSTICS` | 0 | Advanced diagnostics |
| `HR_CFG_USE_FPU` | 0 | FPU context |
| `HR_CFG_USE_MPU` | 0 | MPU |
| `HR_CFG_SINGLE_CORE` | 1 | Single-core only |

## Opaque object storage

| Object | Bytes |
|---|---:|
| task | 384 |
| queue | 192 |
| semaphore | 96 |
| mutex | 160 |
| timer | 160 |

Internal layout phải fit vào public storage. Nếu layout tăng vượt budget, build phải fail bằng compile-time assertion.

## Stack

```text
idle: 128 words
timer-service: 160 words
minimum application: 32 words
```

Minimum thực còn phụ thuộc `HR_PORT_MIN_TASK_STACK_WORDS`.

## Timeout constants

```c
HR_NO_WAIT
HR_WAIT_FOREVER
```

`HR_WAIT_FOREVER` hợp lệ cho blocking IPC nhưng không hợp lệ như period/delay finite.

## haievent

| Macro | Mặc định |
|---|---:|
| `HE_CFG_ENABLED` | 1 |
| Active Object | 1 |
| Flat FSM | 1 |
| HSM | 0 |
| Time Event | 1 |
| Event Pool | 1 |
| Pub/Sub | 1 |
| Max AO | 8 |
| Max signals | 64 |
| Max init transitions | 8 |

## Port capabilities

Cấu hình generic được validate với `hr_port_config.h`:

```text
minimum stack
stack alignment
FPU context support
MPU support
```

Không bật `HR_CFG_USE_FPU=1` nếu port không lưu FPU context.

## Example override

CMake example definition có thể override macro để cô lập behavior cần minh họa. Ví dụ một bài có thể tắt time slicing để đo scheduler path ổn định hơn.

## Quy tắc thay đổi config

Sau khi thay:

1. host tests;
2. target examples bị ảnh hưởng;
3. image size;
4. stack margins;
5. portability target manifests;
6. docs/API expectations.

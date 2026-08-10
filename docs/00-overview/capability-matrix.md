# Ma trận capability của hairtos 1.0.0-rc1

Ký hiệu:

- **Có**: implementation và test/example hiện diện.
- **Một phần**: có nền tảng nhưng chưa hoàn chỉnh/portable.
- **Chưa**: không được triển khai trong v1.
- **Không chủ đích**: project cố ý không cung cấp trong baseline.

## Kernel

| Capability | Trạng thái | Ghi chú |
|---|---|---|
| Static task creation | Có | Caller-owned TCB + stack |
| Dynamic task creation | Không chủ đích | Kernel không dùng heap |
| First task startup | Có | Qua architecture port |
| Context switch | Có | Cortex-M3 SVC/PendSV |
| Fixed-priority scheduling | Có | Priority nhỏ hơn = cao hơn |
| Preemption | Có | Higher priority READY |
| Equal-priority round-robin | Có | Tick quantum |
| Cooperative yield | Có | `hr_task_yield()` |
| Delay | Có | Relative |
| Periodic delay | Có | `hr_task_delay_until()` |
| Tick wrap handling | Có | Dual timeout lists |
| Task suspend/resume | Có | READY/RUNNING/BLOCKED |
| Task delete/join | Chưa | Không có terminal lifecycle |
| Scheduler lock | Chưa | Không public API |
| Tickless idle | Chưa | Fixed periodic tick |
| SMP | Không chủ đích | Single-core enforced |
| FPU context | Chưa | Port capability = 0 |
| MPU isolation | Chưa | Port capability = 0 |

## IPC và đồng bộ

| Capability | Trạng thái | Ghi chú |
|---|---|---|
| FIFO queue | Có | Fixed item size |
| Queue blocking send/receive | Có | Timeout/forever |
| Queue ISR send/receive | Có | Nonblocking |
| Direct handoff | Có | Sender↔receiver |
| Binary semaphore | Có | Counting max=1 |
| Counting semaphore | Có | Task take, task/ISR give |
| Mutex ownership | Có | Owner tracked |
| Recursive mutex | Có | Optional create mode |
| Priority inheritance | Có | Recompute + chain propagation |
| Priority ceiling | Chưa | Không có |
| Deadlock detection | Chưa | Không có ownership graph diagnostics |
| Event flags | Chưa | Không có |
| Task notification | Chưa | Không có |

## Timer

| Capability | Trạng thái | Ghi chú |
|---|---|---|
| One-shot software timer | Có | Timer-service callback |
| Periodic software timer | Có | Auto reload |
| Start/stop/reset/change period | Có | Task context |
| Timer command from ISR | Chưa | Không public |
| Callback in ISR | Không chủ đích | Callback defer về service task |

## Diagnostics

| Capability | Trạng thái | Ghi chú |
|---|---|---|
| Stack fill/guard | Có | A5 + DEADBEEF |
| Stack high-water mark | Có | Pattern scan |
| Runtime counters | Có | Configurable |
| Kernel invariant check | Có | Internal snapshot/validate |
| Health report | Có | Task/list/timeout/stack |
| Retained panic record | Có | `.noinit.hairtos` |
| Cortex-M fault context | Có | CFSR/HFSR/... |
| Trace timeline | Chưa | Chỉ counters/last fault |
| Build-ID in panic record | Chưa | v2 candidate |

## haievent

| Capability | Trạng thái | Ghi chú |
|---|---|---|
| Static event | Có | Caller-owned |
| Fixed-block dynamic event | Có | Reference counted |
| Event pool diagnostics | Một phần | Free/block counts |
| Flat FSM | Có | ENTRY/EXIT/INIT |
| Hierarchical FSM | Chưa | Config flag = 0 |
| Active Object | Có | 1 task + queue + FSM |
| Time Event | Có | Built on software timer |
| Publish/Subscribe | Có | Fixed subscriber table |
| ISR post | Có | AO ISR API |
| Deferred/recall event | Chưa | Không có |
| RTC runtime enforcement | Chưa | Convention only |
| Shared AO executor | Chưa | 1 task/AO only |
| AO error→panic integration | Một phần | Invalid path currently yield/spin |

## Portability

| Capability | Trạng thái | Ghi chú |
|---|---|---|
| Target manifest | Có | Auto discovery |
| Architecture layer | Có | Cortex-M3 |
| SoC layer | Có | STM32F1 |
| Board layer | Có | Blue Pill |
| Portable GPIO/UART/timer facade | Có | Opaque IDs |
| Benchmark backend abstraction | Có | DWT backend selected by target |
| Second physical target | Chưa | Quan trọng để chứng minh abstraction |
| Cortex-M0 full port | Chưa | Chỉ compile probe |
| Cortex-M4F port | Chưa | Cần FPU handling nếu dùng FP |
| Non-ARM port | Chưa | Cần context/interrupt backend mới |

## Build/test

| Capability | Trạng thái |
|---|---|
| Make wrapper | Có |
| CMake source-of-truth | Có |
| GCC Arm toolchain | Có cấu hình |
| Clang/LLD Arm toolchain | Có cấu hình |
| Host sanitizer tests | Có |
| Deterministic stress | Có |
| 21 target examples | Có |
| Automated hardware CI | Chưa |
| Multi-target build matrix | Một phần — infrastructure có, chỉ một target |
| Long-duration soak automation | Chưa |

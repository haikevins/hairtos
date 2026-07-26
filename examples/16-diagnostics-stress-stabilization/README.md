# `16-diagnostics-stress-stabilization` — Diagnostics and Stress Stabilization

> **Môi trường:** Host + Target  
> **Vị trí mã nguồn:** `examples/16-diagnostics-stress-stabilization/main.c`  
> **Mục đích:** Image tích hợp kiểm tra retained fault, runtime statistics, health invariants, stack guards và workload queue/semaphore/mutex/timer kéo dài.

## 1. Mục tiêu học tập

- Lưu và đọc panic/fault record qua reset bằng `.noinit`.
- Thu runtime counters của scheduler.
- Chạy health check định kỳ trên task/list/timeout/stack invariants.
- Tạo workload liên tục để phát hiện race và corruption.
- Chạy deterministic scheduler stress 500.000 vòng trên host.

## 2. Kiến thức trọng tâm

- Strong fault handlers.
- Panic record retained qua reset.
- Health monitor priority cao.
- Queue producer/consumer, semaphore pulse, mutex-protected counters và periodic timer.
- Fault injection bằng `udf #0`.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| `health-monitor` | Priority 1, stack 224 | Report mỗi 1000 ticks và kiểm tra invariants. |
| `queue-consumer` | Priority 2, stack 144 | Receive sequence và kiểm tra ordering. |
| `timer-pulse` | Priority 2, stack 128 | Take counting semaphore từ timer callback. |
| `queue-producer` | Priority 3, stack 144 | Send mỗi 2 ticks với timeout 10. |
| Message queue | 8 × `uint32_t` | Stress blocking/timeout. |
| Diagnostic timer | Periodic 10 ticks | Give counting semaphore, coalesce khi full. |
| Statistics mutex | Non-recursive | Bảo vệ counters. |
| Host stress | 500.000 iterations | Insert/remove/rotate/validate scheduler structures. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Diagnostics/runtime stats | Bật |
| Preemption/time slicing | Bật |
| Health report period | 1000 ticks |
| PASS checkpoint | Report 10, khoảng 10 giây |
| Fault injection macro | `HR_DIAGNOSTICS_INJECT_USAGE_FAULT=1` |

## 4. Luồng thực thi

1. Boot gọi `hr_diagnostics_initialize()` và in retained panic nếu có.
2. Khởi tạo kernel và các synchronization objects.
3. Start bốn tasks và periodic timer.
4. Producer/consumer tạo traffic; timer/pulse tạo synchronization traffic.
5. Health monitor mỗi giây chạy full health check, lấy runtime counters và snapshot workload.
6. Nếu invariant, stack guard hoặc message order sai thì panic.
7. Tại report 10 in PASS checkpoint.
8. Khi bật injection, report 5 chạy `udf #0`; reset sau fault để đọc record.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hairtos/hairtos.h`

### API trọng tâm

- `hr_diagnostics_initialize()`
- `hr_diagnostics_get_last_panic()`
- `hr_diagnostics_run_health_check()`
- `hr_diagnostics_get_runtime_statistics()`
- Queue/semaphore/mutex/timer/task APIs

### Module được đưa vào build

- `diagnostics`
- `fault`
- `queue`
- `semaphore`
- `mutex`
- `timer`

## 6. Build, run và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Target build | `make ENVIRONMENT=target EXAMPLE=16-diagnostics-stress-stabilization build` |
| Target flash/run | `make ENVIRONMENT=target EXAMPLE=16-diagnostics-stress-stabilization run` |
| Host stress build | `make ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization build` |
| Host stress run | `make ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization run` |
| Target check | `make ENVIRONMENT=target EXAMPLE=16-diagnostics-stress-stabilization check` |
| Fault injection | `make ENVIRONMENT=target EXAMPLE=16-diagnostics-stress-stabilization EXTRA_DEFINES=-DHR_DIAGNOSTICS_INJECT_USAGE_FAULT=1 run` |
| Clean | `make ENVIRONMENT=target EXAMPLE=16-diagnostics-stress-stabilization clean` |

Biến thể target có thể cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang ENVIRONMENT=target EXAMPLE=16-diagnostics-stress-stabilization build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
# Target bình thường
hairtos diagnostics and stabilization
retained panic: none
Starting long-duration stress workload through SVC...
health report=1 tick=1000 tasks=<...> ready=<...> timeout=<...> min_stack_free=<...>
runtime switches=<...> preemptions=<...> slices=<...> blocks=<...>
stress produced=<...> consumed=<...> send_timeouts=<...> timer_pulses=<...>
...
Diagnostics/stress: PASS (10 s checkpoint)

# Host
hairtos scheduler stress: PASS
iterations=500000 insertions=<...> removals=<...> rotations=<...> validations=500000
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- Target health check luôn trả HR_OK.
- `kernel_invariants_valid` và `all_stack_guards_valid` luôn true.
- Consumed tiếp tục tăng và order errors bằng 0.
- Có PASS checkpoint sau 10 report.
- Host stress trả exit code 0 sau 500.000 validations.
- Fault injection tạo retained record có reason/task/PC/LR/CFSR sau reset.

### Lỗi thường gặp

- Health FAIL: đọc report cuối, panic record và task diagnostics.
- Consumed không tăng: deadlock/starvation/queue corruption.
- Stack guard fail: tăng stack hoặc tìm overflow.
- Không có retained record: kiểm tra linker `.noinit`, startup zeroing range và fault handler.
- Host stress sanitizer fail: intrusive-list invariant hoặc invalid lifetime.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=16-diagnostics-stress-stabilization clean
make EXAMPLE=16-diagnostics-stress-stabilization build
```

## 9. Giới hạn của example

- Checkpoint 10 giây không thay thế soak test nhiều giờ trên phần cứng.
- Fault injection phải tắt trong image bình thường.
- Runtime UART report làm tăng tải và có thể ảnh hưởng timing.
- Physical reset/power-cycle behavior của `.noinit` phụ thuộc loại reset và startup/linker thực tế.

## 10. Liên hệ với lộ trình

Đây là example tích hợp cuối. Sau khi PASS, bước tiếp theo là chạy soak test dài, đo trên phần cứng và chuẩn bị release.

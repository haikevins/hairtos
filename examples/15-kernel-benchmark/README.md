# `15-kernel-benchmark` — Benchmark kernel

> **Môi trường:** Target — STM32F103C8T6  
> **Vị trí mã nguồn:** `examples/15-kernel-benchmark/main.c`  
> **Mục đích:** Đo các đường đi quan trọng của kernel bằng DWT_CYCCNT, giữ sample trong RAM tĩnh và in report sau khi thu thập.

## 1. Mục tiêu học tập

- Đo latency bằng cycle counter thay vì UART timestamp.
- Trừ measurement overhead.
- Tính min, p50, mean, p95 và max.
- Đo stack high-water mark, Flash và static RAM.
- Đối chiếu một số đường đi bằng marker PB0.

## 2. Kiến thức trọng tâm

- DWT CYCCNT 32-bit trên Cortex-M3.
- Benchmark perturbation và deferred UART output.
- Startup probe priority 0.
- Round-trip measurement cho yield/wake/event.
- Kết quả phụ thuộc compiler, optimization và interrupt load.

## 3. Thành phần và cấu hình

### Thành phần chính

| Thành phần | Cấu hình | Vai trò |
| --- | --- | --- |
| Phần cứng | STM32F103C8T6 Blue Pill | Chạy firmware target. |
| Nạp/debug | ST-Link V2 qua SWD | Dùng OpenOCD để flash, verify và reset. |
| UART | USART1, PA9 TX / PA10 RX, 115200 8-N-1 | Theo dõi log và trạng thái PASS/FAIL. |
| LED | PC13, active-low | Hiển thị heartbeat hoặc trạng thái quan sát. |
| PB0 | Active-high marker | Bao quanh switch/wake/event samples cho logic analyzer. |
| `startup-probe` | Priority 0, stack 128 | Đo SVC đến instruction đầu tiên. |
| `benchmark-receiver` | Priority 2, stack 192 | Đánh thức qua queue và chiếm quyền. |
| AO sự kiện | Priority 3, stack 224 | Một vòng dispatch của haievent. |
| `benchmark-runner` | Priority 4, stack 320 | Thực hiện và in phép đo. |
| `benchmark-peer` | Priority 4, stack 160 | Một vòng yield. |

### Tham số quan trọng

| Tham số | Giá trị |
| --- | --- |
| Samples thông thường | 32 |
| Timer interval samples | 24 |
| Timer period | 10 ticks |
| Preemption | Bật |
| Time slicing | Tắt để giảm nhiễu benchmark |

## 4. Luồng thực thi

1. Khởi tạo DWT và PB0.
2. Tạo primitive queue, wake queue, semaphore, mutex, timer và event AO.
3. Ghi timestamp trước `hr_kernel_start()`.
4. Startup probe ghi timestamp đầu task rồi suspend.
5. Benchmark runner đo từng primitive và lưu sample.
6. Timer callback thu interval/jitter.
7. Sau khi đủ mẫu, in CSV-like report và stack/footprint.
8. LED nháy sau PASS.

## 5. API và mã nguồn liên quan

### Header được dùng

- `hr_benchmark.h`
- `hairtos/hairtos.h`
- `haievent/haievent.h`
- `hr_scheduler_internal.h` (có chủ đích)

### API trọng tâm

- `hr_benchmark_clock_now()`
- `hr_benchmark_stats_record()`
- `hr_benchmark_stats_percentile()`
- Các API queue/semaphore/mutex/timer/context được đo

### Module được đưa vào bản biên dịch

- `benchmark`
- `haievent_benchmark`
- Toàn bộ kernel primitives cần đo

## 6. Biên dịch, chạy và kiểm tra

Chạy các lệnh từ thư mục gốc chứa `Makefile`:

| Thao tác | Lệnh |
| --- | --- |
| Biên dịch | `make EXAMPLE=15-kernel-benchmark build` |
| Flash và chạy | `make EXAMPLE=15-kernel-benchmark run` |
| Kiểm tra | `make EXAMPLE=15-kernel-benchmark check` |
| Dọn build riêng | `make EXAMPLE=15-kernel-benchmark clean` |

Dùng `TOOLCHAIN=clang` khi cần cross-build bằng Clang/LLD:

```bash
make TOOLCHAIN=clang EXAMPLE=15-kernel-benchmark build
```

## 7. Kết quả mong đợi

Output dưới đây là mẫu. Các giá trị tick, counter, địa chỉ hoặc thống kê có thể thay đổi theo thời điểm chạy và toolchain.

```text
hairtos kernel benchmark
Collecting DWT samples; UART output is deferred.
PB0 is the active-high external timing marker.

hairtos kernel benchmark report
cpu,STM32F103C8T6 Cortex-M3,72000000 Hz
metric,count,min,p50,mean,p95,max,mean_ns
svc_startup,...
critical_section,...
...
Kernel benchmark: PASS
```

## 8. Tiêu chí PASS và xử lý lỗi

### Tiêu chí PASS

- DWT available và report kết thúc bằng PASS.
- Mỗi metric có count hợp lệ.
- p50/p95 nằm giữa min/max.
- Flash/RAM và stack rows được in.
- PB0 có pulse tại các phép đo được đánh dấu.

### Lỗi thường gặp

- `DWT CYCCNT unavailable`: debugger/core config chặn DWT.
- Metric count 0: handshake task hoặc timer không hoàn tất.
- Số đo cực lớn/không ổn định: UART/interrupt/debugger gây nhiễu hoặc counter wrap quanh sample.

Khi example gọi `board_panic()`, LED và UART log ngay trước đó là dữ liệu đầu tiên cần kiểm tra. Với lỗi build/include, chạy lại:

```bash
make EXAMPLE=15-kernel-benchmark clean
make EXAMPLE=15-kernel-benchmark build
```

## 9. Giới hạn của ví dụ

- Kết quả build thành công chỉ xác nhận firmware biên dịch và liên kết; hành vi thời gian thực cần được kiểm chứng trên Blue Pill vật lý.
- UART có thể làm thay đổi timing nếu in quá nhiều; các bài đo timing chuyên dụng sẽ trì hoãn việc in cho đến khi thu mẫu xong.
- Không coi số liệu là guarantee cho board/compiler khác.
- Build mặc định `-Og`; muốn so sánh optimization phải ghi rõ toolchain/flags.
- Benchmark được phép include internal scheduler API, không phải mẫu application bình thường.

## 10. Liên hệ với lộ trình

Bài tiếp theo: [`16-diagnostics-stress-stabilization`](../16-diagnostics-stress-stabilization/README.md). Bài cuối tích hợp diagnostics, fault retention và stress workload dài hạn.

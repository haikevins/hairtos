# Kernel benchmark

## 1. Mục tiêu

Đo latency/overhead trên Cortex-M3 bằng DWT cycle counter mà không đưa UART vào vùng đo.

## 2. Metrics

SVC startup, critical section, scheduler selection, queue, semaphore, mutex, timer command, two-PendSV yield round trip, queue wake/preempt, HairEvent dispatch và timer jitter.

## 3. Statistics

Mỗi metric giữ tối đa 64 sample và tính min, p50, mean, p95, max. Measurement overhead được đo riêng và trừ có clamp về 0.

## 4. Hardware markers

PB0 high/low bao quanh một số path để logic analyzer xác nhận thời gian và thứ tự event.

## 5. Build

```bash
make EXAMPLE=15-kernel-benchmark run
```

## 6. Diễn giải

Cycle phụ thuộc compiler, optimization, Flash wait state, interrupt load và clock. Chỉ so sánh khi giữ cùng điều kiện.

## 7. Không nên làm

- Không in UART giữa start/end timestamp.
- Không dùng một sample làm guarantee.
- Không so số đo host với Cortex-M3.
- Không bỏ qua warm-up/cache/pipeline effects dù M3 không có data cache.

## 8. RAM budget

Benchmark dùng static sample buffers và chỉ link trong Phase 15.

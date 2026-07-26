# Scheduler

## 1. Mục tiêu

Cung cấp fixed-priority scheduling, preemption và round-robin xác định.

## 2. Priority policy

Priority số nhỏ hơn có độ khẩn cấp cao hơn. Mỗi priority có FIFO ready queue. Priority cuối dành cho idle task.

```text
P0: task A -> task B
P1: task C
...
P7: idle
```

Scheduler chọn đầu queue priority thấp nhất đang có bit trong bitmap.

## 3. Cooperative yield

`hr_task_yield()` đặt switch reason `YIELD` và pend PendSV. Nếu có peer cùng priority, current node được rotate về cuối queue. Nếu không có peer, task có thể được chọn lại.

## 4. Preemption

Khi task priority cao hơn trở thành READY do timeout, queue, semaphore, mutex handoff hoặc resume, kernel đánh dấu `PREEMPT` và yêu cầu PendSV sau critical section/ISR.

## 5. Round-robin

Mỗi task có `time_slice_remaining`. SysTick giảm quantum khi preemption và time slicing bật. Khi về 0 và có peer cùng priority, kernel đánh dấu `TIME_SLICE`.

## 6. Switch reasons

```text
YIELD | BLOCK | PREEMPT | TIME_SLICE
```

PendSV selector dùng reason để quyết định state transition và queue rotation.

## 7. Priority inheritance

Mutex có thể thay effective priority của owner. Kernel phải remove/reinsert ready node hoặc wait node để giữ ordering đúng. Chained inheritance được giới hạn bởi số task tối đa.

## 8. Invariants

- RUNNING task vẫn thuộc ready queue cho đến khi block/suspend.
- Idle luôn là lựa chọn cuối.
- Ready bitmap và count luôn đồng bộ.
- Task blocked không có ready node linked.

## 9. Giới hạn

Không có deadline scheduling, EDF, SMP hoặc dynamic priority API cho application.

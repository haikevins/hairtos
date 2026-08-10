# Scheduler

## Policy

Fixed-priority, preemptive, single-core.

```text
priority 0 = cao nhất
...
idle priority = thấp nhất
```

Mỗi priority có FIFO queue.

## Ready representation

```text
ready_bitmap
ready_queues[HR_CFG_PRIORITY_COUNT]
```

Highest selection quét priority từ 0 lên. Với default 8 priority, chi phí nhỏ và deterministic.

## Switch reasons

Kernel phân biệt:

```text
YIELD
BLOCK
PREEMPT
TIME_SLICE
```

Reason quan trọng vì RUNNING task vẫn nằm trong ready queue.

### YIELD

Rotate current priority nếu có peer.

### BLOCK

Current đã bị remove READY hoặc selector thực hiện transition cần thiết; chọn highest còn lại.

### PREEMPT

Current vẫn READY; task priority cao hơn vừa READY. Không rotate current queue vô lý.

### TIME_SLICE

Rotate peers cùng priority khi quantum hết.

## Preemption sources

- timeout wakeup;
- queue handoff;
- semaphore give;
- mutex unlock/handoff;
- resume;
- ISR wakeup.

## Time slice

`time_slice_remaining` giảm theo tick khi feature bật. Khi 0 và có peer cùng priority, request PendSV.

## Priority inheritance interaction

Nếu effective priority task thay đổi:

- ready task phải được move sang ready queue mới;
- blocked waiter có thể cần reorder wait list;
- propagation có thể tiếp tục tới owner upstream.

## Invariants

- bitmap phản ánh queue nonempty;
- running task có đúng state/membership;
- idle là fallback;
- blocked task không có ready node;
- ready ordering dùng effective priority.

## Chưa có

EDF, deadline, sporadic server, CPU budget, SMP, application dynamic priority API.

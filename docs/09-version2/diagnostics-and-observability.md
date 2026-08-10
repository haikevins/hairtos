# Diagnostics và observability Version 2

## V1 hiện có

- last retained panic/fault;
- runtime aggregate counters;
- health report;
- stack watermark/guard.

Thiếu: sequence dẫn tới lỗi.

## Static trace ring

Đề xuất fixed-size ring:

```c
timestamp
event_type
object/task id
arg0
arg1
```

Không lưu string dài trong hot path.

## Candidate trace events

Kernel:

- context switch;
- task READY/BLOCKED/SUSPENDED;
- timeout;
- queue handoff;
- semaphore wake;
- mutex boost/restore/handoff;
- timer expire/callback.

haievent:

- post;
- drop;
- dispatch;
- transition;
- event allocate/free/ref change;
- pubsub publish.

## Build identity

Panic record v2 nên có:

- record version;
- hairtos version;
- build ID/commit short hash nếu build system cung cấp;
- target ID;
- config fingerprint.

## Export

Trace export không thuộc kernel hot path. Có thể:

- debugger memory dump;
- UART command after fault;
- application diagnostics task.

## Cost control

Compile-time config:

```text
trace enable
record count
event masks
timestamp backend
```

Static storage rõ ràng.

## Privacy/safety của trace

Không tự động copy arbitrary application payload/pointers thành log text. Chỉ record metadata nhỏ do subsystem định nghĩa.

## Success criterion

Một timeout/deadlock-like anomaly trong stress test phải có đủ trace để reconstruct task/event order gần lỗi.

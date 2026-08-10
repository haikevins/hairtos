# 02 — Đồng bộ hóa và IPC

Nhóm này mô tả cách task rời READY state để chờ dữ liệu/tài nguyên, cách completion/timeout cleanup TCB, và cách priority/preemption được giữ đúng khi wake.

## Tài liệu

- [blocking-contract.md](blocking-contract.md)
- [queue.md](queue.md)
- [semaphore.md](semaphore.md)
- [mutex.md](mutex.md)
- [suspend-resume.md](suspend-resume.md)
- [software-timer.md](software-timer.md)

## Mẫu blocking chung

```text
try immediate operation
    |
    +-- success -> HR_OK
    |
    +-- HR_NO_WAIT -> resource-specific error
    |
    +-- block allowed
          -> fill TCB wait metadata
          -> insert object wait list
          -> optional timeout list
          -> remove READY
          -> PendSV
          -> wake/timeout
          -> return wait_result
```

Completion phải resolve ownership/data trước khi task chạy lại.

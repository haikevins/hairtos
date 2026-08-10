# 01 — Lõi kernel

Kernel v1 là single-core, fixed-priority, preemptive và static-first. Phần C generic được tách khỏi context/interrupt mechanism của target.

## Tài liệu

1. [kernel-lifecycle.md](kernel-lifecycle.md)
2. [memory-model.md](memory-model.md)
3. [intrusive-data-structures.md](intrusive-data-structures.md)
4. [task-model.md](task-model.md)
5. [scheduler.md](scheduler.md)
6. [context-switch.md](context-switch.md)
7. [interrupt-model.md](interrupt-model.md)
8. [time-and-timeout.md](time-and-timeout.md)
9. [kernel-invariants.md](kernel-invariants.md)

## Execution model

```text
task READY
  -> selected
  -> RUNNING
  -> yield / block / preempt / time-slice
  -> PendSV
  -> next task
```

Kernel policy nằm trong `kernel/`; mechanism save/restore context nằm trong `arch/`.

## Điểm cần nhớ

- RUNNING task vẫn linked trong ready queue.
- Một TCB dùng nhiều intrusive nodes cho nhiều membership.
- Blocking operation có wait metadata + wait list + optional timeout.
- Effective priority có thể khác base priority do mutex.
- Timeout wrap được xử lý bằng current/overflow lists.
- Public object là opaque storage; application không được cast sang internal type.

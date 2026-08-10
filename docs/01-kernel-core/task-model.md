# Task model và TCB

## Public state

```text
INVALID
CREATED
READY
RUNNING
BLOCKED
SUSPENDED
```

## TCB nội bộ

TCB lưu:

- saved stack pointer;
- stack base/top/word count;
- name;
- entry + argument;
- state + suspended resume state;
- base/effective priority;
- wake tick;
- time-slice remaining;
- ready/wait/timeout/all-task nodes;
- wait object/list/buffer/cleanup/result/kind;
- owned mutex list/count;
- runtime counters;
- magic.

Saved SP phải ở offset assembly port mong đợi.

## Create

`hr_task_create_static()`:

1. validate argument/priority/stack;
2. initialize TCB;
3. fill stack + guard;
4. gọi port dựng initial stack frame;
5. state = CREATED.

## Start

`hr_task_start()` register TCB vào all-task list và ready set, chuyển CREATED → READY.

## Running

Scheduler chọn task và state thành RUNNING. Task vẫn giữ ready membership.

## Block

Blocking operation remove ready membership, thiết lập wait metadata và optional timeout, rồi yêu cầu switch.

## Suspend

Suspend không nhất thiết xóa wait membership. Task có `suspended_resume_state` để biết khi resume phải READY hay tiếp tục BLOCKED.

## Priority

`base_priority`: application config.

`effective_priority`: có thể boost do mutex. Scheduler và wait ordering dùng effective priority.

## Task return

Task entry được kỳ vọng không return. v1 route task return tới `hr_task_exit_error()` spin. Đây là v2 cleanup candidate: panic/hook policy rõ ràng hơn.

## Queries

Public API cho name/state/priorities/stack words/high-watermark/guard/current task.

## V1 không có

- delete;
- join;
- restart;
- dynamic stack;
- affinity;
- application set-priority.

# Kernel invariants

Đây là các điều kiện nếu bị phá thì kernel không còn đáng tin cậy.

## Scheduler

- bitmap bit = 1 khi và chỉ khi ready queue tương ứng nonempty;
- task READY/RUNNING có đúng ready membership;
- task BLOCKED không có ready membership;
- priority index nằm trong configured range;
- effective priority phản ánh mutex inheritance.

## Current task

Khi RUNNING:

- current TCB valid;
- đúng một task có state RUNNING;
- current task thuộc scheduler;
- saved/runtime stack metadata hợp lệ.

## Lists

- `next->previous` và `previous->next` đối xứng;
- node linked có `node->list` đúng;
- list size khớp số node;
- node không double-link.

## Wait

Task đang chờ object có:

- wait kind hợp lệ;
- wait object/list tương ứng;
- wait node đúng list;
- result ở trạng thái pending cho tới completion.

## Timeout

Task có timeout:

- timeout node linked đúng current/overflow list;
- wake tick đúng;
- timeout count khớp list totals.

## Mutex

- owner NULL ↔ recursion count 0 theo state phù hợp;
- owner giữ mutex node trong owned list;
- waiter ordering theo effective priority;
- boost không thấp hơn (số lớn hơn) base priority;
- handoff update owner trước khi waiter chạy.

## Stack

- guard valid;
- SP trong vùng stack;
- opaque task object magic/layout valid.

## Diagnostics

`hr_kernel_validate_internal()` và `hr_diagnostics_run_health_check()` hiện thực hóa phần lớn check trên. Invariant failure không nên bị coi là recoverable application error.

## Khi thêm feature

Mọi feature mới phải cập nhật:

1. state model;
2. membership model;
3. validation;
4. regression tests;
5. diagnostics snapshot.

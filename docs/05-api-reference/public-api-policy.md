# Public API policy

## Public vs internal

Public:

```text
kernel/include/hairtos/
haievent/include/haievent/
drivers/include/
boards/<board>/include/board.h
```

Internal:

```text
kernel/internal/
haievent/internal/
```

## Compatibility mục tiêu

Trong v1.x:

- tránh rename/remove public API không cần thiết;
- thêm query/API mới theo backward-compatible form;
- internal layout có thể đổi nếu opaque storage budget vẫn đủ.

Version 2 có thể có breaking changes nhưng cần migration document và deprecated compatibility layer khi chi phí hợp lý.

## Context là một phần contract

Function signature chưa đủ; docs phải nói task/ISR/timer-service/AO-handler context.

## Ownership là một phần contract

Đặc biệt queue buffer và dynamic event.

## Error behavior

Không thêm silent fallback khi invalid state có thể che bug. Status code phải phân biệt argument/state/resource/timeout/context.

## Feature flags

Không public "ghost API" cho feature chưa implement/test. Planned v2 feature chỉ nằm trong docs cho tới khi có source + tests.

# Suspend và resume

## Mục tiêu

Suspend là administrative control, độc lập với lý do task đang chờ.

## READY

Suspend remove task khỏi ready queue và set SUSPENDED với resume state READY.

## RUNNING

Self/current suspend remove current READY membership, set SUSPENDED và request context switch.

## BLOCKED

Task vẫn có thể giữ object wait node và timeout node. Suspend chỉ phủ lên scheduling state.

Nếu event/timeout complete khi suspended:

```text
SUSPENDED(BLOCKED)
  -> logical SUSPENDED(READY)
```

Task chưa vào ready queue cho tới resume.

## Resume

Nếu underlying operation chưa complete -> tiếp tục BLOCKED.

Nếu đã complete -> READY; nếu priority cao hơn current thì preemption có thể xảy ra.

## Cấm

Idle task không được suspend. CREATED/invalid state không dùng runtime suspend semantics.

## Invariant

Suspend không được gây leak wait/timeout membership và không được làm mất wait result.

## V2

Nếu sau này có cancellation/task termination, cần tách rõ "administrative suspend", "cancel wait", "terminate task"; không overload một API.

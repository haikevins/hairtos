# Time và timeout

## Tick

`hr_tick_t` = `uint32_t`. `hr_time_now()` trả current kernel tick.

Default target configuration: 1000 Hz.

## Delay

`hr_task_delay(0)` tương đương yield.

Finite nonzero delay:

```text
RUNNING
 -> remove ready
 -> add timeout
 -> BLOCKED
 -> switch
 -> timeout expires
 -> READY
```

`HR_WAIT_FOREVER` không hợp lệ cho delay đơn thuần.

## `delay_until`

Caller giữ `last_wake_tick`. Deadline mới = previous release + period, không phải `now + period`, nên periodic schedule giảm drift.

## Dual-list timeout

```text
current epoch list
overflow epoch list
```

Nếu absolute deadline nằm sau wrap thì insert overflow. Khi tick wrap, swap lists.

## Timeout completion

Timeout callback/cleanup của blocked IPC gỡ task khỏi object wait list, đặt wait result TIMEOUT rồi đưa task READY nếu không administratively suspended.

## Race một tick

Blocking API có thể request PendSV nhưng tick IRQ đến trước PendSV và timeout task ngay. Selector phải kiểm tra actual state thay vì giả định request BLOCK vẫn còn đúng.

## Wrap-safe elapsed

Unsigned subtraction phù hợp để tính elapsed trong khoảng nhỏ hơn full wrap.

## V2

Tickless idle nên dùng "next deadline" từ timeout/timer system thay vì đổi public semantics ngay. Có thể bổ sung 64-bit uptime diagnostic riêng mà vẫn giữ `hr_tick_t` 32-bit.

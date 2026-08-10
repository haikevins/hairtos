# Blocking contract

## TCB wait context

Khi một API block, TCB lưu đủ dữ liệu để operation hoàn tất sau context switch:

```text
wait kind
wait object
wait list
wait buffer
cleanup callback
wait result
timeout node/deadline
```

Caller stack vẫn tồn tại vì task chỉ bị deschedule, không return khỏi API cho tới khi wake.

## Hai nguồn completion

Operation có thể hoàn tất bởi:

```text
object event
timeout
```

Chỉ một nguồn được thắng. Critical section bảo đảm cleanup membership còn lại trước khi task trở READY.

## Timeout modes

```text
HR_NO_WAIT       không block
finite           block + timeout node
HR_WAIT_FOREVER  block không timeout node
```

## Direct completion

Queue/semaphore/mutex tránh "wake rồi tranh lại resource" bằng cách quyết định data/token/ownership trong critical section trước khi waiter chạy.

## Suspend interaction

Task BLOCKED có thể administratively SUSPENDED. Object event/timeout vẫn có thể hoàn tất wait operation; task logic chuyển thành suspended-ready nhưng chưa enqueue đến `resume()`.

## ISR

ISR-safe API không đi qua blocking path. Nó chỉ thử operation ngay, wake waiter nếu có và báo có cần context switch.

## Rule khi thêm blocking primitive mới

Phải định nghĩa rõ:

- wait kind;
- object wait ordering;
- timeout cleanup;
- suspend behavior;
- ISR behavior;
- direct handoff/ownership;
- effective priority impact;
- host tests cho event-vs-timeout race.

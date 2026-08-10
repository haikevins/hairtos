# Software timer

## Kiến trúc

Timer callback không chạy trong tick ISR.

```text
tick IRQ
 -> timer deadline expires
 -> pending_count++
 -> pending list
 -> give timer-service semaphore
 -> timer-service task
 -> callback
```

## Timer object

Control block lưu period, auto-reload, callback/arg, timeout node, pending node, active flag và pending count.

## Service task

Timer system có static service task/stack/semaphore. Priority do config/example override.

## One-shot

Expiration làm timer inactive, callback được queue cho service task.

## Periodic

Deadline tiếp theo dựa trên schedule deadline, không đơn thuần callback completion time. Nếu service chậm, pending count có thể tích lũy/saturate theo implementation.

## Commands

- create;
- start;
- stop;
- reset;
- change period.

Task context only.

## Stop semantics

Stop remove active deadline và xử lý pending state theo implementation contract; application không nên giả định callback đang chạy có thể bị preempt/cancel giữa chừng.

## Invariants

- active timer có đúng timeout membership;
- pending node không double-link;
- callback không chạy trong ISR;
- period hợp lệ.

## V2

Có thể thêm timer command queue từ ISR nếu use-case rõ, nhưng không nên gọi callback từ ISR.

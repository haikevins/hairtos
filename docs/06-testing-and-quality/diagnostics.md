# Diagnostics

## Runtime counters

Counters cho tick, PendSV, switch reasons, timeout wake, invariant/stack checks và panic.

Counters giúp trả lời "hệ thống có hoạt động không", nhưng không cho biết timeline chi tiết.

## Task snapshot

Diagnostics task info gồm:

- state;
- base/effective priority;
- stack total/free/used;
- runtime ticks;
- guard validity.

## Health check

Health report kết hợp:

```text
kernel invariant validation
+ stack guard checks
+ task counts
+ ready bitmap
+ timeout counts
+ minimum free stack
```

## Retained panic

Record lưu signature/version/boot/sequence/reason/tick/task/source/fault registers.

`.noinit` cho phép đọc record ở boot sau nếu reset giữ SRAM.

## Fault reasons

NMI, HardFault, MemManage, BusFault, UsageFault và software panic reasons.

## Source hash

Record giữ source hash/line thay vì pointer string dài trong retained memory.

## Hạn chế

Không có trace ring buffer. Nếu deadlock/priority anomaly xảy ra nhưng chưa fault, counters có thể không đủ để reconstruct sequence.

## V2

Thêm static trace ring:

```text
timestamp
event type
task/object
small payload
```

Các event quan trọng: switch, block/wake, timeout, mutex inheritance, timer expiry, AO post/dispatch/transition, panic.

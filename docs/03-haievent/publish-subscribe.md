# Publish / Subscribe

## Table

Caller cung cấp subscriber storage cho:

```text
signal_count × max_subscribers_per_signal
```

Mỗi slot là AO pointer.

## Subscribe

Signal phải trong configured range. Duplicate subscription bị từ chối. Unsubscribe compact slot.

## Publish flow

1. validate event/signal;
2. snapshot subscribers dưới critical section;
3. rời critical section;
4. post shared event tới từng AO;
5. count successful deliveries;
6. release publisher dynamic reference;
7. trả first error + delivered count.

Snapshot giúp tránh giữ global interrupt mask suốt các queue posts.

## Dynamic event sharing

Shared post retain reference trước enqueue. Nếu enqueue fail, retain phải rollback/release.

## Partial delivery

Một số subscriber có thể nhận trong khi subscriber khác queue full. Application phải hiểu `delivered_count`, không giả định publish là atomic multicast transaction.

## Chưa có

- wildcard topic;
- topic string;
- retained value;
- event priority;
- dynamic subscriber table;
- QoS.

v2 nên ưu tiên observability và HSM hơn việc biến pub/sub thành message broker lớn.

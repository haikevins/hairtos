# Event model

## Signal

`he_signal_t` = `uint16_t`.

Reserved:

```text
NONE    0
ENTRY   1
EXIT    2
INIT    3
TIMEOUT 4
USER    32 trở lên
```

## Event header

`he_event_t` mang metadata:

- owning pool;
- magic;
- event size;
- reference count;
- signal;
- flags.

Application payload thường đặt `he_event_t` làm field đầu tiên trong struct mở rộng.

## Static event

`he_event_init_static()`:

- pool = NULL;
- không return về pool;
- lifetime do application chịu trách nhiệm.

Dùng tốt cho immutable command/event lặp lại có lifetime global.

## Dynamic event

`he_event_new()`:

- lấy một fixed block;
- event size phải fit block;
- reference count bắt đầu từ 1;
- release cuối cùng trả block về pool.

## Pool

Pool dùng free list trong caller-provided storage. Block size fixed, số block fixed.

Ưu điểm: không external fragmentation, bounded storage.

Nhược điểm: internal fragmentation nếu event sizes chênh lệch; v1 không tự chọn size class.

## Thread safety

Pool retain/release/free-list update được bảo vệ bằng critical wrapper.

## Error cases

- event size quá lớn;
- pool hết block;
- invalid magic;
- retain overflow;
- release invalid/static semantics sai.

## V2

Nên bổ sung pool diagnostics/trace và optional multiple size-class registry ở framework level chỉ khi có use-case rõ; tránh biến framework thành general heap.

# 07 — Labs và examples

Examples là executable specification cho từng capability. Chúng không thay host unit tests, nhưng giúp quan sát behavior thật theo lộ trình.

## Thứ tự

Xem [example-index.md](example-index.md) và [`../../examples/README.md`](../../examples/README.md).

Lộ trình chính:

```text
bare metal
 -> data structures
 -> task stack/start
 -> context switch
 -> scheduler
 -> tick/time
 -> IPC/sync
 -> timer
 -> haievent
 -> allocator experiment
 -> benchmark
 -> diagnostics/stress
```

## Host vs target

Example 02 host-only. 14 và 16 có host+target. Các bài còn lại target.

## Port target mới

Không bắt đầu bằng example 16. Chạy từ 01 lên để cô lập lỗi startup/clock/stack/context/tick trước khi thêm IPC/framework.

## Allocator

Allocator lab không phải kernel heap. Xem [memory-allocator-lab.md](memory-allocator-lab.md).

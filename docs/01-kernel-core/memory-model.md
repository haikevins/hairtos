# Memory model

## 1. Mục tiêu

Giữ memory footprint xác định và tránh heap dependency trong kernel runtime.

## 2. Static-first

Application cấp phát:

```c
static hr_task_t task;
static hr_stack_t stack[256];
static hr_queue_t queue;
static message_t storage[8];
```

Kernel chỉ khởi tạo object trên vùng nhớ đã cung cấp.

## 3. Opaque storage

Public object là union căn chỉnh theo `max_align_t`. Internal control block được đặt bên trong `storage[]`. Cách này ẩn layout nhưng vẫn cho phép cấp phát tĩnh.

## 4. Task stack

Stack Cortex-M3 tăng xuống. Khi tạo task, kernel:

1. fill stack bằng `0xA5`;
2. đặt guard `0xDEADBEEF` tại đáy;
3. căn chỉnh SP 8 byte;
4. tạo software frame R4–R11;
5. tạo hardware-compatible frame R0–R3, R12, LR, PC, xPSR.

`R0` chứa argument; `LR` trỏ tới trap nếu task entry return; `xPSR` bật Thumb bit.

## 5. Stack diagnostics

- `hr_task_stack_guard_is_valid()` kiểm tra guard.
- `hr_task_get_stack_high_watermark()` quét pattern để tính vùng chưa dùng.
- hairtos health check tổng hợp stack margin của mọi task.

## 6. Linker memory

Board linker script dùng Flash 64 KiB và RAM 20 KiB, có vùng main stack/MSP và `.noinit.hairtos` cho retained panic record.

## 7. Allocator lab

Phase 14 có fixed-block pool và first-fit heap nhưng chúng nằm trong `labs/` và không được kernel sử dụng.

## 8. Giới hạn

Không có task stack overflow hardware protection vì MPU chưa được bật. Stack fill chỉ phát hiện sau khi kiểm tra, không ngăn ghi vượt vùng nhớ.

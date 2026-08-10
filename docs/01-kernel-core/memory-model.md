# Memory model

## Static-first

Kernel không cấp phát object runtime từ heap. Application sở hữu storage:

```c
static hr_task_t task;
static hr_stack_t stack[256];
static hr_queue_t queue;
static message_t queue_storage[8];
```

Create API chỉ initialize storage đã tồn tại.

## Opaque public object

Ví dụ:

```c
typedef union hr_task
{
    max_align_t alignment;
    unsigned char storage[HR_CFG_TASK_STORAGE_BYTES];
} hr_task_t;
```

Internal TCB được đặt vào `storage`. Ưu điểm:

- application biết footprint compile-time;
- internal layout không public;
- không cần heap;
- alignment bảo đảm qua `max_align_t`.

Internal build phải static-assert object thật fit vào public storage.

## Stack

`hr_stack_t` hiện là `uint32_t`. Architecture port quyết định:

```text
minimum words
alignment
initial frame
```

Cortex-M3:

- alignment 8 bytes;
- fill `0xA5A5A5A5`;
- guard word `0xDEADBEEF`;
- stack grows downward;
- initial frame dựng để exception return vào task entry.

## High-water mark

Sau khi fill, kernel scan vùng còn nguyên pattern để ước lượng free words. Đây là post-factum diagnostic, không phải hardware protection.

## `.data`, `.bss`, `.noinit`

Startup:

- copy `.data`;
- zero `.bss`;
- không zero retained diagnostics section.

Linker script board chịu trách nhiệm memory map.

## Allocator lab không thuộc kernel

`labs/memory-allocator/` chỉ minh họa pool/first-fit. Không dùng nó để tạo TCB/queue/timer.

## V1 limits

- không MPU guard region;
- stack overflow có thể corrupt trước khi health check phát hiện;
- opaque storage size có fixed headroom;
- không per-object memory provenance tracking.

## V2

MPU và stronger stack protection chỉ nên thêm khi architecture port hỗ trợ thật; không bật macro trước implementation.

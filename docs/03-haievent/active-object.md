# Active Object

## 1. Mục tiêu

Đóng gói một task, event queue và state machine thành execution context độc lập.

## 2. Static creation

Application cung cấp:

- opaque AO object;
- event pointer queue storage;
- task stack;
- initial state handler;
- private context;
- priority.

`he_active_create_static()` tạo queue nội bộ, state machine và hairtos task.

## 3. Execution loop

```text
block receive event
  -> state_machine_dispatch
  -> release dynamic event
  -> receive tiếp
```

Mỗi AO xử lý một event tại một thời điểm.

## 4. Posting

`he_active_post()` dùng task-context queue send. `he_active_post_from_isr()` dùng ISR-safe queue API và trả cờ preemption.

## 5. Ownership

Queue lưu pointer tới event, không copy toàn payload. Vì vậy event lifetime và reference count là phần bắt buộc của thiết kế.

## 6. Priority

AO priority chính là task priority. AO priority cao có thể preempt publisher thấp sau post.

## 7. Diagnostics

API cung cấp tên, pending count, task pointer và state machine pointer.

## 8. Giới hạn

Không có AO deletion, queue resize hoặc multi-thread dispatch cho cùng AO.

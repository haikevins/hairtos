# Software timer

## 1. Mục tiêu

Cung cấp one-shot và periodic callback mà không chạy callback trong SysTick ISR.

## 2. Cấu trúc

Timer control block có timeout node, pending node, period, callback, argument, pending count, active/auto-reload/callback-running flags.

## 3. Timer service

Timer đầu tiên tạo ra hệ thống timer tĩnh gồm ordered timeout list, pending callback list, wake semaphore và timer-service task.

## 4. Tick path

```text
SysTick -> timer expiration -> pending_count++ -> wake timer-service
```

Timer-service lấy từng pending callback và gọi trong task context.

## 5. One-shot

Sau expiration, timer inactive. Pending callback vẫn được xử lý bởi service task.

## 6. Periodic

Deadline tiếp theo được tính từ deadline cũ để giảm drift. Nếu service chậm, `pending_count` có thể lớn hơn 1 và callback được gọi tương ứng.

## 7. Commands

- `start`: chỉ cho inactive timer.
- `stop`: hủy deadline và pending callback chưa chạy.
- `reset`: deadline mới từ thời điểm hiện tại.
- `change_period`: cập nhật period và rearm.

## 8. Invariants

Timer active có timeout node linked; timer pending có pending node linked tối đa một lần; callback không chạy trong ISR.

## 9. Giới hạn

Không có timer command from ISR, delete timer hoặc callback deadline guarantee cứng.

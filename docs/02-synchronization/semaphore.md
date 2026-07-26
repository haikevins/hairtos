# Semaphore

## 1. Mục tiêu

Đồng bộ event/token giữa task và ISR bằng binary hoặc counting semaphore.

## 2. Cấu trúc

Control block gồm current count, max count, priority-ordered wait list và magic.

## 3. Tạo object

```c
hr_semaphore_create_binary(&sem, false);
hr_semaphore_create_counting(&sem, 2U, 8U);
```

Binary semaphore là counting semaphore có `max_count = 1`.

## 4. Take

- Nếu count > 0: giảm count và trả `HR_OK`.
- Nếu count = 0 và `HR_NO_WAIT`: trả `HR_ERROR_SEMAPHORE_EMPTY`.
- Nếu được phép chờ: task BLOCKED trên wait list, có thể kèm timeout.

## 5. Give

Nếu có waiter, token được chuyển trực tiếp cho waiter priority cao nhất; count không tăng trung gian. Nếu không có waiter, tăng count đến max; vượt max trả `HR_ERROR_SEMAPHORE_FULL`.

## 6. ISR

`hr_semaphore_give_from_isr()` có cùng direct handoff nhưng không block và trả cờ preemption.

## 7. Ordering

Waiter priority cao hơn được phục vụ trước; cùng priority FIFO.

## 8. Giới hạn

Không có take-from-ISR và không có semaphore deletion.

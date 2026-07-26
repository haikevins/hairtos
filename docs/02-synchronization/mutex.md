# Mutex và priority inheritance

## 1. Mục tiêu

Bảo vệ resource có ownership, hỗ trợ recursive option và giảm priority inversion.

## 2. Ownership

Mutex lưu owner task, recursion count, waiters và intrusive node để liên kết vào danh sách mutex do owner giữ.

Chỉ owner được unlock. Unlock từ task khác trả `HR_ERROR_NOT_OWNER`.

## 3. Non-recursive và recursive

```c
hr_mutex_create(&mutex);           /* non-recursive */
hr_mutex_create_recursive(&mutex); /* recursive */
```

Owner lock lại non-recursive mutex là invalid/busy; recursive mutex tăng recursion count.

## 4. Blocking lock

Khi mutex có owner khác, waiter được insert theo effective priority và có thể timeout. Mutex có direct ownership handoff khi unlock: owner mới được đặt trước khi waiter chạy.

## 5. Priority inheritance

Nếu waiter có priority cao hơn owner, owner được boost. Effective priority được tính từ:

```text
min(base priority, highest waiter trên mọi mutex đang giữ)
```

Khi unlock hoặc waiter timeout, kernel tính lại thay vì khôi phục mù về base priority.

## 6. Chained inheritance

Nếu boosted owner đang chờ mutex khác, boost được truyền tiếp theo chuỗi. Độ sâu giới hạn bởi `HR_CFG_MAX_TASKS` để tránh cycle vô hạn.

## 7. Invariants

- `recursion_count > 0` khi có owner.
- owner node linked đúng owned-mutex list.
- waiter ordering phản ánh effective priority.
- priority restoration xảy ra sau handoff/timeout.

## 8. Giới hạn

Không có deadlock detection đầy đủ, priority ceiling protocol hoặc mutex từ ISR.

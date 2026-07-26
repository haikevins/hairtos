# Mutex API

## 1. Header

```c
#include "hairtos/hr_mutex.h"
```

## 2. Create

```c
hr_mutex_create(&mutex);
hr_mutex_create_recursive(&mutex);
```

## 3. Queries

`is_valid`, `is_recursive`, `get_owner`, `get_recursion_count`, `get_waiting_tasks`.

## 4. Lock

```c
hr_mutex_lock(&mutex, timeout);
```

Có thể non-blocking, finite hoặc WAIT_FOREVER. Không gọi từ ISR.

## 5. Unlock

```c
hr_mutex_unlock(&mutex);
```

Chỉ owner được gọi. Recursive mutex chỉ handoff khi recursion count về 0.

## 6. Priority inheritance

API không expose thao tác boost; kernel tự động cập nhật effective priority khi waiter xuất hiện/biến mất.

## 7. Ví dụ

```c
if (hr_mutex_lock(&lock, HR_WAIT_FOREVER) == HR_OK)
{
    /* protected resource */
    (void)hr_mutex_unlock(&lock);
}
```

## 8. Lưu ý

Giữ mutex ngắn, không delay lâu khi đang sở hữu, và thiết kế lock ordering để tránh deadlock.

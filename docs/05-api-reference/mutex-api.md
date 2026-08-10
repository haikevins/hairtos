# Mutex API

## Create

```c
hr_mutex_create(&mutex);
hr_mutex_create_recursive(&mutex);
```

## Query

```c
hr_mutex_is_valid()
hr_mutex_is_recursive()
hr_mutex_get_owner()
hr_mutex_get_recursion_count()
hr_mutex_get_waiting_tasks()
```

## Lock

```c
hr_mutex_lock(&mutex, timeout);
```

Task context only. Có priority inheritance nếu block.

## Unlock

```c
hr_mutex_unlock(&mutex);
```

Chỉ owner. Recursive mutex chỉ release ownership khi recursion count về 0.

## Effective priority

Application có thể quan sát task effective priority qua task API, nhưng không tự set inheritance.

## ISR

Mutex API không hợp lệ trong ISR.

## Design advice

Không giữ mutex qua delay dài hoặc external I/O chậm. Thiết kế lock ordering để tránh deadlock vì v1 không detect cycle ở application level.

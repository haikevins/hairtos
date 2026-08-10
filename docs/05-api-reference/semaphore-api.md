# Semaphore API

## Create

```c
hr_semaphore_create_counting(&sem, initial, maximum);
hr_semaphore_create_binary(&sem, initially_available);
```

`initial <= maximum`, maximum > 0.

## Query

```c
hr_semaphore_is_valid()
hr_semaphore_get_count()
hr_semaphore_get_max_count()
hr_semaphore_get_waiting_tasks()
```

## Take

```c
hr_semaphore_take(&sem, timeout);
```

Task context only. Có thể no-wait, finite hoặc forever.

## Give

```c
hr_semaphore_give(&sem);
```

Task context; wake waiter hoặc increment count.

## Give from ISR

```c
hr_semaphore_give_from_isr(&sem, &wake);
```

Nonblocking.

## Ownership

Không có owner. Nếu cần bảo vệ resource critical section giữa tasks, dùng mutex.

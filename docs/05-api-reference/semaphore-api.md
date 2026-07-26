# Semaphore API

## 1. Header

```c
#include "hairtos/hr_semaphore.h"
```

## 2. Create

```c
hr_semaphore_create_binary(&sem, false);
hr_semaphore_create_counting(&sem, initial, maximum);
```

`initial <= maximum`, maximum phải lớn hơn 0.

## 3. Queries

`is_valid`, `get_count`, `get_max_count`, `get_waiting_tasks`.

## 4. Take/Give

```c
hr_semaphore_take(&sem, timeout);
hr_semaphore_give(&sem);
```

Take có thể block; give không block và có thể đánh thức waiter.

## 5. ISR give

```c
bool wake = false;
hr_semaphore_give_from_isr(&sem, &wake);
hr_yield_from_isr(wake);
```

## 6. Return values

Empty/full có status riêng; finite wait có thể timeout.

## 7. Lưu ý

Semaphore không có owner, vì vậy không dùng thay mutex để bảo vệ critical resource cần ownership/priority inheritance.

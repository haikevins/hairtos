# Queue API

## 1. Header

```c
#include "hairtos/hr_queue.h"
```

## 2. Create

```c
hr_queue_create_static(queue, storage, item_size, capacity);
```

Storage phải tồn tại suốt lifetime queue và có ít nhất `item_size * capacity` byte.

## 3. Queries

`is_valid`, `get_count`, `get_capacity`, `get_waiting_senders`, `get_waiting_receivers`.

## 4. Task-context operations

```c
hr_queue_send(queue, item, timeout);
hr_queue_receive(queue, item, timeout);
```

Timeout có thể là `HR_NO_WAIT`, finite hoặc `HR_WAIT_FOREVER`.

## 5. ISR operations

```c
hr_queue_send_from_isr(queue, item, &wake);
hr_queue_receive_from_isr(queue, item, &wake);
```

Không block; caller gọi `hr_yield_from_isr(wake)`.

## 6. Return values

- Full/empty cho non-blocking.
- Timeout cho finite wait.
- Invalid argument/state cho object/buffer sai.
- `HR_ERROR_FROM_ISR` nếu gọi task API trong ISR.

## 7. Ví dụ

```c
typedef struct { uint32_t id; } msg_t;
static hr_queue_t q;
static msg_t storage[4];

hr_queue_create_static(&q, storage, sizeof(msg_t), 4U);
msg_t m = { .id = 1U };
hr_queue_send(&q, &m, 10U);
```

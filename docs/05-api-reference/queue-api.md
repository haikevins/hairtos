# Queue API

## Create

```c
hr_queue_create_static(queue, storage, item_size, capacity);
```

`storage` phải đủ `item_size * capacity` và tồn tại suốt queue lifetime.

## Queries

```c
hr_queue_is_valid()
hr_queue_get_count()
hr_queue_get_capacity()
hr_queue_get_waiting_senders()
hr_queue_get_waiting_receivers()
```

## Task send/receive

```c
hr_queue_send(queue, &item, timeout);
hr_queue_receive(queue, &item, timeout);
```

Timeout:

- `HR_NO_WAIT`;
- finite;
- `HR_WAIT_FOREVER`.

## ISR

```c
hr_queue_send_from_isr(queue, &item, &wake);
hr_queue_receive_from_isr(queue, &item, &wake);
```

Không block.

## Return semantics

Send no-wait full -> `HR_ERROR_QUEUE_FULL`.

Receive no-wait empty -> `HR_ERROR_QUEUE_EMPTY`.

Finite wait hết hạn -> `HR_ERROR_TIMEOUT`.

## Buffer lifetime

Nếu task block, source/destination buffer pointer nằm trong TCB wait context. Vì task stack vẫn tồn tại, local buffer hợp lệ miễn caller không vi phạm C lifetime.

## Không hỗ trợ

Variable-size frame và zero-copy buffer ownership.

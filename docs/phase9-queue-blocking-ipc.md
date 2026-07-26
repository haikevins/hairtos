# Phase 9 — Queue and Blocking IPC

## Scope

Phase 9 introduces statically allocated fixed-size message queues. It does not
add semaphores, mutex ownership, priority inheritance, or dynamic allocation.

## Static queue model

The public `hr_queue_t` is opaque storage. The application separately supplies
the payload array:

```c
static hr_queue_t queue;
static message_t storage[4];

hr_queue_create_static(&queue, storage, sizeof(message_t), 4U);
```

The queue control block stores ring indices, item size, capacity, current count,
and two priority-ordered wait lists. HairRTOS never allocates payload storage.

## Send and receive modes

| Timeout | Behavior when operation cannot complete |
|---|---|
| `HR_NO_WAIT` | Return `HR_ERROR_QUEUE_FULL` or `HR_ERROR_QUEUE_EMPTY`. |
| finite tick count | Block until matched or return `HR_ERROR_TIMEOUT`. |
| `HR_WAIT_FOREVER` | Block until matched; no timeout node is inserted. |

Normal APIs reject ISR context. ISR APIs never block.

## Direct handoff

When a receiver is already blocked, send copies directly into the receiver's
saved destination buffer before waking it. When a full queue has blocked
senders, receive removes one queued item, then atomically copies the highest-
priority sender's pending item into the newly freed slot before waking that
sender.

This reservation-by-copy avoids a wake-up race where another task could consume
the resource before the original waiter resumes.

## Wait ordering

Sender and receiver lists are ordered by effective priority. Smaller priority
numbers wake first. Equal-priority waiters preserve FIFO insertion order.

## Timeout integration

A finite queue wait links one TCB into two independent structures:

1. the queue's sender or receiver wait list;
2. the global timeout list.

Object completion removes the timeout node. Timeout completion removes the
object wait node. The task is then inserted into the scheduler ready set once.

## ISR APIs

```c
hr_queue_send_from_isr(queue, item, &higher_priority_task_woken);
hr_queue_receive_from_isr(queue, item, &higher_priority_task_woken);
```

The caller passes the returned flag to the port's ISR-yield mechanism. No ISR
operation waits for space or data.

## Validation

```bash
make host-tests
make EXAMPLE=09-queue-blocking-ipc
make phase9-check
```

The host suite covers FIFO wrap, full/empty behavior, direct handoff, blocked
sender refill, finite timeout cleanup, ISR wake-up, and equal-priority FIFO
waiters. Physical runtime timing must still be verified on an STM32F103 target.

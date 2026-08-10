# Active Object

## Internal composition

AO control block chứa:

```text
hr_task_t
hr_queue_t
he_state_machine_t
name
magic
```

Application còn cung cấp queue storage và task stack.

## Create

`he_active_create_static()`:

1. validate storage/priority/state handler;
2. create queue lưu `he_event_t *`;
3. init FSM;
4. create static task dùng AO task entry;
5. start task.

## Task loop

```text
start FSM
for ever:
    receive event WAIT_FOREVER
    validate event
    dispatch
    release event
```

## Priority

AO priority chính là RTOS task priority. AO high priority có thể preempt low task/AO sau post.

## Post

Task post dùng `he_active_post()`. ISR post dùng `he_active_post_from_isr()`.

## Error path v1

Nếu AO gặp invalid event/FSM dispatch failure nội bộ, current implementation không có rich framework panic policy; path có thể yield/spin.

Version 2 nên route lỗi framework invariant tới diagnostics hook/panic record.

## RAM trade-off

Dedicated task/AO rất dễ hiểu nhưng stack cost tăng tuyến tính theo AO count.

## Không có

- delete AO;
- resize queue;
- shared executor;
- migration giữa executors;
- per-event priority.

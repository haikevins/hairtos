# Kiến trúc hairtos

## 1. Các layer

```text
+--------------------------------------------------+
| Application / examples                           |
+-------------------------+------------------------+
| haievent                | hairtos public API     |
| event/FSM/AO/pubsub      | task/IPC/time/etc.    |
+-------------------------+------------------------+
| Kernel internals                                  |
| scheduler/list/wait/timeout/TCB                   |
+--------------------------------------------------+
| Architecture port                                 |
| stack/context/critical/ISR/fault/tick adapter     |
+--------------------------------------------------+
| SoC              | Board              | Drivers   |
+--------------------------------------------------+
```

## 2. Kernel và framework khác nhau ở đâu?

Kernel quản lý **CPU execution contexts và synchronization**.

`haievent` quản lý **application behavior dưới dạng event/state**.

Một Active Object không thay thế task; ở v1 nó chứa chính một task. Khi AO queue có event, task AO trở READY và scheduler kernel quyết định khi nào nó chạy.

## 3. Luồng preemption

```text
low task RUNNING
    |
IRQ/tick/IPC makes high task READY
    |
kernel requests context switch
    |
PendSV
    |
save low context
    |
select highest READY
    |
restore high context
```

## 4. Luồng event

```text
producer / ISR / timer
        |
        v
he_active_post()
        |
        v
AO queue
        |
        v
AO task READY
        |
        v
scheduler
        |
        v
he_state_machine_dispatch()
        |
        v
state handler RTC
```

## 5. Luồng time event

```text
target tick IRQ
  -> hr_kernel_tick_from_isr()
  -> timer expiration
  -> pending callback
  -> timer-service task
  -> he time-event callback
  -> AO queue
  -> AO state handler
```

Không có application state handler trong SysTick ISR.

## 6. Memory architecture

Kernel không gọi heap allocator. Object public là opaque storage cố định:

```text
hr_task_t
hr_queue_t
hr_semaphore_t
hr_mutex_t
hr_timer_t
```

Internal object được đặt vào storage này và kiểm tra size bằng static assertion.

`haievent` dùng cùng pattern cho Active Object/FSM/Time Event/PubSub. Dynamic **event payload** là ngoại lệ có kiểm soát: nó đến từ fixed-block event pool do application cung cấp.

## 7. Scheduling model

- single-core;
- preemptive fixed-priority;
- priority range compile-time;
- priority 0 cao nhất;
- idle priority cuối;
- running task vẫn là member ready queue;
- equal-priority FIFO/time slice.

## 8. Blocking model

Một blocking operation liên quan ít nhất ba nơi:

```text
TCB wait metadata
wait list của object
timeout list (nếu timeout hữu hạn)
```

Completion phải cleanup các membership còn lại atomically trước khi task trở READY.

## 9. Port architecture

Kernel gọi `hr_port_*`, không truy cập register Cortex-M. Target manifest binding:

```text
architecture port
+ SoC startup/clock/IRQ
+ board/linker
+ drivers
+ debugger config
```

## 10. Điều không thuộc kiến trúc v1

Không có SMP, userspace/kernelspace, memory protection domain, hierarchical FSM hoặc dynamic kernel object lifecycle.

Xem Version 2: [`../09-version2/architecture.md`](../09-version2/architecture.md).

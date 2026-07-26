# HairRTOS Software Timers

## Purpose

Software timers provide delayed and periodic callback execution without running
application code in the SysTick interrupt. SysTick only advances expiration
state and wakes the dedicated timer-service task.

## Static timer object

Applications allocate `hr_timer_t` statically and create it after
`hr_kernel_init()` but before or after the scheduler starts, provided the timer
system was initialized by the first timer before `hr_kernel_start()`.

```c
static hr_timer_t heartbeat;

hr_timer_create_static(&heartbeat,
                       "heartbeat",
                       500U,
                       true,
                       heartbeat_callback,
                       NULL);
```

The first timer creation lazily creates:

- an ordered timeout list;
- a pending-callback list;
- a binary wake semaphore;
- a statically allocated timer-service task.

## Timer modes

- **One-shot:** becomes inactive after its deadline is reached.
- **Auto-reload:** is rearmed from the expiration deadline, not callback finish
  time, reducing accumulated drift.

## Callback context

Callbacks execute from `timer-service` task context. They may use task-context
APIs, including timer stop, reset, and period change. They must remain bounded:
a long callback delays every other software-timer callback.

The timer-service priority is controlled by `HR_CFG_TIMER_TASK_PRIORITY`. The
Phase 12 example uses priority 1.

## Operations

- `hr_timer_start()` arms an inactive timer and clears stale pending callbacks.
- `hr_timer_stop()` cancels the next deadline and queued callbacks that have not
  started.
- `hr_timer_reset()` rearms the timer from the current timer-list time.
- `hr_timer_change_period()` changes the period and rearms immediately.

These operations are task-context-only in Phase 12. ISR variants are not
provided.

## Expiration flow

```text
SysTick_Handler
    -> hr_kernel_tick_from_isr()
    -> hr_timer_tick_from_isr()
    -> move expired timer to pending list
    -> rearm periodic timer
    -> give timer-service semaphore
    -> PendSV if the timer service has higher priority
    -> timer-service executes callback
```

Timer metadata is updated in critical sections. The callback itself runs after
the critical section has been released.

## Pending expirations

A timer owns a pending counter. If the timer service is delayed and a periodic
timer expires repeatedly, expirations accumulate without inserting the same
intrusive node twice. The service drains one callback invocation per pending
expiration.

## Limitations

- no timer creation from ISR;
- no timer start/stop/reset/change-period from ISR;
- no timer deletion API;
- callback jitter must be measured on physical hardware;
- timer-service starvation remains possible if configured below a permanently
  runnable application task.

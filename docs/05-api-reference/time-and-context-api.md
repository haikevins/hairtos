# Time và Context API

## Time

```c
hr_tick_t hr_time_now(void);
```

Lấy snapshot current kernel tick. Không bảo đảm wall-clock hoặc milliseconds nếu target config đổi tick rate.

Elapsed wrap-safe phổ biến:

```c
hr_tick_t elapsed = hr_time_now() - start;
```

## Critical section

```c
hr_irq_state_t state = hr_critical_enter();
/* short atomic section */
hr_critical_exit(state);
```

Phải exit bằng state tương ứng. Không giữ critical section qua UART/blocking/callback dài.

## ISR detection

```c
bool hr_is_inside_isr(void);
```

Dùng để validate context; không dùng thay synchronization.

## Deferred yield

```c
hr_yield_from_isr(switch_required);
```

Nếu true, port request deferred context switch.

## Example ISR pattern

```c
bool wake = false;
hr_status_t status = hr_semaphore_give_from_isr(&sem, &wake);
if (status == HR_OK)
{
    hr_yield_from_isr(wake);
}
```

## Portability

Public context API không expose PRIMASK/PendSV. Architecture backend có thể thay mechanism mà application không đổi.

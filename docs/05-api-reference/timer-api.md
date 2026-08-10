# Software Timer API

## Create

```c
hr_timer_create_static(timer, name, period_ticks,
                       auto_reload, callback, argument);
```

Timer storage/callback argument do caller quản lý lifetime.

## Queries

```c
hr_timer_is_valid()
hr_timer_is_active()
hr_timer_get_name()
hr_timer_get_period()
hr_timer_get_pending_count()
```

## Commands

```c
hr_timer_start()
hr_timer_stop()
hr_timer_reset()
hr_timer_change_period()
```

Task context only trong v1.

## Callback context

Callback chạy trong timer-service task, không chạy trong tick ISR.

Callback nên ngắn; nếu block lâu sẽ làm trễ callback timer khác.

## Disabled feature

Build có thể compile timer API với feature disabled path và trả `HR_ERROR_NOT_SUPPORTED` tùy mapping/config.

## Không hỗ trợ

Timer delete và ISR command API.

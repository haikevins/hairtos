# Software timer API

## 1. Header

```c
#include "hairtos/hr_timer.h"
```

## 2. Create

```c
hr_timer_create_static(&timer, "name", period_ticks,
                       auto_reload, callback, argument);
```

Period phải khác 0 và khác `HR_WAIT_FOREVER`; callback không NULL.

## 3. Queries

`is_valid`, `is_active`, `get_name`, `get_period`, `get_pending_count`.

## 4. Commands

- `hr_timer_start()` — active từ now.
- `hr_timer_stop()` — hủy active/pending chưa chạy.
- `hr_timer_reset()` — deadline từ now.
- `hr_timer_change_period()` — đổi period và rearm.

## 5. Callback context

Callback chạy trong timer-service task. Có thể gọi task-context API phù hợp nhưng nên ngắn và không block lâu để tránh trễ timer khác.

## 6. Ví dụ

```c
static void on_timer(void *arg) { (void)arg; }
hr_timer_create_static(&timer, "t", 1000U, true, on_timer, NULL);
hr_timer_start(&timer);
```

## 7. Lưu ý

Không có command from ISR và không delete timer.

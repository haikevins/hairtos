# Time và context API

## 1. Header

```c
#include "hairtos/hr_time.h"
#include "hairtos/hr_context.h"
```

## 2. Time

```c
hr_tick_t hr_time_now(void);
```

Trả tick snapshot. Dùng subtraction unsigned để tính elapsed qua wrap:

```c
hr_tick_t elapsed = hr_time_now() - start;
```

## 3. Critical section

```c
hr_irq_state_t hr_critical_enter(void);
void hr_critical_exit(hr_irq_state_t state);
```

Luôn exit bằng state trả về từ lần enter tương ứng.

## 4. Context detection

```c
bool hr_is_inside_isr(void);
```

Dùng để reject API không an toàn trong ISR; không dùng làm cơ chế synchronization.

## 5. Deferred switch

```c
void hr_yield_from_isr(bool switch_required);
```

Nếu true, port pend PendSV. Hàm không switch ngay trong ISR.

## 6. Ví dụ ISR

```c
void EXTI0_IRQHandler(void)
{
    bool wake = false;
    (void)hr_semaphore_give_from_isr(&sem, &wake);
    hr_yield_from_isr(wake);
}
```

## 7. Lưu ý

Critical section dùng PRIMASK nên phải ngắn. Không gọi UART hoặc callback dài khi interrupt đang mask.

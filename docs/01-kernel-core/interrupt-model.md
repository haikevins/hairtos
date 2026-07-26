# Interrupt model

## 1. Mục tiêu

Xác định API nào được gọi trong ISR, cách bảo vệ kernel structure và thời điểm context switch xảy ra.

## 2. Critical section

Cortex-M3 port dùng PRIMASK. `hr_port_enter_critical()` trả trạng thái interrupt cũ; exit khôi phục trạng thái đó, cho phép nesting theo từng task/context.

Public wrapper:

```c
hr_irq_state_t hr_critical_enter(void);
void hr_critical_exit(hr_irq_state_t state);
```

## 3. ISR-safe API

Hiện có:

- `hr_queue_send_from_isr()`
- `hr_queue_receive_from_isr()`
- `hr_semaphore_give_from_isr()`
- `he_active_post_from_isr()`

Các API này không block và có thể trả cờ `higher_priority_task_woken`.

## 4. Yield from ISR

ISR gọi:

```c
hr_yield_from_isr(higher_priority_task_woken);
```

Port chỉ pend PendSV; switch xảy ra sau exception return.

## 5. SysTick

SysTick cập nhật tick, timeout, software timer và quantum. Nó không dispatch callback hoặc state machine.

## 6. Fault handlers

Strong handlers NMI/HardFault/MemManage/BusFault/UsageFault chụp exception frame và SCB registers, ghi retained record rồi halt.

## 7. Quy tắc

- Không gọi API blocking trong ISR.
- Không giữ critical section qua operation dài hoặc UART output.
- Không gọi application callback từ SysTick.
- Driver ISR chỉ tương tác kernel qua public ISR API.

## 8. Giới hạn

Không có BASEPRI-based interrupt ceiling; PRIMASK tạm thời mask toàn bộ configurable interrupts trong critical section.

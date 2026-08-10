# Interrupt model

## ISR detection

Public wrapper:

```c
bool hr_is_inside_isr(void);
```

Cortex-M port đọc exception state.

## Critical section

```c
hr_irq_state_t hr_critical_enter(void);
void hr_critical_exit(hr_irq_state_t state);
```

Cortex-M3 v1 dùng PRIMASK: critical section tạm mask toàn bộ configurable interrupts.

Ưu điểm: đơn giản, dễ audit.

Nhược điểm: high-urgency interrupt cũng bị trì hoãn. Version 2 dự kiến nghiên cứu BASEPRI ceiling.

## ISR-safe APIs

v1 có:

- queue send/receive from ISR;
- semaphore give from ISR;
- Active Object post from ISR.

ISR API:

- không block;
- không dùng finite wait;
- trả/ghi `higher_priority_task_woken`;
- caller gọi `hr_yield_from_isr()`.

## Deferred switch

ISR không trực tiếp restore task stack. Nó pend context switch; PendSV chạy sau ISR nesting phù hợp.

## Tick

Kernel generic không định nghĩa `SysTick_Handler`. Target/architecture adapter gọi:

```c
hr_kernel_tick_from_isr();
```

Target không dùng SysTick có thể cung cấp IRQ backend khác.

## Fault

Cortex-M fault assembly chọn MSP/PSP frame từ EXC_RETURN, capture context rồi gọi diagnostics backend.

## V2 interrupt contract

Nên bổ sung:

- max syscall interrupt priority;
- explicit rule ISR nào được gọi kernel API;
- BASEPRI implementation trên M3/M4;
- validation priority configuration;
- latency benchmark cho critical section.

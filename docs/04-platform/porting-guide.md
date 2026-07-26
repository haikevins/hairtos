# Porting guide

## 1. Mục tiêu

Liệt kê công việc cần làm khi chuyển hairtos sang MCU/CPU khác.

## 2. Tách hai mức port

### Cùng Cortex-M3, MCU khác

Giữ `arch/arm/cortex-m3`; thay `soc/`, `boards/`, driver, startup vector và linker script.

### CPU architecture khác

Phải viết lại initial stack, critical section, context switch, exception startup và fault capture.

## 3. Checklist architecture

- Kiểu stack word và alignment.
- Callee-saved register set.
- Cách chuyển first task.
- Cách request deferred context switch.
- ISR nesting/priority rules.
- Atomic critical-section primitive.
- Tick source.
- Fault frame.

## 4. Checklist platform

- clock frequency thật;
- UART debug;
- idle WFI equivalent;
- linker Flash/RAM;
- vector table;
- OpenOCD/debug config;
- board pins.

## 5. Contract với kernel

TCB saved SP phải ở offset mà assembly dùng. Port phải bảo đảm `hr_port_request_context_switch()` an toàn từ task và ISR. `hr_port_is_inside_isr()` phải chính xác.

## 6. Validation tối thiểu

1. Bare-metal blink/UART.
2. Initial stack host test tương đương.
3. First task startup.
4. Cooperative switch bảo toàn local variable.
5. Preemption/time slicing.
6. ISR semaphore wake.
7. Fault injection.
8. Full host regression.

## 7. Cortex-M0 proof

Repository chỉ có compile proof cho generic C structures; chưa có Cortex-M0 assembly port hoàn chỉnh.

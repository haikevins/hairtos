# Driver abstraction

## Public API

```text
drivers/include/
  hr_gpio.h
  hr_uart.h
  hr_hw_timer.h
```

GPIO pin và UART instance là opaque integers. Generic caller không biết encoding.

## SoC backend

STM32F1 implementation:

```text
drivers/stm32f1/
```

SoC-specific header định nghĩa macro encode pin/instance.

## GPIO

Generic config:

- input;
- output push-pull/open-drain;
- alternate push-pull/open-drain;
- pull none/up/down;
- abstract drive level.

Backend ánh xạ sang register semantics thật.

## UART

Caller truyền instance/baud/TX/RX. Backend tự lấy clock. Điều này tránh board code phải tính baud divider/PCLK.

## Hardware timer

Generic API nhận desired tick rate. Backend target quyết định SysTick/peripheral mechanism.

## Layer boundary

Driver không biết task/scheduler internals. Kernel tick IRQ không phải driver responsibility; architecture adapter gọi kernel tick.

## Khi thêm SoC

Tạo `drivers/<soc>` + target include/source mapping. Chỉ mở rộng public API nếu target thứ hai chứng minh abstraction thiếu khả năng thực tế.

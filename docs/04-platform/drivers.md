# Drivers

## 1. Mục tiêu

Cung cấp peripheral access register-level nhỏ, không phụ thuộc HAL/SPL và không
phụ thuộc kernel internal API.

## 2. Cấu trúc

```text
drivers/
├── include/       # hr_gpio.h, hr_uart.h, hr_hw_timer.h
├── common/        # code dùng chung giữa các SoC tương thích
└── stm32f1/       # implementation register-level cho STM32F1
```

Board và application chỉ include header từ `drivers/include`. Build system chọn
implementation SoC phù hợp. Khi thêm SoC thứ hai, tạo thư mục riêng như
`drivers/stm32f4/`; không đặt hai implementation lẫn trong cùng thư mục driver.

## 3. GPIO

`drivers/stm32f1/hr_gpio_stm32f1.c` cấu hình mode và điều khiển pin. Board layer
giữ mapping PC13/PB0 thay vì kernel biết pin cụ thể.

## 4. UART

`drivers/stm32f1/hr_uart_stm32f1.c` cung cấp polling UART cho log và example.
UART không được dùng trong benchmark critical path hoặc ISR dài.

## 5. Hardware timer và SysTick

`drivers/stm32f1/hr_hw_timer_stm32f1.c` cung cấp millisecond base.
`arch/arm/cortex-m3/hr_baremetal_tick_irq.c` nối SysTick vào driver cho các
example bare-metal. Từ example 07, kernel sở hữu strong SysTick handler qua
`kernel/src/hr_time.c`.

## 6. Dependency

Driver có thể phụ thuộc SoC register definitions nhưng không include
`kernel/internal` hoặc tạo task. ISR driver chỉ gọi public ISR API khi cần.

## 7. Giới hạn

Driver set hiện tại gồm GPIO, UART và timer. DMA, CAN, I2C, SPI và power
management chưa thuộc hairtos core.

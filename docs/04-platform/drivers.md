# Drivers

## 1. Mục tiêu

Cung cấp peripheral access nhỏ, register-level và không phụ thuộc HAL/SPL.

## 2. GPIO

`hr_gpio_stm32f1.c` cấu hình mode và điều khiển pin phục vụ board/example. Board layer giữ mapping PC13/PB0 thay vì kernel biết pin cụ thể.

## 3. UART

`hr_uart_stm32f1.c` cung cấp polling UART cho log/example. UART output không được dùng trong critical path benchmark hoặc ISR dài.

## 4. Hardware timer và SysTick

Driver timer cung cấp millisecond base cho bare-metal phases. Từ Phase 7, kernel sở hữu strong SysTick handler qua `hr_time.c`. Make/CMake chọn đúng một implementation để tránh duplicate symbol.

## 5. Board layer

`board.c` và `board_clock.c` kết hợp GPIO/UART/clock thành API thân thiện cho example.

## 6. Quy tắc dependency

Driver không include scheduler internal và không tạo task. ISR driver có thể gọi public ISR API của kernel.

## 7. Giới hạn

Driver set hiện tại tối thiểu: GPIO, UART và timer. Không có DMA, CAN, I2C, SPI hoặc power-management driver trong hairtos core.

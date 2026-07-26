# Driver layer

## 1. Mục đích

Thư mục `drivers/` cung cấp các interface ngoại vi nhỏ, độc lập với application, đồng thời tách implementation theo SoC. Driver layer hiện phục vụ GPIO, USART1 và SysTick millisecond counter cho target STM32F1.

Mục tiêu chính:

- giữ public driver API ổn định;
- tránh để application thao tác register trực tiếp cho các thao tác cơ bản;
- cho phép thêm SoC thứ hai mà không trộn nhiều implementation trong cùng thư mục;
- không phụ thuộc `kernel/internal` hoặc `haievent/internal`.

## 2. Phạm vi và trách nhiệm

Driver layer chịu trách nhiệm:

- enable GPIO port clock;
- cấu hình GPIO output, alternate-function output và floating input;
- đọc, ghi và toggle GPIO;
- khởi tạo USART1 polling TX/RX;
- tạo SysTick 1 kHz cho các example bare-metal;
- cung cấp millisecond counter và blocking delay dựa trên `WFI`.

Driver layer không chịu trách nhiệm:

- quản lý task hoặc scheduler;
- cung cấp asynchronous UART queue;
- DMA;
- interrupt-driven UART service;
- clock-tree policy của board;
- kernel SysTick handler từ Phase 7 trở đi.

## 3. Cấu trúc thư mục

```text
drivers/
├── README.md
├── include/
│   ├── hr_gpio.h
│   ├── hr_hw_timer.h
│   └── hr_uart.h
├── common/
│   └── hr_systick_baremetal_irq.c
└── stm32f1/
    ├── hr_gpio_stm32f1.c
    ├── hr_hw_timer_stm32f1.c
    └── hr_uart_stm32f1.c
```

Quy ước:

- `include/`: public interface không mang tên SoC trong API;
- `common/`: code dùng lại được cho nhiều target tương thích;
- `<soc>/`: implementation register-level theo SoC.

## 4. Thành phần triển khai

### GPIO

`drivers/stm32f1/hr_gpio_stm32f1.c` ánh xạ `hr_gpio_port_t` sang GPIOA/B/C và cấu hình nibble trong CRL/CRH.

Hỗ trợ:

- push-pull output;
- alternate-function push-pull;
- floating input;
- atomic set/reset bằng BSRR/BRR;
- input read qua IDR;
- toggle dựa trên ODR.

### UART

`hr_uart_stm32f1.c` sử dụng USART1:

```text
TX: PA9, alternate push-pull
RX: PA10, floating input
Mode: polling
```

BRR được tính từ peripheral clock và baud rate do caller cung cấp.

### Hardware timer

`hr_hw_timer_stm32f1.c` cấu hình SysTick ở 1 kHz và duy trì `g_millisecond_tick`.

`drivers/common/hr_systick_baremetal_irq.c` cung cấp `SysTick_Handler()` cho các example bare-metal. Các example kernel-time không link module này; chúng dùng strong kernel SysTick handler trong `kernel/src/hr_time.c`.

## 5. Public API

### GPIO

```c
void hr_gpio_enable_port_clock(hr_gpio_port_t port);
void hr_gpio_config_output_push_pull(hr_gpio_port_t port,
                                     uint32_t pin,
                                     hr_gpio_speed_t speed);
void hr_gpio_config_alternate_push_pull(hr_gpio_port_t port,
                                        uint32_t pin,
                                        hr_gpio_speed_t speed);
void hr_gpio_config_input_floating(hr_gpio_port_t port, uint32_t pin);
void hr_gpio_write(hr_gpio_port_t port, uint32_t pin, bool high);
bool hr_gpio_read(hr_gpio_port_t port, uint32_t pin);
void hr_gpio_toggle(hr_gpio_port_t port, uint32_t pin);
```

### UART

```c
void hr_uart_init(uint32_t baud_rate, uint32_t peripheral_clock_hz);
void hr_uart_write_char(char character);
void hr_uart_write_string(const char *text);
bool hr_uart_try_read_char(char *character);
```

### Bare-metal timer

```c
void hr_hw_timer_init_1khz(uint32_t core_clock_hz);
uint32_t hr_hw_timer_millis(void);
void hr_hw_timer_delay_ms(uint32_t milliseconds);
void hr_hw_timer_tick_isr(void);
```

Các API hiện dùng `void` cho nhiều thao tác cấu hình. Input không hợp lệ được bỏ qua thay vì trả status; đây là lựa chọn đơn giản của driver giáo dục hiện tại.

## 6. Luồng hoạt động

### Bare-metal millisecond delay

```text
board_init()
    |
    +--> hr_hw_timer_init_1khz(core_clock)
              |
              +--> SysTick reload và enable interrupt
                          |
                          +--> SysTick_Handler()
                                      |
                                      +--> hr_hw_timer_tick_isr()
                                                  |
                                                  +--> tick++
```

`hr_hw_timer_delay_ms()` ghi nhận tick bắt đầu, sau đó dùng `WFI` cho tới khi unsigned delta đạt thời gian yêu cầu. Phép trừ unsigned cho phép hoạt động đúng qua wrap 32-bit nếu khoảng chờ nhỏ hơn toàn bộ chu kỳ wrap.

### UART polling transmit

```text
hr_uart_write_string()
    |
    +--> từng ký tự
             |
             +--> chờ USART_SR_TXE
                         |
                         +--> ghi USART_DR
```

## 7. Tích hợp build và dependency

Module `platform` trong `cmake/hairtos_modules.cmake` link:

```text
drivers/stm32f1/hr_gpio_stm32f1.c
drivers/stm32f1/hr_uart_stm32f1.c
drivers/stm32f1/hr_hw_timer_stm32f1.c
```

Module `baremetal_tick` link:

```text
drivers/common/hr_systick_baremetal_irq.c
```

Public include path:

```text
drivers/include
```

Driver implementation được compile với public platform includes, không nhận kernel internal includes.

## 8. Build và kiểm tra

Build example bare-metal dùng cả ba driver và bare-metal SysTick handler:

```bash
make EXAMPLE=01-baremetal-foundation build
```

Build firmware tích hợp dùng GPIO/UART nhưng kernel sở hữu SysTick:

```bash
make EXAMPLE=16-diagnostics-stress-stabilization build
```

Flash example:

```bash
make EXAMPLE=01-baremetal-foundation run
```

Kiểm tra compiler command qua:

```bash
make EXAMPLE=01-baremetal-foundation intellisense
```

Sau đó xem `compile_commands.json` để xác nhận chỉ có `drivers/include` là public header path.

## 9. Bất biến và giới hạn

- Pin phải nhỏ hơn 16; pin/port không hợp lệ bị bỏ qua hoặc trả `false`.
- GPIO API hiện chỉ hỗ trợ port A, B và C.
- UART hiện cố định USART1, PA9/PA10 và polling.
- Không có timeout trong `hr_uart_write_char()`; nếu peripheral không tiến triển, hàm có thể chờ vô hạn.
- `hr_hw_timer_delay_ms()` là blocking delay và chỉ phù hợp cho bare-metal examples.
- Bare-metal SysTick handler và kernel SysTick handler không được link cùng lúc.
- Driver không thread-safe theo nghĩa tổng quát; caller chịu trách nhiệm serialization khi nhiều task dùng cùng peripheral.
- Chưa có API error reporting chi tiết, DMA hoặc low-power peripheral abstraction.

## 10. Mở rộng và tài liệu liên quan

Khi thêm SoC mới, giữ public headers nếu abstraction vẫn phù hợp:

```text
drivers/
├── include/
├── common/
├── stm32f1/
└── stm32f4/
    ├── hr_gpio_stm32f4.c
    ├── hr_uart_stm32f4.c
    └── hr_hw_timer_stm32f4.c
```

Các bước cần làm:

1. thêm implementation dưới `drivers/<soc>/`;
2. thêm register/clock support dưới `soc/<soc>/`;
3. tạo board mới trong `boards/`;
4. định nghĩa module platform tương ứng trong `cmake/hairtos_modules.cmake`;
5. chọn module theo target/board configuration;
6. thêm compile test và target smoke example;
7. chỉ thay public API khi nhiều SoC chứng minh abstraction hiện tại không đủ.

Tài liệu liên quan:

- `docs/04-platform/drivers.md`;
- `docs/04-platform/stm32f103-platform.md`;
- `docs/04-platform/porting-guide.md`.

# Lớp driver

## 1. Mục đích

`drivers/` cung cấp public peripheral API nhỏ và các implementation riêng theo SoC. Mục tiêu là giữ application và board code không phụ thuộc trực tiếp register layout, GPIO port enumeration hoặc peripheral clock của một MCU cụ thể.

## 2. Phạm vi và trách nhiệm

Driver layer hiện cung cấp:

- cấu hình, đọc, ghi và toggle GPIO;
- UART polling TX/RX;
- hardware tick source dùng cho các example bare-metal;
- opaque identifier cho pin và peripheral instance.

Driver layer không quản lý task, scheduler, DMA, asynchronous service hoặc interrupt policy của kernel.

## 3. Cấu trúc thư mục

```text
drivers/
├── README.md
├── include/
│   ├── hr_gpio.h
│   ├── hr_hw_timer.h
│   └── hr_uart.h
└── stm32f1/
    ├── include/
    │   ├── hr_gpio_stm32f1.h
    │   └── hr_uart_stm32f1.h
    ├── hr_gpio_stm32f1.c
    ├── hr_hw_timer_stm32f1.c
    └── hr_uart_stm32f1.c
```

Quy ước:

- `drivers/include/`: interface portable;
- `drivers/<soc>/include/`: pin/peripheral identifier do SoC định nghĩa;
- `drivers/<soc>/*.c`: register-level implementation;
- target manifest chọn đúng implementation và include path.

## 4. Thành phần triển khai

### GPIO

Public API sử dụng `hr_gpio_pin_t` opaque. STM32F1 mã hóa port/pin qua các macro:

```c
HR_STM32F1_GPIO_PIN_A(pin)
HR_STM32F1_GPIO_PIN_B(pin)
HR_STM32F1_GPIO_PIN_C(pin)
```

Encoding này chỉ xuất hiện trong board/SoC-specific code, không xuất hiện trong application generic.

### UART

`hr_uart_config_t` chứa instance, baud rate, TX pin và RX pin. Driver STM32F1 tự lấy peripheral clock từ SoC clock service; caller không truyền PCLK.

### Hardware timer

`hr_hw_timer_init(tick_rate_hz)` nhận tần số tick mong muốn. Implementation target ánh xạ tần số đó sang SysTick hoặc timer phù hợp. IRQ adapter không nằm trong driver generic:

```text
arch/arm/cortex-m3/hr_baremetal_tick_irq.c
arch/arm/cortex-m3/hr_kernel_tick_irq.c
```

Nhờ đó kernel generic không định nghĩa `SysTick_Handler()`.

## 5. API công khai

### GPIO

```c
typedef uint32_t hr_gpio_pin_t;

bool hr_gpio_configure(hr_gpio_pin_t pin,
                       const hr_gpio_config_t *config);
void hr_gpio_write(hr_gpio_pin_t pin, bool high);
bool hr_gpio_read(hr_gpio_pin_t pin);
void hr_gpio_toggle(hr_gpio_pin_t pin);
```

### UART

```c
typedef uint32_t hr_uart_instance_t;

bool hr_uart_init(const hr_uart_config_t *config);
void hr_uart_write_char(char character);
void hr_uart_write_string(const char *text);
bool hr_uart_try_read_char(char *character);
```

### Hardware timer

```c
bool hr_hw_timer_init(uint32_t tick_rate_hz);
uint32_t hr_hw_timer_millis(void);
void hr_hw_timer_delay_ms(uint32_t milliseconds);
void hr_hw_timer_tick_isr(void);
```

## 6. Luồng hoạt động

### Khởi tạo board

```text
board_init()
    |
    +--> SoC clock initialization
    +--> hr_gpio_configure(board-defined pins)
    +--> hr_uart_init(board-defined configuration)
    +--> hr_hw_timer_init(board-defined tick rate)
```

### Tick bare-metal

```text
architecture IRQ adapter
    |
    +--> hr_hw_timer_tick_isr()
             |
             +--> millisecond counter
```

### Tick kernel

```text
architecture IRQ adapter
    |
    +--> hr_kernel_tick_from_isr()
```

Hai adapter không được link trong cùng image.

## 7. Tích hợp build và dependency

Target manifest cung cấp:

```cmake
HAIRTOS_TARGET_PUBLIC_INCLUDES
HAIRTOS_TARGET_PLATFORM_C
HAIRTOS_TARGET_BAREMETAL_TICK_C
HAIRTOS_TARGET_KERNEL_TICK_C
```

`cmake/hairtos_modules.cmake` chỉ ánh xạ các biến target này thành module `platform`, `baremetal_tick` và `kernel_time`; file root không hard-code STM32F1.

Driver implementation chỉ nhận public platform includes, không nhận `kernel/internal`.

## 8. Biên dịch và kiểm tra

```bash
make TARGET=bluepill_f103c8 EXAMPLE=01-baremetal-foundation build
make TARGET=bluepill_f103c8 EXAMPLE=16-diagnostics-stress-stabilization build
make TARGET=bluepill_f103c8 EXAMPLE=01-baremetal-foundation intellisense
```

Example 01 link bare-metal tick adapter. Example 16 link kernel tick adapter.

## 9. Bất biến và giới hạn

- `hr_gpio_pin_t` và `hr_uart_instance_t` chỉ có ý nghĩa với target tạo ra chúng.
- Application generic không được tự suy diễn encoding của opaque identifier.
- UART hiện dùng polling và không có timeout/DMA.
- Hardware timer API hiện chỉ biểu diễn periodic tick source tối giản.
- Driver không tự đảm bảo thread safety; board/application phải serialization khi cần.
- Public abstraction chỉ nên mở rộng khi target thứ hai chứng minh interface hiện tại chưa đủ.

## 10. Thêm SoC mới

Tạo:

```text
drivers/<new-soc>/
├── include/
│   ├── hr_gpio_<new-soc>.h
│   └── hr_uart_<new-soc>.h
├── hr_gpio_<new-soc>.c
├── hr_uart_<new-soc>.c
└── hr_hw_timer_<new-soc>.c
```

Sau đó khai báo các file và include path trong `cmake/targets/<new-target>.cmake`. Không sửa `CMakeLists.txt`, `Makefile` hoặc application source.

Tài liệu liên quan:

- [`../docs/04-platform/drivers.md`](../docs/04-platform/drivers.md);
- [`../docs/04-platform/porting-new-target.md`](../docs/04-platform/porting-new-target.md).

## Liên hệ audit và Version 2

Audit source xác nhận public driver API hiện đã loại GPIO port enum/peripheral clock khỏi application-facing contract. Bước chứng minh tiếp theo là target thứ hai dùng cùng public API mà không sửa kernel/framework.

- [`../docs/00-overview/project-analysis.md`](../docs/00-overview/project-analysis.md)
- [`../docs/04-platform/drivers.md`](../docs/04-platform/drivers.md)
- [`../docs/09-version2/portability-roadmap.md`](../docs/09-version2/portability-roadmap.md)

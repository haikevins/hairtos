# Port một target MCU mới

## 1. Mục tiêu

Cơ chế target tách toàn bộ thông tin phụ thuộc phần cứng khỏi `CMakeLists.txt`,
`Makefile` và cấu hình example. Mỗi target được mô tả bởi đúng một manifest:

```text
cmake/targets/<target>.cmake
```

Sau khi target được thêm, người dùng build bằng:

```bash
make TARGET=<target> EXAMPLE=16-diagnostics-stress-stabilization build
```

## 2. Thành phần cần cung cấp

Một target mới phải cung cấp:

```text
arch/<architecture>/
soc/<soc>/
boards/<board>/
drivers/<soc>/
tools/openocd/<board>.cfg
cmake/targets/<target>.cmake
```

Không sửa source trong `kernel/` hoặc `haievent/` để thêm MCU mới.

## 3. Contract của architecture port

Architecture port phải triển khai các API `hr_port_*`, context switch assembly,
fault capture nếu diagnostics được bật, kernel tick IRQ adapter, bare-metal tick IRQ adapter và file
`hr_port_config.h`.

`hr_port_config.h` phải khai báo tối thiểu:

```c
#define HR_PORT_NAME                         "..."
#define HR_PORT_MIN_TASK_STACK_WORDS         ...
#define HR_PORT_STACK_ALIGNMENT_BYTES        ...
#define HR_PORT_SUPPORTS_FPU_CONTEXT          0
#define HR_PORT_SUPPORTS_MPU                  0
```

Khi `HR_CFG_USE_FPU=1`, port bắt buộc lưu và khôi phục đầy đủ FPU context.

## 4. Contract của SoC và board

SoC chịu trách nhiệm startup, vector table, clock, IRQ và register definition.
Board chịu trách nhiệm pin mapping, UART console, LED, panic, thông tin target,
đo kích thước image/RAM và marker benchmark.

Public board API nằm trong `boards/<board>/include/board.h`.

## 5. Contract của driver

Public driver API không chứa tên port GPIO, clock ngoại vi hoặc register cụ thể
của một SoC. Identifier peripheral và pin là giá trị opaque do target định nghĩa.

Header riêng theo SoC, chẳng hạn:

```text
drivers/stm32f1/include/hr_gpio_stm32f1.h
drivers/stm32f1/include/hr_uart_stm32f1.h
```

chỉ được dùng trong board hoặc implementation dành cho target đó.

## 6. Manifest target

Sao chép `cmake/targets/target_template.cmake.example`, sau đó khai báo:

- CPU flags;
- compile definitions;
- public include directories;
- platform C/ASM sources;
- port, tick và fault sources;
- benchmark clock backend;
- linker script;
- OpenOCD configuration và erase command.

Manifest `.cmake` được tự động phát hiện. Không cần sửa `CMakeLists.txt`,
Makefile, `cmake/hairtos_targets.cmake` hoặc `cmake/hairtos_modules.cmake`.

## 7. Linker contract

Linker script phải cung cấp các symbol startup:

```text
_estack
_sidata
_sdata
_edata
_sbss
_ebss
```

Để benchmark báo cáo bộ nhớ portable, linker script cũng phải cung cấp:

```text
__flash_start__
__flash_image_end__
__ram_start__
__static_ram_end__
```

## 8. Kiểm tra

```bash
make list-targets
make TARGET=<target> EXAMPLE=01-baremetal-foundation build
make TARGET=<target> EXAMPLE=04-start-first-task build
make TARGET=<target> EXAMPLE=08-preemption-round-robin build
make TARGET=<target> EXAMPLE=16-diagnostics-stress-stabilization build
make TARGET=<target> host-tests
```

Sau compile validation cần kiểm tra trên phần cứng: startup, UART, tick,
PendSV/SVC, interrupt wake-up, fault retention và benchmark marker.


## Audit checklist cho target mới

Sau khi manifest configure được, xác nhận thêm:

- không sửa `kernel/` hoặc `haievent/` để target build;
- generic examples không cần SoC register header;
- target tick adapter chỉ có một strong IRQ handler;
- fault handlers không duplicate fallback vector handlers;
- stack port capability khớp config;
- public GPIO/UART identifiers không bị application suy diễn;
- benchmark backend báo unavailable rõ ràng nếu target không có clock/marker;
- `.noinit` retention được test sau reset thật;
- compile database IntelliSense lấy đúng target flags.

Một target thứ hai là bằng chứng quan trọng cho portability v1/v2; compile-only architecture probe không thay thế runtime port.

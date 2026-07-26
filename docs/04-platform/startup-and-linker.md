# Startup và linker

## 1. Mục tiêu

Thiết lập vector table, stack, `.data`, `.bss`, `.noinit` và memory map trước khi vào `main()`.

## 2. Startup sequence

```text
Reset_Handler
  -> copy .data từ Flash sang RAM
  -> zero .bss
  -> không zero .noinit.hairtos
  -> SystemInit/board setup theo source
  -> main
```

## 3. Vector table

Vector đầu là `_estack`, tiếp theo Reset/NMI/fault/SVC/PendSV/SysTick và peripheral IRQ. Strong kernel/example handlers override weak default aliases.

## 4. Linker sections

- `.isr_vector` đặt đầu Flash.
- `.text/.rodata` trong Flash.
- `.data` load Flash, run RAM.
- `.bss` zero tại startup.
- `.noinit.hairtos` giữ panic record qua reset.
- stack top `_estack` ở cuối RAM.

## 5. Garbage collection

Build dùng `-ffunction-sections -fdata-sections` và `--gc-sections`, nên source được link nhưng không referenced có thể bị loại khỏi image.

## 6. Map file

Mỗi target build tạo `.map`; dùng để kiểm tra symbol, section và RAM/Flash consumption.

## 7. Quy tắc sửa

Thay memory size phải sửa linker script và validation limit. Không đặt retained record vào `.bss`. Không thay vector order nếu không đối chiếu STM32F103 reference manual.

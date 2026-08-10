# STM32F103 / Blue Pill target

## Target identity

```text
TARGET=bluepill_f103c8
MCU=STM32F103C8T6
CPU=ARM Cortex-M3
```

## Clock

Board target cố gắng đạt nominal 72 MHz từ HSE 8 MHz + PLL. Platform có clock query để board/report biết actual state.

## Board services

`board.h` cung cấp:

- init;
- board/CPU name;
- core clock;
- LED;
- UART log;
- millis/delay bare-metal;
- flash/static RAM footprint;
- benchmark marker;
- panic.

## GPIO/UART

Public driver không expose STM32 port enum. Board dùng STM32F1 pin encoding header để khai PC13/PA9/PA10/PB0.

UART driver tự lấy peripheral clock từ STM32F1 clock service.

## Benchmark marker

Target tham chiếu dùng PB0 active-high. Đây là board detail, không phải benchmark API generic.

## Debug

ST-Link/SWD/OpenOCD config nằm dưới `tools/openocd`.

## Memory

Linker profile dùng board-specific Flash/RAM regions và export symbols cho diagnostics/benchmark footprint.

## Portability note

Application không nên hard-code PA9/PC13/PB0. Một số educational EXTI examples vẫn truy cập STM32F1 register trực tiếp để minh họa ISR; chúng cần adapter nếu chạy target khác.

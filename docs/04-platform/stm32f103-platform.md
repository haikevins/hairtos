# STM32F103/Blue Pill platform

## 1. Mục tiêu

Mô tả MCU, board clock, memory và wiring mặc định.

## 2. Target

- MCU: STM32F103C8T6.
- CPU: ARM Cortex-M3.
- Flash linker region: 64 KiB.
- RAM linker region: 20 KiB.
- Board: Blue Pill.

## 3. Clock

Board cố gắng dùng HSE 8 MHz và PLL x9 để đạt 72 MHz. Nếu HSE không start, code giữ HSI fallback và driver dùng clock thực tế.

## 4. LED

PC13 active-low. Board API cung cấp init/toggle/panic behavior cho example.

## 5. UART

USART1:

| Signal | Pin |
|---|---|
| TX | PA9 |
| RX | PA10 |
| GND | GND |

Mặc định 115200, 8-N-1.

## 6. Debug

ST-Link V2 qua SWDIO/SWCLK/GND/3.3 V reference. OpenOCD config không yêu cầu NRST và dùng system/software reset sau program.

## 7. Interrupts

Startup vector table định nghĩa core exceptions và STM32F1 IRQs. Weak handlers có thể được example/kernel override bằng strong symbol.

## 8. Giới hạn

Blue Pill clone có thể có Flash/RAM thực khác marking; linker vẫn dùng profile 64/20 KiB để an toàn.

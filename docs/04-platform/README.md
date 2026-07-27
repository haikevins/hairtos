# 04 — Nền tảng và port

## 1. Mục tiêu

Nhóm này mô tả các phần phải thay hoặc cung cấp khi thêm architecture, SoC, board hoặc MCU target mới.

## 2. Nội dung

- [cortex-m3-port.md](cortex-m3-port.md)
- [stm32f103-platform.md](stm32f103-platform.md)
- [startup-and-linker.md](startup-and-linker.md)
- [drivers.md](drivers.md)
- [porting-guide.md](porting-guide.md)
- [porting-new-target.md](porting-new-target.md)

## 3. Ranh giới trách nhiệm

- `arch/`: context switch, exception entry, tick adapter, fault backend và benchmark clock.
- `soc/`: startup, clock, IRQ và register map.
- `boards/`: board service, pins, linker script và marker hooks.
- `drivers/`: public peripheral API và implementation theo SoC.
- `cmake/targets/`: binding tất cả thành một selectable target.

## 4. Quy trình port

Bắt đầu từ `porting-new-target.md`, dùng target template, cross-build example 01, rồi tiến dần tới first task, scheduling, time, IPC, benchmark và diagnostics.

## 5. Điều kiện hoàn tất

Target chỉ được coi là port hoàn chỉnh sau khi build, flash, reset, UART, tick, context switch, ISR wakeup, fault record và soak test đều được xác nhận trên hardware.

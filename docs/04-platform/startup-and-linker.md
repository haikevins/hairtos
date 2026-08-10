# Startup và linker contract

## Startup nhiệm vụ

Reset handler:

1. lấy stack top từ vector;
2. copy `.data` load image Flash → RAM;
3. zero `.bss`;
4. giữ `.noinit.hairtos`;
5. gọi system initialization/main flow.

## Vector table

SoC startup cung cấp Reset/core exception/peripheral IRQ entries. Weak fallback handlers có thể được strong kernel/fault/example handler override.

## Linker sections

```text
.isr_vector
.text / .rodata
.data
.bss
.noinit.hairtos
```

Retained diagnostics không được đặt vào zeroed `.bss`.

## Symbols

Board/linker export các symbol memory được board footprint API dùng, ví dụ flash image end và static RAM end.

## MSP/PSP

`_estack` là main/handler stack top. Task PSP đến từ caller-owned task stacks, không từ linker global task heap.

## GC sections

Target build dùng function/data sections + linker garbage collection. Vì vậy "source file có trong target" không đồng nghĩa mọi function đều chiếm image.

## Port target mới

Linker script phải khớp đúng memory map MCU/bootloader. Không copy nguyên Blue Pill script rồi chỉ đổi tên.

## Diagnostics test

Fault retention chỉ được coi là hợp lệ sau reset test trên hardware, vì reset loại nào giữ SRAM là vấn đề target-specific.

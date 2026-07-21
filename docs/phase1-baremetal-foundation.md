# Phase 1 — Bare-metal foundation

## Deliverables

Phase 1 provides a buildable STM32F103C8T6 program without HAL or SPL:

- vector table and Reset handler;
- `.data` copy and `.bss` clearing;
- linker script for 64 KiB flash and 20 KiB RAM;
- HSE 8 MHz to PLL 72 MHz configuration with HSI fallback;
- Flash wait-state and prefetch configuration;
- GPIO driver and onboard PC13 LED;
- USART1 polling driver at 115200 baud;
- 1 kHz SysTick millisecond time base;
- Makefile, CMake target, OpenOCD, and GDB files;
- bare-metal blink and UART heartbeat example.

## Boot flow

```text
CPU reset
  -> vector table loads MSP and Reset_Handler
  -> copy .data from flash to RAM
  -> clear .bss
  -> SystemInit
       -> reset clock tree to HSI
       -> set VTOR
       -> start HSE
       -> set Flash latency to 2 wait states
       -> PLL = HSE x9
       -> SYSCLK = PLL, HCLK = 72 MHz
       -> PCLK1 = 36 MHz, PCLK2 = 72 MHz
  -> main
       -> GPIO PC13
       -> USART1
       -> SysTick 1 kHz
       -> heartbeat loop
```

## Why SysTick exists before the RTOS

In Phase 1 SysTick is a simple bare-metal millisecond counter used to verify the
clock and interrupt foundation. In the scheduler phase, the kernel becomes the
owner of SysTick and replaces `hr_hw_timer_stm32f1.c`'s handler.

## Clock fallback

If HSE or PLL does not become ready within a bounded loop, firmware stays on HSI
at 8 MHz. `SystemCoreClock`, UART baud calculation, and SysTick reload remain
consistent with the fallback clock.

## No NRST requirement

The OpenOCD configuration uses `reset_config none`, so the ST-Link connection
requires SWDIO, SWCLK, GND, and target voltage reference. A physical NRST wire
is not required for normal software reset and flash programming, although it can
help recover a target whose firmware disables SWD or immediately enters a mode
that prevents connection.

## Completion criteria

Phase 1 is complete when:

1. `make` produces ELF, BIN, HEX, and MAP files;
2. `make flash` verifies flash contents and resets the target;
3. PC13 toggles every 500 ms;
4. UART1 prints the boot banner and heartbeat;
5. the map file keeps static data within 20 KiB RAM;
6. `make phase1-check` passes.

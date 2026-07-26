# 00 — Bare-metal blink

This is the executable deliverable for hairtos Phase 1.

## Hardware

- STM32F103C8T6 Blue Pill
- ST-Link V2 using SWDIO, SWCLK, GND, and 3.3 V reference
- optional USB-to-UART adapter

UART1 wiring:

| Blue Pill | USB-to-UART |
|---|---|
| PA9 (TX) | RX |
| PA10 (RX) | TX |
| GND | GND |

Serial settings: `115200 8-N-1`.

The onboard LED is on PC13 and is active-low.

## Build

```bash
make EXAMPLE=01-baremetal-foundation build
```

Expected outputs:

```text
build/target/01-baremetal-foundation/hairtos.elf
build/target/01-baremetal-foundation/hairtos.bin
build/target/01-baremetal-foundation/hairtos.hex
build/target/01-baremetal-foundation/hairtos.map
```

For a local Clang/LLD validation build:

```bash
make TOOLCHAIN=clang EXAMPLE=01-baremetal-foundation build
```

## Flash

```bash
make EXAMPLE=01-baremetal-foundation run
```

The OpenOCD configuration uses SWD without an NRST connection. It requests a
software/system reset after programming.

## Expected UART output

```text
hairtos Phase 1
Board: Blue Pill STM32F103C8T6
Core clock: 72000000 Hz
Clock source: HSE 8 MHz -> PLL x9
Bare-metal foundation ready.
LED PC13 toggles every 500 ms.
heartbeat=1 uptime_ms=0
heartbeat=2 uptime_ms=500
```

If the 8 MHz external crystal does not start, firmware remains on the 8 MHz HSI
clock and prints a warning. UART and SysTick use the detected clock, so the
example still runs.

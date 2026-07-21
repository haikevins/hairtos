# Changelog

## Unreleased

### Added — Phase 1

- Buildable STM32F103C8T6 bare-metal target.
- Reset handler, vector table, `.data` initialization, and `.bss` clearing.
- Blue Pill linker script for 64 KiB flash and 20 KiB RAM.
- 72 MHz HSE/PLL clock configuration with HSI fallback.
- Register-level GPIO, USART1, and SysTick drivers.
- PC13 LED and UART board support.
- Bare-metal blink/heartbeat example.
- GNU Arm and Clang/LLD build paths.
- ST-Link/OpenOCD and GDB configuration without an NRST requirement.
- Phase 1 validation script and documentation.

### Added — Phase 0

- Initial directory structure and architecture specifications.

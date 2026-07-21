# Phase 1 validation report

## Automated validation performed

The Phase 1 source was compiled and linked with:

```text
clang --target=arm-none-eabi
-mcpu=cortex-m3 -mthumb
LLD 17
```

All C translation units were compiled with warnings treated as errors. The
validation generated ELF, BIN, HEX, and MAP outputs successfully.

## Build footprint

The exact size may vary slightly between LLVM and GNU Arm toolchains. The LLVM
validation build uses approximately:

```text
Flash: about 2.4 KiB
Initialized RAM: 4 bytes
BSS RAM: 4 bytes
Reserved main stack: 1024 bytes
```

## ELF checks

The validation confirms:

- ELF format is 32-bit little-endian ARM;
- `.isr_vector` begins at `0x08000000`;
- initial MSP value is `0x20005000`;
- Reset handler vector has the Thumb bit set;
- `SystemCoreClock` is placed in `.data`;
- the millisecond counter is placed in `.bss`;
- there are no undefined symbols.

## Hardware limitation

The code has not been flashed onto a physical Blue Pill in this environment.
The user should verify PC13, UART1 output, HSE startup, and ST-Link reset behavior
on their board. `make flash` performs OpenOCD programming and verification.

# HairRTOS Porting Guide

A new target port must provide stack-frame initialization, first-task startup,
PendSV save/restore, critical sections, ISR detection, context-switch request,
and wait-for-interrupt. The first TCB field must remain the saved stack pointer.

Required steps:

1. Define board memory and linker symbols, including `_estack` and `.noinit`.
2. Implement startup, vector table, clock, tick source, UART, and panic LED.
3. Implement `hr_port.h` functions and SVC/PendSV assembly.
4. Keep PendSV at the lowest exception priority and SysTick below SVC.
5. Add fault-frame capture for the architecture.
6. Run host tests, target symbol/disassembly checks, and a long target stress run.

The kernel, HairEvent, and allocator lab must not include STM32 headers.
Architecture code may depend on ARM core registers but not board peripherals.

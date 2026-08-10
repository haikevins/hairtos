# ARM Cortex-M3 port

## Files

```text
hr_port.c
hr_port_stack.c
hr_portasm.S
hr_fault.c
hr_faultasm.S
hr_kernel_tick_irq.c
hr_baremetal_tick_irq.c
hr_benchmark_clock_dwt.c
include/hr_port*.h
```

## Stack/exception model

Thread tasks dùng PSP; handlers dùng MSP.

Cortex-M hardware stack R0–R3/R12/LR/PC/xPSR. Port save thêm R4–R11.

## SVC

SVC được dùng cho first-task bootstrap:

- restore software frame;
- set PSP;
- select PSP in Thread mode;
- exception return.

## PendSV

PendSV priority thấp dùng cho context switch để tránh switch giữa ISR priority cao hơn.

## SysTick adapter

`hr_kernel_tick_irq.c` chỉ bridge `SysTick_Handler` → `hr_kernel_tick_from_isr()`.

Bare-metal variant bridge tới hardware timer counter. Chỉ một adapter được link trong image.

## Critical section

v1 dùng PRIMASK/cpsid i và restore prior mask.

## Fault

Fault assembly capture stacked frame, exception return và SCB registers. Strong handler chỉ link khi diagnostics/fault module được chọn.

## DWT benchmark

Benchmark clock backend dùng DWT cycle counter. Statistics generic không biết DWT.

## Capability

```text
FPU context: no
MPU: no
stack align: 8
min initial frame: 18 words
```

## V2

- BASEPRI critical ceiling;
- Cortex-M4F variant với optional FP context;
- interrupt priority validation;
- actual second target.

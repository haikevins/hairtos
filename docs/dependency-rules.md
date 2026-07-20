# Dependency Rules

## Allowed direction

```text
application -> framework/event -> kernel/include -> arch -> soc -> board/drivers
```

## Forbidden dependencies

- Kernel includes STM32 or board headers.
- Kernel calls GPIO or UART.
- HairEvent accesses NVIC, SysTick, PSP, MSP, or kernel internal headers.
- Drivers include TCB or scheduler internals.
- Applications include `kernel/internal`.
- Architecture code contains application signals.
- SoC or board code makes scheduling decisions.

## Public boundaries

Public headers are only under `kernel/include`, `framework/event/include`,
driver include directories, and board include directories. All `internal`
headers are private.

## Adapter rule

RTOS-aware driver behavior belongs in `adapters/`. A raw UART driver reports a
byte; an UART/RTOS adapter sends that byte to a queue from ISR context.

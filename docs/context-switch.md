# Cortex-M3 Context Switch

## Exception roles

- SVC starts the first task.
- PendSV performs deferred context switching.
- SysTick updates kernel time and may pend PendSV.

Cortex-M hardware saves R0-R3, R12, LR, PC, and xPSR on exception entry.
PendSV additionally saves and restores R4-R11.

## PendSV sequence

1. Read PSP.
2. Save R4-R11.
3. Store PSP into the current TCB.
4. Call scheduler selection code.
5. Load the next TCB's PSP.
6. Restore R4-R11.
7. Write PSP.
8. Exception return.

PendSV must have the lowest configurable exception priority. It must not
allocate memory, print, call drivers, or run application callbacks.

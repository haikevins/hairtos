# Porting Guide — Phase 0 Contract

The architecture port must eventually provide stack initialization, first-task
startup, deferred switch request, nested critical sections, ISR-context
detection, and yield-from-ISR.

Cortex-M3 assumptions: Thumb, PSP for tasks, MSP for exceptions, PendSV, SVC,
SysTick, privileged tasks, and no FPU context.

A valid port demonstrates correct first-task argument, retained local variables
across millions of switches, lowest PendSV priority, restored interrupt state,
correct tick rate, deferred ISR preemption, and 8-byte stack alignment.

Porting to Cortex-M0 must not require edits to kernel scheduler, queue,
semaphore, or HairEvent source files.

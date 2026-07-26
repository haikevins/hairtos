# Phase 10.1 — Semaphore From ISR

This target example demonstrates an ISR-to-task synchronization path:

1. the priority-1 waiter blocks on a binary semaphore;
2. the priority-3 trigger task periodically sets EXTI0 through `SWIER`;
3. `EXTI0_IRQHandler` calls `hr_semaphore_give_from_isr()`;
4. the ISR reports that a higher-priority task became READY;
5. PendSV runs after exception return and switches to the waiter.

Build and flash:

```bash
make EXAMPLE=10-01-semaphore-from-isr run
```

The EXTI interrupt is software-triggered, so no external button wiring is
required. USART1 output remains at 115200 baud.

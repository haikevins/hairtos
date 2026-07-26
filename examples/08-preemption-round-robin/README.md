# Phase 8 — Preemption and Round-Robin

This STM32F103 target example demonstrates both Phase 8 scheduling rules.

- `monitor`, priority 1, blocks for 250 ticks and preempts the workers as soon
  as its timeout expires;
- `worker-a` and `worker-b`, priority 3, are CPU-bound and never call
  `hr_task_yield()`;
- SysTick rotates the equal-priority workers after
  `HR_CFG_TIME_SLICE_TICKS`.

Build:

```bash
make EXAMPLE=08-preemption-round-robin
```

Flash:

```bash
make EXAMPLE=08-preemption-round-robin flash
```

Expected behavior: both worker counters continue increasing, while the monitor
runs immediately at each periodic release. A stalled worker triggers
`board_panic()`.

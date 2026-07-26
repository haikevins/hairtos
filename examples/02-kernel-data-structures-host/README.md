# Phase 2 — Kernel Data Structures Host Demo

This example runs on the development computer, not on STM32. It demonstrates
Phase 2 ready-set and wait-list ordering without creating real tasks.

```bash
make EXAMPLE=02-kernel-data-structures-host run
```

Expected behavior:

- priority 0 is selected before larger priority numbers;
- equal-priority ready nodes rotate FIFO;
- waiters are ordered by priority and preserve FIFO among equals.

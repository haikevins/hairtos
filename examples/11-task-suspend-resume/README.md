# Phase 11 — Task Suspend and Resume

Target-only STM32F103 demonstration of:

- suspending a task that is blocked on a delay;
- preserving its timeout while administratively suspended;
- preventing timeout completion from entering the ready queue;
- resuming a higher-priority task and preempting immediately;
- self-suspend and external resume.

Build and flash:

```bash
make EXAMPLE=11-task-suspend-resume flash
```

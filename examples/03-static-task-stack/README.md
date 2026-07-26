# Static Task Stack

This example creates one static task object and prepares its Cortex-M3 initial
stack frame. It deliberately does not start the task; SVC startup belongs to
the `04-start-first-task` example.

Build:

```bash
make EXAMPLE=03-static-task-stack build
```

Expected UART output confirms that the TCB and initial frame were created.

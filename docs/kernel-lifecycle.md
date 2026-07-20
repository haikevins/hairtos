# Kernel Lifecycle

```text
RESET -> INITIALIZING -> READY -> RUNNING -> PANIC
```

`hr_kernel_init()` initializes scheduler structures, timeout structures, the
idle task, optional service tasks, guards, and statistics.

`hr_kernel_start()` validates the port, selects the first task, and enters the
SVC startup path. It must not return during normal operation.

A fatal invariant violation moves the system to PANIC and calls
`hr_hook_kernel_panic()`. The default debug behavior should preserve state for
GDB instead of silently restarting.

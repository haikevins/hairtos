# Start First Task Using SVC

This target example proves the first real task launch on STM32F103 Cortex-M3.

## What it demonstrates

- `hr_kernel_init()` creates and readies the internal idle task;
- an application creates and registers one static task;
- the ready set chooses the application task over idle by priority;
- `hr_kernel_start()` invokes SVC;
- `SVC_Handler` restores R4–R11 and the hardware exception frame;
- Thread mode changes from MSP to PSP;
- R0 carries the task argument;
- the task entry runs and `main()` is abandoned.

This example does **not** implement PendSV switching. The first task remains the only
application task that can execute.

## Build and flash

```bash
make EXAMPLE=04-start-first-task build
make EXAMPLE=04-start-first-task run
```

## Expected UART output

```text
hairtos first-task startup
Preparing idle task and first application task...
Invoking SVC to leave main/MSP and enter task/PSP...
First task entered through SVC.
Current task: first-task
PSP active: yes
Task argument: valid
First-task startup: PASS
first-task heartbeat=1
```

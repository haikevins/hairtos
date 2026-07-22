# Kernel Benchmark Plan

## Principle

Correctness comes before speed. Benchmarks begin in Phase 15 after context
switching, scheduling, blocking IPC, timers, and Event-Driven dispatch pass
functional and stress tests.

## Metrics

### Startup

- SVC entry to first task instruction.

### Context switch

- PendSV entry to next task instruction;
- same-priority switch;
- different-priority switch.

### Scheduling

- ready-set decision time with different numbers of priorities and ready tasks.

### Wake-up latency

- SysTick interrupt to delayed task execution;
- ISR semaphore give to woken task execution;
- queue send to receiver execution.

### Event framework

- event post to Active Object dispatch;
- static event versus pooled dynamic event.

### Timers

- requested expiration versus callback start;
- periodic timer jitter.

### Critical sections

- maximum observed critical-section duration;
- number of nested entries.

### Resources

- Flash size;
- static RAM;
- stack high-water mark;
- queue and event-pool high-water marks.

## Measurement tools

### DWT_CYCCNT

Preferred for cycle-level measurements on Cortex-M3.

### GPIO pulse

Toggle a dedicated pin around the measured region and capture it with a logic
analyzer. This is useful for validating DWT measurements and interrupt latency.

### UART reporting

Store samples in RAM and print only after the run. Printing inside the measured
region is forbidden.

## Reporting

Each benchmark record must state:

- CPU and clock;
- compiler and version;
- optimization level;
- enabled kernel configuration;
- interrupt load;
- sample count;
- measurement overhead;
- minimum, maximum, mean, and selected percentiles.

## Anti-goals

Benchmark code must not:

- change scheduler policy;
- disable safety checks in the normal build without disclosure;
- include UART output in measured intervals;
- claim hard real-time guarantees from average latency alone.
